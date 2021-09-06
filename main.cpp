/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "freetronicsLCDShield.h"
#include "FloatRingBuffer.h"
#include "Stepper.h"
#include <cstdio>
#include <cstdlib>
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


MemoryPool<ScaleMeasurement_t, 2> ScaleMeasurementQueueMemoryPool;
Queue<ScaleMeasurement_t, 1> ScaleMeasurementQueue;


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
    POWDER_TRICKLE_WAIT_FOR_CONFIRM,
} TricklerState_t;

typedef enum
{
    TRICKLER_FINAL_STATE_0_WAIT_FOR_STABLE,
    TRICKLER_FINAL_STATE_0_TRICKLE,
    TRICKLER_FINAL_STATE_1_WAIT_FOR_STABLE,
    TRICKLER_FINAL_STATE_1_TRICKLE,
    TRICKLER_FINAL_STATE_2_WAIT_FOR_STABLE,
    TRICKLER_FINAL_STATE_2_TRICKLE, 
} TricklerFinalState_t;


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
                // printf("%s", string_buf);
                
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
    ThisThread::sleep_for(1s);
    lcd.cls();
    
    // Initialize thread
    ButtonPollThread.start(button_poll_handler);
    ScaleStreamThread.start(scale_stream_measurement_handler);
    
    // Initialize the scale
    // ScaleSerial.write("SIR\r\n", 5);  // put the scale to continuous reporting mode

    trickler.period_ms(1);  // 1khz output
    trickler.write(0.0f);  // 0 duty cycle
    
    TricklerState_t TricklerState = SELECT_WEIGHT;
    
    size_t average_buffer_length = 1;
    int8_t cursor_loc = 0;
    char charge_weight[6];  // include the trailing '\0'
    int pwm_percentage = 0;
    int stepper_motor_delay_us = 900;

    // PID control related variables
    float trickler_setpoint = 0.0f;
    float trickler_max_pwm = 0.25;
    float trickler_min_pwm = 0.19;

    float kp = 0.10f;
    float ki = 0.00001f;


    
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
            memset(charge_weight, '0', sizeof(charge_weight));
            
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
                if (charge_weight[cursor_loc] > '9'){
                    charge_weight[cursor_loc] = '0';
                }
            }
            else if (*button_press == freetronicsLCDShield::BTN_DOWN){
                charge_weight[cursor_loc] -= 1;
                if (charge_weight[cursor_loc] < '0'){
                    charge_weight[cursor_loc] = '9';
                }
            }
            else if (*button_press == freetronicsLCDShield::BTN_SELECT){
                // Turn charge weight to trickler setpoint
                charge_weight[2] = '.';
                trickler_setpoint = float(atof(charge_weight));
                printf("trickler_setpoint: %f\r\n", trickler_setpoint);
                TricklerState = POWDER_THROW;
            }
            
            // Display current cursor
            lcd.setCursorPosition(1, cursor_loc);
            lcd.printf("%c", charge_weight[cursor_loc]);
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

                ScaleSerial.write("Z\r\n", 3);
                ThisThread::sleep_for(1s);  // Zero takes about 1s to take in effect

                TricklerState = POWDER_THROW_WAIT_FOR_COMPLETE;
            }
        }
        else if (TricklerState == POWDER_THROW_WAIT_FOR_COMPLETE){
            lcd.setCursorPosition(1, 0);
            // lcd.printf("                ");
            if (*button_press == NULL){
                continue;
            }

            if (*button_press == freetronicsLCDShield::BTN_UP){
                stepper_motor_delay_us += 50;
            }
            else if (*button_press == freetronicsLCDShield::BTN_DOWN){
                stepper_motor_delay_us -= 50;
            }
            else if (*button_press == freetronicsLCDShield::BTN_LEFT){
                StepMotor.step(400);
            }
            else if (*button_press == freetronicsLCDShield::BTN_RIGHT){
                StepMotor.step(-400);
            }

            StepMotor.setStepDelayUs(stepper_motor_delay_us);
            
            lcd.setCursorPosition(1, 0);
            lcd.printf("%016d", stepper_motor_delay_us);
            
            // Auto enter trickle state
            TricklerState = POWDER_TRICKLE;
        }
        else if (TricklerState == POWDER_TRICKLE){
            lcd.cls();
            lcd.setCursorPosition(0, 0);
            lcd.printf("Powder Trickle");
            
            TricklerState = POWDER_TRICKLE_WAIT_FOR_INPUT;
        }
        else if (TricklerState == POWDER_TRICKLE_WAIT_FOR_INPUT){
            TricklerState = POWDER_TRICKLE_WAIT_FOR_STABLE;
        }
        else if (TricklerState == POWDER_TRICKLE_WAIT_FOR_STABLE){
            lcd.setCursorPosition(1, 0);
            lcd.printf("                ");

            TricklerFinalState_t TricklerFinalState = TRICKLER_FINAL_STATE_0_WAIT_FOR_STABLE;
            bool trickle_complete = false;

            int s1_measurement_buffer_length = 5;
            FloatRingBuffer S1MeasurementBuffer(s1_measurement_buffer_length);

            int s2_measurement_buffer_length = 32;
            FloatRingBuffer S2MeasurementBuffer(s2_measurement_buffer_length);

            // Rapid reading the measurement
            char display_buffer[16];

            while (!trickle_complete) {
                ScaleMeasurement_t *measurement_ptr = NULL;
                ScaleMeasurementQueue.try_get_for(20ms, &measurement_ptr);

                if (measurement_ptr != NULL) {
                    // Make a copy of the measurement
                    ScaleMeasurement_t measurement;
                    memcpy(&measurement, measurement_ptr, sizeof(measurement));
                    ScaleMeasurementQueueMemoryPool.free(measurement_ptr);

                    float error = trickler_setpoint - measurement.measurement;

                    printf("STATE=%d Err=%f\r\n", TricklerFinalState, error);

                    if (error < 0) {
                        trickle_complete = true;
                        continue;
                    }

                    // Final trickling stage
                    switch (TricklerFinalState){
                        case TRICKLER_FINAL_STATE_0_WAIT_FOR_STABLE:  // 0
                        {
                            // Wait for any measurement error to be less than 1.5
                            if (error > 0.5) {
                                TricklerFinalState = TRICKLER_FINAL_STATE_0_TRICKLE;
                            }
                            else {
                                TricklerFinalState = TRICKLER_FINAL_STATE_1_WAIT_FOR_STABLE;
                            }
                            break;
                        }
                        case TRICKLER_FINAL_STATE_0_TRICKLE:  // 1
                        {
                            trickler.write(0.23f);
                            TricklerFinalState = TRICKLER_FINAL_STATE_0_WAIT_FOR_STABLE;
                            break;
                        }

                        case TRICKLER_FINAL_STATE_1_WAIT_FOR_STABLE:  // 2
                        {
                            trickler.write(0.0f);
                            if (measurement.header == SCALE_HEADER_STABLE) {
                                S1MeasurementBuffer.enqueue(measurement.measurement);
                            }
                            else {
                                S1MeasurementBuffer.reset();
                            }

                            if (S1MeasurementBuffer.getCounter() == s1_measurement_buffer_length && S1MeasurementBuffer.getSd() <= 0.02) {
                                float precise_error = trickler_setpoint - S1MeasurementBuffer.getMean();
                                if (precise_error > 0.15) {
                                    TricklerFinalState = TRICKLER_FINAL_STATE_1_TRICKLE;
                                }
                                else {
                                    TricklerFinalState = TRICKLER_FINAL_STATE_2_WAIT_FOR_STABLE;
                                }
                            }

                            break;
                        }

                        case TRICKLER_FINAL_STATE_1_TRICKLE:  // 3
                        {

                            trickler.write(0.23);
                            ThisThread::sleep_for(220ms);
                            trickler.write(0.0f);

                            S1MeasurementBuffer.reset();

                            TricklerFinalState = TRICKLER_FINAL_STATE_1_WAIT_FOR_STABLE;
                            break;
                        }

                        case TRICKLER_FINAL_STATE_2_WAIT_FOR_STABLE:  // 4
                        {
                            trickler.write(0.0f);
                            if (measurement.header == SCALE_HEADER_STABLE) {
                                S2MeasurementBuffer.enqueue(measurement.measurement);
                            }
                            else {
                                S2MeasurementBuffer.reset();
                            }

                            if (S2MeasurementBuffer.getCounter() == s2_measurement_buffer_length && S2MeasurementBuffer.getSd() <= 0.02) {
                                float precise_error = trickler_setpoint - S2MeasurementBuffer.getMean();
                                if (precise_error > 0.01) {
                                    TricklerFinalState = TRICKLER_FINAL_STATE_2_TRICKLE;
                                }
                                else {
                                    trickle_complete = true;
                                }
                            }

                            break;
                        }

                        case TRICKLER_FINAL_STATE_2_TRICKLE:  // 5
                        {
                            trickler.write(0.20);
                            ThisThread::sleep_for(220ms);
                            trickler.write(0.0f);

                            ThisThread::sleep_for(200ms);
                            S2MeasurementBuffer.reset();

                            TricklerFinalState = TRICKLER_FINAL_STATE_2_WAIT_FOR_STABLE;
                            break;
                        }
                    }


                    // Display
                    memset(display_buffer, 0, sizeof(display_buffer));

                    // Determine sign
                    char sign = '+';
                    if (measurement.measurement < 0){
                        sign = '-';
                    }

                    // Determine unit
                    if (measurement.unit == SCALE_UNIT_GRAM){
                        snprintf(display_buffer, sizeof(display_buffer), "%c%07.4f g ", sign, abs(measurement.measurement));
                    }
                    else if (measurement.unit == SCALE_UNIT_GRAIN){
                        snprintf(display_buffer, sizeof(display_buffer), "%c%05.2f gn", sign, abs(measurement.measurement));
                    }

                    // Update display
                    lcd.setCursorPosition(1, 0);
                    lcd.printf("%s", display_buffer);                    
                }
            }
            
            // Auto enter trickle state
            TricklerState = POWDER_TRICKLE_WAIT_FOR_CONFIRM;
        }
        else if (TricklerState == POWDER_TRICKLE_WAIT_FOR_CONFIRM){
            if (*button_press == NULL){
                continue;
            }

            if (*button_press == freetronicsLCDShield::BTN_SELECT){
                TricklerState = POWDER_THROW;
            }
        }
        
        ButtonQueueMemoryPool.free(button_press);
    }
}