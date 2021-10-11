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

#include "app.h"
#include "cleanup_mode.h"
#include "main_menu.h"
#include "charge_mode.h"


// External peripherals
freetronicsLCDShield lcd(D8, D9, D4, D5, D6, D7, D10, A0); // rs, e, d0, d1, d2, d3, bl, a0
BufferedSerial pc(USBTX, USBRX, 115200);
PwmOut Trickler(PC_8);
Stepper StepMotor(PA_15, PB_7, PA_13, PA_14, PC_2, PC_3, 200);

// RTOS control
Semaphore lcdWeightPrintEnable(0, 1);
Thread ButtonPollThread;
MemoryPool<freetronicsLCDShield::ButtonType_t, 2> ButtonQueueMemoryPool;
Queue<freetronicsLCDShield::ButtonType_t, 1> ButtonQueue;

// Comm protocol: https://www.digitalscalesblog.com/interface-description-rs-232-fx-i-fz-i-precision-balances/
// TODO: The scale need to change the serial comm to following settings to get maximum performance
// Baud rate: 19200
// Databits: 8
// Parity: None
BufferedSerial ScaleSerial(PC_10, PC_5, 19200);
Thread ScaleStreamThread;

MemoryPool<ScaleMeasurement_t, 2> ScaleMeasurementQueueMemoryPool;
Queue<ScaleMeasurement_t, 1> ScaleMeasurementQueue;



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





// Configuration
int cfg_powder_measure_discharge_angle_step = 170;


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
    char buf[10];
    memcpy(buf, received_data->data, 9);
    buf[9] = 0;  // correct termination for atof
    measurement->measurement = (float)atof(buf);
    
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

                // Decode received serial data into the measurement object
                ScaleMeasurement_t measurement;
                decode_and_standard_format(((ScaleStandardOutputDataFormat_t *)string_buf), &measurement);
                
                // Decode received data and generate measurement data
                ScaleMeasurement_t *measurement_to_send = ScaleMeasurementQueueMemoryPool.alloc();
                memcpy(measurement_to_send, &measurement, sizeof(ScaleMeasurement_t));
                
                // Send data to the queue
                while (ScaleMeasurementQueue.try_put(measurement_to_send) != true){
                    // Read from the front of queue to free up space
                    ScaleMeasurement_t *old_measurement = NULL;
                    ScaleMeasurementQueue.try_get_for(0s, &old_measurement);
                    if (old_measurement != NULL) {
                        ScaleMeasurementQueueMemoryPool.free(old_measurement);
                    }

                    ThisThread::sleep_for(20ms);
                }

                // Optionally, if print weight feature is requested then it will show current measured weight
                if (lcdWeightPrintEnable.try_acquire()){
                    // Display
                    char display_buffer[16];
                    memset(display_buffer, 0, sizeof(display_buffer));

                    // Determine sign
                    char sign = '+';
                    if (measurement.measurement < 0){
                        sign = '-';
                    }

                    // Determine unit
                    if (measurement.unit == SCALE_UNIT_GRAM){
                        snprintf(display_buffer, sizeof(display_buffer), "%c%08.4f g ", sign, abs(measurement.measurement));
                    }
                    else if (measurement.unit == SCALE_UNIT_GRAIN){
                        snprintf(display_buffer, sizeof(display_buffer), "%c%06.2f gn", sign, abs(measurement.measurement));
                    }

                    // If not stable then add * to the end of the line
                    if (measurement.header != SCALE_HEADER_STABLE){
                        display_buffer[15] = '*';
                    }

                    // Update display
                    lcd.setCursorPosition(1, 0);
                    lcd.printf("%s", display_buffer);                    
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
    ScaleSerial.write("SIR\r\n", 5);  // put the scale to continuous reporting mode
    
    // Initialize stepper motor
    StepMotor.enableHold(false);
    StepMotor.setRPM(170);

    Trickler.period_ms(1);  // 1khz output
    Trickler.write(0.0f);  // 0 duty cycle
    
    TricklerState_t TricklerState = MAIN_MENU;

    
    while (true)
    {
        freetronicsLCDShield::ButtonType_t *button_press = NULL;
        
        // Block reading ButtonEvent
        ButtonQueue.try_get_for(20ms, &button_press);

        // printf("%d\r\n", TricklerState);
        if (TricklerState == MAIN_MENU){
            TricklerState = main_menu();
        }
        else if (TricklerState == MAIN_MENU_WAIT_FOR_INPUT){
            if (*button_press == NULL){
                continue;
            }
                
            TricklerState = main_menu_wait_for_input(button_press);
        }

        else if (TricklerState == CLEANUP_MODE_MENU){
            TricklerState = cleanup_mode_menu();
        }

        else if (TricklerState == CLEANUP_MODE_WAIT_FOR_INPUT) {
            if (*button_press == NULL){
                continue;
            }

            TricklerState = cleanup_mode_wait_for_input(button_press);
        }

        else if (TricklerState == CLEANUP_MODE_WAIT_FOR_COMPLETE) {
            TricklerState = cleanup_mode_wait_for_complete();
        }
        
        else if (TricklerState == CHARGE_MODE_SELECT_WEIGHT) {
            TricklerState = charge_mode_select_weight();
        }

        else if (TricklerState == CHARGE_MODE_SELECT_WEIGHT_WAIT_FOR_INPUT) {
            if (*button_press == NULL){
                continue;
            }
                
            TricklerState = charge_mode_select_weight_wait_for_input(button_press);
        }

        else if (TricklerState == CHARGE_MODE_POWDER_THROW) {
            TricklerState = charge_mode_powder_throw();
        }

        else if (TricklerState == CHARGE_MODE_POWDER_THROW_WAIT_FOR_INPUT){
            // Enable weight display regardless the user input
            lcdWeightPrintEnable.release();

            if (*button_press == NULL){
                continue;
            }
            TricklerState = charge_mode_powder_throw_wait_for_input(button_press);
        }

        else if (TricklerState == CHARGE_MODE_POWDER_THROW_WAIT_FOR_COMPLETE){
            TricklerState = charge_mode_powder_throw_wait_for_complete();
        }

        else if (TricklerState == CHARGE_MODE_POWDER_TRICKLE_WAIT_FOR_COMPLETE){
            TricklerState = charge_mode_powder_trickle_wait_for_complete();
        }

        else if (TricklerState == CHARGE_MODE_POWDER_TRICKLE_WAIT_FOR_CUP_REMOVAL){
            TricklerState = charge_mode_powder_trickle_wait_for_cup_removal();
        }

        else if (TricklerState == CHARGE_MODE_POWDER_TRICKLE_WAIT_FOR_CUP_REMOVAL){
            TricklerState = charge_mode_powder_trickle_wait_for_cup_removal();
        }

        else if (TricklerState == CHARGE_MODE_POWDER_TRICKLE_WAIT_FOR_CUP_RETURNED){
            TricklerState = charge_mode_powder_trickle_wait_for_cup_removal();
        }

        else{
            lcd.cls();
            lcd.setCursorPosition(0, 0);
            lcd.printf("Not Implemented");

            ThisThread::sleep_for(1s);
            TricklerState = MAIN_MENU;
        }
        
        ButtonQueueMemoryPool.free(button_press);
    }
}