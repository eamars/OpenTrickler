/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "freetronicsLCDShield.h"
#include "FloatRingBuffer.h"
#include "Stepper.h"
#include <cstdio>
#include <cstring>


freetronicsLCDShield lcd(D8, D9, D4, D5, D6, D7, D10, A0); // rs, e, d0, d1, d2, d3, bl, a0
BufferedSerial pc(USBTX, USBRX, 115200);
PwmOut trickler(PC_8);


// Stepper rotor control
Stepper StepMotor(PA_15, PB_7, PA_13, PA_14, PC_2, PC_3, 200);

Thread ButtonPollThread;
MemoryPool<freetronicsLCDShield::ButtonType_t, 6> ButtonQueueMemoryPool;
Queue<freetronicsLCDShield::ButtonType_t, 5> ButtonQueue;

// Comm protocol: https://www.digitalscalesblog.com/interface-description-rs-232-fx-i-fz-i-precision-balances/
// TODO: The scale need to change the serial comm to following settings to get maximum performance
// Baud rate: 19200
// Databits: 8
// Parity: None
BufferedSerial ScaleSerial(PC_10, PC_5, 19200);
Thread ScaleStreamThread;

typedef enum 
{
    SCALE_HEADER_STABLE,
    SCALE_HEADER_UNSTABLE,
    SCALE_HEADER_OVERLOAD,
    SCALE_HEADER_STABLE_COUNTING,
} ScaleMeasurementHeader_e;

typedef enum
{
    SCALE_UNIT_GRAM,
    SCALE_UNIT_GRAIN,
} ScaleMesasurementUnit_e;

typedef struct {
    float measurement;
    char raw_measurement[10];  // 9 + '\0'
    ScaleMeasurementHeader_e header;
    ScaleMesasurementUnit_e unit; 
} ScaleMeasurement_t;


typedef union {
    struct __attribute__((__packed__)){
        char header[2];
        char comma;
        char data[9];
        char unit[3];
        char terminator[2];
    };
    char packet[17];
} ScaleStandardOutputDataFormat_t;


MemoryPool<ScaleMeasurement_t, 6> ScaleMeasurementQueueMemoryPool;
Queue<ScaleMeasurement_t, 5> ScaleMeasurementQueue;


typedef enum 
{
    MAIN_MENU,
    MAIN_MENU_WAIT_FOR_INPUT,
    SELECT_WEIGHT,
    SELECT_WEIGHT_WAIT_FOR_INPUT,
    POWDER_THROW,
    POWDER_THROW_WAIT_FOR_INPUT,
    POWDER_THROW_WAIT_FOR_COMPLETE,
    POWDER_TRICKLE,
    POWDER_TRICKLE_WAIT_FOR_INPUT,
    POWDER_TRICKLE_WAIT_FOR_STABLE,
} TricklerState_t;


FileHandle *mbed::mbed_override_console(int fd)
{
    return &pc;
}


void button_poll_handler(void){
    freetronicsLCDShield::ButtonType_t last_button_press = freetronicsLCDShield::BTN_NONE;
    while(true){
        freetronicsLCDShield::ButtonType_t *button_press = ButtonQueueMemoryPool.alloc();
        *button_press = lcd.readButton();
        
        // Look for press event (0->any)
        // Prev == NONE and Current != None
        if (last_button_press == freetronicsLCDShield::BTN_NONE && *button_press != freetronicsLCDShield::BTN_NONE){
            // Record last button press
            last_button_press = *button_press;
            
            while (ButtonQueue.try_put(button_press) != true){
                // Read from the front of queue to free up space
                freetronicsLCDShield::ButtonType_t *old_btn_press = NULL;
                ButtonQueue.try_get_for(0s, &old_btn_press);
                if (old_btn_press != NULL) {
                    ButtonQueueMemoryPool.free(old_btn_press);
                }
                
        
                ThisThread::sleep_for(20ms);
            }
        }
        else {
            // Record last button press
            last_button_press = *button_press;
            ButtonQueueMemoryPool.free(button_press);
        }
        
        ThisThread::sleep_for(50ms);
    }
}


void decode_and_standard_format(ScaleStandardOutputDataFormat_t *received_data, ScaleMeasurement_t *measurement){
    // Decode header
    if (strncmp(received_data->header, "ST", 2) == 0){
        measurement->header = SCALE_HEADER_STABLE;
    }
    else if (strncmp(received_data->header, "US", 2) == 0){
        measurement->header = SCALE_HEADER_UNSTABLE;
    }
    else if (strncmp(received_data->header, "OL", 2) == 0){
        measurement->header = SCALE_HEADER_OVERLOAD;
    }
    else if (strncmp(received_data->header, "QT", 2) == 0){
        measurement->header = SCALE_HEADER_STABLE_COUNTING;
    }
    
    // Decode data
    memcpy(measurement->raw_measurement, received_data->data, 9);
    measurement->raw_measurement[9] = 0;  // correct termination for atof
    measurement->measurement = (float)atof(measurement->raw_measurement);
    
    // Decode unit
    if (strncmp(received_data->unit, "  g", 3) == 0){
        measurement->unit = SCALE_UNIT_GRAM;
    }
    else if (strncmp(received_data->unit, " GN", 3) == 0){
        measurement->unit = SCALE_UNIT_GRAIN;
    }
}


void scale_stream_measurement_handler(void){
    char string_buf[20];
    uint8_t string_buf_idx = 0;
    char ch_buf;
    
    while (true) {
        // Read one char at a time, if data is received then keep reading, 
        // until running out of bytes in the buffer
        ssize_t read_length = ScaleSerial.read(&ch_buf, 1);
        if (read_length == 1){
            string_buf[string_buf_idx++] = ch_buf;
            
            if (string_buf_idx == 17) {
                // full data
                printf("%s", string_buf);
                
                // Decode received data and generate measurement data
                ScaleMeasurement_t *measurement = ScaleMeasurementQueueMemoryPool.alloc();
                decode_and_standard_format(((ScaleStandardOutputDataFormat_t *)string_buf), measurement);
                
                // Send data to the queue
                while (ScaleMeasurementQueue.try_put(measurement) != true){
                    // Read from the front of queue to free up space
                    ScaleMeasurement_t *old_measurement = NULL;
                    ScaleMeasurementQueue.try_get_for(0s, &old_measurement);
                    if (old_measurement != NULL) {
                        ScaleMeasurementQueueMemoryPool.free(old_measurement);
                    }

                    ThisThread::sleep_for(20ms);
                }
                
                // reset the pointer and start-off
                string_buf_idx = 0; 
            }
            // If the last byte is LF, then reset the index (end of message)
            else if (string_buf[string_buf_idx - 1] == '\n') {
                string_buf_idx = 0; 
            }
        }
        else {
            // If no data is received then wait for 20ms for next poll cycle
            ThisThread::sleep_for(20ms);
        }
    }
}

 
int main(void) {
    // turn on the back light (it's off by default)
    lcd.setBackLight(true);
    lcd.setCursor(false, false);
    lcd.cls();
    
    lcd.setCursorPosition(0, 0);
    lcd.printf("OpenTrickler");
    lcd.setCursorPosition(0, 0);
    lcd.printf("OpenTrickler");
    ThisThread::sleep_for(2s);
    lcd.cls();
    
    // Initialize thread
    ButtonPollThread.start(button_poll_handler);
    ScaleStreamThread.start(scale_stream_measurement_handler);
    
    // Initialize the scale
    // ScaleSerial.set_format(7, BufferedSerial::Even, 1);
    // ScaleSerial.write("SIR\r\n", 5);  // put the scale to continuous reporting mode

    trickler.period_ms(1);  // 1khz output
    trickler.write(0.0f);  // 50% duty cycle
    
    TricklerState_t TricklerState = SELECT_WEIGHT;
    
    size_t average_buffer_length = 16;
    int8_t cursor_loc = 0;
    int8_t charge_weight[5];
    int pwm_percentage = 0;
    
    while (true)
    {
        freetronicsLCDShield::ButtonType_t *button_press = NULL;
        
        // Block reading ButtonEvent
        ButtonQueue.try_get_for(20ms, &button_press);
        
        if (TricklerState == SELECT_WEIGHT) {
            lcd.cls();
            lcd.setCursorPosition(0, 0);
            lcd.printf("Select Weight");
            
            lcd.setCursorPosition(1, 0);
            lcd.printf("00.00 gn");
            memset(charge_weight, 0, sizeof(charge_weight));
            
            lcd.setCursor(true, false);
            lcd.setCursorPosition(1, 0);
            TricklerState = SELECT_WEIGHT_WAIT_FOR_INPUT;
        }
        else if (TricklerState == SELECT_WEIGHT_WAIT_FOR_INPUT) {
            if (*button_press == NULL){
                continue;
            }
                
            if (*button_press == freetronicsLCDShield::BTN_LEFT){
                cursor_loc -= 1;
                if (cursor_loc < 0){
                    cursor_loc = 4;
                }
                else if (cursor_loc == 2){
                    // Skip the dot
                    cursor_loc = 1;
                }
            }
            else if (*button_press == freetronicsLCDShield::BTN_RIGHT) {
                cursor_loc += 1;
                if (cursor_loc > 4){
                    cursor_loc = 0;
                }
                else if (cursor_loc == 2){
                    // Skip the dot
                    cursor_loc = 3;
                }
            }
            else if (*button_press == freetronicsLCDShield::BTN_UP){
                charge_weight[cursor_loc] += 1;
                if (charge_weight[cursor_loc] > 9){
                    charge_weight[cursor_loc] = 0;
                }
            }
            else if (*button_press == freetronicsLCDShield::BTN_DOWN){
                charge_weight[cursor_loc] -= 1;
                if (charge_weight[cursor_loc] < 0){
                    charge_weight[cursor_loc] = 9;
                }
            }
            else if (*button_press == freetronicsLCDShield::BTN_SELECT){
                TricklerState = POWDER_THROW;
            }
            
            // Display current cursor
            lcd.setCursorPosition(1, cursor_loc);
            lcd.printf("%d", charge_weight[cursor_loc]);
            // printf will auto shift 
            lcd.setCursorPosition(1, cursor_loc);
        }
        else if (TricklerState == POWDER_THROW) {
            lcd.cls();
            lcd.setCursorPosition(0, 0);
            lcd.printf("Powder Throw");
            
            lcd.setCursorPosition(1, 0);
            lcd.printf("> SELECT");
            
            // disable cursor from now on
            lcd.setCursor(false, false);
            
            TricklerState = POWDER_THROW_WAIT_FOR_INPUT;
        }
        else if (TricklerState == POWDER_THROW_WAIT_FOR_INPUT){
            if (*button_press == NULL){
                continue;
            }
            if (*button_press == freetronicsLCDShield::BTN_SELECT){
                lcd.setCursorPosition(1, 0);
                lcd.printf("                ");
                TricklerState = POWDER_THROW_WAIT_FOR_COMPLETE;
            }
        }
        else if (TricklerState == POWDER_THROW_WAIT_FOR_COMPLETE){
            lcd.setCursorPosition(1, 0);
            // lcd.printf("                ");
            
            // Rapid reading the measurement
            // TODO: Show measurement for about 2s then move on. 
            if (*button_press == NULL){
                continue;
            }

            float percentage = 0.0;
                
            if (*button_press == freetronicsLCDShield::BTN_UP){
                pwm_percentage += 5;
                if (pwm_percentage > 100) {
                    pwm_percentage = 100;
                }
            }
            else if (*button_press == freetronicsLCDShield::BTN_DOWN){
                pwm_percentage -= 5;
                if (pwm_percentage < 0) {
                    pwm_percentage = 0;
                }
            }
            else if (*button_press == freetronicsLCDShield::BTN_LEFT){
                StepMotor.step(50);
            }
            else if (*button_press == freetronicsLCDShield::BTN_RIGHT){
                StepMotor.step(-50);
            }

            percentage = pwm_percentage / 100.0f;
            trickler.write(pwm_percentage / 100.0f); 
            
            lcd.setCursorPosition(1, 0);
            lcd.printf("%03d%%", pwm_percentage);
            
            // Auto enter trickle state
            // TricklerState = POWDER_TRICKLE;
        }
        else if (TricklerState == POWDER_TRICKLE){
            lcd.cls();
            lcd.setCursorPosition(0, 0);
            lcd.printf("Powder Trickle");
            
            lcd.setCursorPosition(1, 0);
            lcd.printf("> AUTO");
            
            TricklerState = POWDER_TRICKLE_WAIT_FOR_INPUT;
        }
        else if (TricklerState == POWDER_TRICKLE_WAIT_FOR_INPUT){
            TricklerState = POWDER_TRICKLE_WAIT_FOR_STABLE;
        }
        else if (TricklerState == POWDER_TRICKLE_WAIT_FOR_STABLE){
            lcd.setCursorPosition(1, 0);
            lcd.printf("                ");
            
            // Use PID controller to fine adjust the amount of powder to trickle
            
            // Rapid reading the measurement
            FloatRingBuffer MeasurementBuffer(average_buffer_length);
            float sd = 0;
            float mean = 0;
            char display_buffer[16];

            while (true) {
                ScaleMeasurement_t *measurement = NULL;
                ScaleMeasurementQueue.try_get_for(0s, &measurement);
                if (measurement != NULL) {
                    lcd.setCursorPosition(1, 0);

                    // Have enought data to calculate mean and SD
                    MeasurementBuffer.enqueue(measurement->measurement);
                    if (MeasurementBuffer.getCounter() > average_buffer_length){
                        sd = MeasurementBuffer.getSd();
                        mean = MeasurementBuffer.getMean();
                        // printf("%f\r\n", MeasurementBuffer.getSd());

                        memset(display_buffer, 0, sizeof(display_buffer));

                        // Determine sign
                        char sign = '+';
                        if (mean < 0){
                            sign = '-';
                        }

                        // Determine unit
                        char unit[2];
                        if (measurement->unit == SCALE_UNIT_GRAM){
                            memcpy(unit, " g", 2);
                        }
                        else if (measurement->unit == SCALE_UNIT_GRAIN){
                            memcpy(unit, "gn", 2);
                        }

                        snprintf(display_buffer, sizeof(display_buffer), "%c%08.4f %s", sign, mean, unit);

                        // Update display
                        lcd.setCursorPosition(1, 0);
                        lcd.printf("%s", display_buffer);
                    }
                    
                    ScaleMeasurementQueueMemoryPool.free(measurement);
                }
            }
            
            // Auto enter trickle state
            TricklerState = POWDER_THROW;
        }
        
        
        ButtonQueueMemoryPool.free(button_press);
    }
}