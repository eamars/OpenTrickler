// /* mbed Microcontroller Library
//  * Copyright (c) 2019 ARM Limited
//  * SPDX-License-Identifier: Apache-2.0
//  */

#include "mbed.h"

#include "SH1106.h"
#include "Arial12x12.h"
#include "Arial24x23.h"
#include "Arial28x28.h"
#include "Hotboards_keypad.h"
#include "XNucleoIHM02A1.h"

#include "app.h"
#include "cleanup_mode.h"
#include "main_menu.h"
#include "charge_mode.h"





// // External peripherals
BufferedSerial pc(USBTX, USBRX, 115200);
PwmOut Trickler(PC_8);

/*
OLED display
SPI2_MOSI -- PB15
SPI2_MISO -- PB14 -- NC
SPI2_SCK  -- PB13
RST 	  -- PC4
DC 	  -- PB1
CS	  -- PB2
*/
SH1106 OLEDScreen(SPI_8, 8000000, PB_15, PB_14, PB_13, PB_2, PC_4, PB_1, "OLED");

/*
COL1 -- PA13
COL2 -- PA14
COL3 -- PA15
COL4 -- PB7
ROW1 -- PC13
ROW2 -- PH1
ROW3 -- PC2
ROW4 -- PC3
*/
// Defines the pins connected to the rows
DigitalInOut colPins[ 4 ] = { PA_13 , PA_14 , PA_15 , PB_7 };
// Defines the pins connected to the cols
DigitalInOut rowPins[ 4 ] = { PC_13 , PH_1 , PC_2 , PC_3 };

// Defines the keys array with it's respective number of rows & cols,
// and with the value of each key
char keys[ 4 ][ 4 ] =
{
    { '1' , '2' , '3' , 'F' },
    { '4' , '5' , '6' , 'E' },
    { '7' , '8' , '9' , 'D' },
    { 'A' , '0' , 'B' , 'C' }
};
 
 
// Creates a new keyboard with the values entered before
Keypad MatrixKeyPad( makeKeymap( keys ) , rowPins , colPins , 4 , 4 );


/*
Motor board
D12 -- PA6 -- SDO
D11 -- PA7 -- SDI
D4  -- PB5 -- rst
D13 -- PA5 -- SCK
A2  -- PA4 -- CS
A4  -- PC1 -- nflag
A5  -- PC0 -- nbusy
*/
/* Initialization parameters of the motors connected to the expansion board. */
L6470_init_t TricklerMotorConfig = {
        12.0,                           /* Motor supply voltage in V. */
        200,                           /* Min number of steps per revolution for the motor. */
        0.7,                           /* Max motor phase voltage in A. */
        2.45,                          /* Max motor phase voltage in V. */
        300.0,                         /* Motor initial speed [step/s]. */
        500.0,                         /* Motor acceleration [step/s^2] (comment for infinite acceleration mode). */
        500.0,                         /* Motor deceleration [step/s^2] (comment for infinite deceleration mode). */
        992.0,                         /* Motor maximum speed [step/s]. */
        0.0,                           /* Motor minimum speed [step/s]. */
        602.7,                         /* Motor full-step speed threshold [step/s]. */
        2.45,                          /* Holding kval [V]. */
        2.45,                          /* Constant speed kval [V]. */
        2.45,                          /* Acceleration starting kval [V]. */
        2.45,                          /* Deceleration starting kval [V]. */
        61.52,                         /* Intersect speed for bemf compensation curve slope changing [step/s]. */
        392.1569e-6,                   /* Start slope [s/step]. */
        643.1372e-6,                   /* Acceleration final slope [s/step]. */
        643.1372e-6,                   /* Deceleration final slope [s/step]. */
        0,                             /* Thermal compensation factor (range [0, 15]). */
        2.45 * 1000 * 1.10,            /* Ocd threshold [ma] (range [375 ma, 6000 ma]). */
        2.45 * 1000 * 1.00,            /* Stall threshold [ma] (range [31.25 ma, 4000 ma]). */
        StepperMotor::STEP_MODE_1_128, /* Step mode selection. */
        0,                          /* Alarm conditions enable. */
        0x2E88                         /* Ic configuration. */
};

L6470_init_t ThrowerMotorConfig = {
        12.0,                           /* Motor supply voltage in V. */
        200,                           /* Min number of steps per revolution for the motor. */
        1.7,                           /* Max motor phase voltage in A. */
        2.25,                          /* Max motor phase voltage in V. */
        300.0,                         /* Motor initial speed [step/s]. */
        1500.0,                         /* Motor acceleration [step/s^2] (comment for infinite acceleration mode). */
        1500.0,                         /* Motor deceleration [step/s^2] (comment for infinite deceleration mode). */
        1200.0,                         /* Motor maximum speed [step/s]. */
        0.0,                           /* Motor minimum speed [step/s]. */
        602.7,                         /* Motor full-step speed threshold [step/s]. */
        2.25,                          /* Holding kval [V]. */
        2.25,                          /* Constant speed kval [V]. */
        2.25,                          /* Acceleration starting kval [V]. */
        2.25,                          /* Deceleration starting kval [V]. */
        61.52,                         /* Intersect speed for bemf compensation curve slope changing [step/s]. */
        392.1569e-6,                   /* Start slope [s/step]. */
        643.1372e-6,                   /* Acceleration final slope [s/step]. */
        643.1372e-6,                   /* Deceleration final slope [s/step]. */
        0,                             /* Thermal compensation factor (range [0, 15]). */
        2.25 * 1000 * 1.10,            /* Ocd threshold [ma] (range [375 ma, 6000 ma]). */
        2.25 * 1000 * 1.00,            /* Stall threshold [ma] (range [31.25 ma, 4000 ma]). */
        StepperMotor::STEP_MODE_1_128, /* Step mode selection. */
        0,                          /* Alarm conditions enable. */
        0x2E88                         /* Ic configuration. */
};


DevSPI _dev_spi(PA_7, PA_6, PA_5);
XNucleoIHM02A1 MotorController(&TricklerMotorConfig, &ThrowerMotorConfig, A4, A5, D4, A2, &_dev_spi);
L6470 *TricklerMotor;
L6470 *ThrowerMotor;

const int cfg_thrower_microstepping = 128;
const int cfg_thrower_throw_full_step_position = 175;


// RTOS control
Semaphore lcdWeightPrintEnable(0, 1);
Thread ButtonPollThread;
MemoryPool<char, 2> ButtonQueueMemoryPool;
Queue<char, 1> ButtonQueue;

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


FileHandle *mbed::mbed_override_console(int fd)
{
    return &pc;
}


void button_poll_handler(void){
    while(true){
        char *button_press = ButtonQueueMemoryPool.alloc();
        *button_press = MatrixKeyPad.getKey();
        
        // Look for press event (0->any)
        // Prev == NONE and Current != None
        if (*button_press){
            while (ButtonQueue.try_put(button_press) != true){
                // Read from the front of queue to free up space
                char *old_btn_press = NULL;
                ButtonQueue.try_get_for(0s, &old_btn_press);
                if (old_btn_press != NULL) {
                    ButtonQueueMemoryPool.free(old_btn_press);
                }
                
                ThisThread::sleep_for(20ms);
            }
        }
        else {
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

                    // // Update display
                    // lcd.setCursorPosition(1, 0);
                    // lcd.printf("%s", display_buffer);                    
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


void thrower_charge(void (*cb)(int)=NULL){
    // Go to charge position
    ThrowerMotor->go_to(0, StepperMotor::BWD);
    while (ThrowerMotor->is_busy()){
        if (cb != NULL) {
            cb(ThrowerMotor->get_position());
        }
    }
}

void thrower_discharge(void (*cb)(int)=NULL)
{
    // Go to discharge position
    ThrowerMotor->go_to(cfg_thrower_throw_full_step_position * cfg_thrower_microstepping, StepperMotor::FWD);
    while (ThrowerMotor->is_busy()){
        if (cb != NULL) {
            cb(ThrowerMotor->get_position());
        }
    }
}

void thrower_dispense(uint32_t stay_ms=250, void (*cb)(int)=NULL){
    // Go to discharge position
    thrower_discharge(cb);
    thread_sleep_for(stay_ms);

    // Go to charge position
    thrower_charge(cb);
    thread_sleep_for(stay_ms);
}


void _render_undefined_state(OpenTricklerStateFlag_e state){
    OLEDScreen.locate(0, 0);
    OLEDScreen.set_font((unsigned char *) Terminal6x8);

    OLEDScreen.background(White);
    OLEDScreen.foreground(Black);
    OLEDScreen.printf("          ERROR         \n");
    OLEDScreen.background(Black);
    OLEDScreen.foreground(White);

    OLEDScreen.locate(0, 10);
    OLEDScreen.printf("Undefined State: %d\n", state);
}
 

 
int main(void) {
    OLEDScreen.set_auto_up(false);
    OLEDScreen.set_orientation(3); 
    OLEDScreen.foreground(White);
    OLEDScreen.background(Black);

    OLEDScreen.set_font((unsigned char *) Arial28x28);
    OLEDScreen.printf("Open\nTrickler\n");
    OLEDScreen.copy_to_lcd();

    ThisThread::sleep_for(1s);

    // Display initializing sequence
    OLEDScreen.cls();
    OLEDScreen.locate(0, 0);
    OLEDScreen.set_font((unsigned char *) Terminal6x8);

    // Initialze motor
    OLEDScreen.printf("Init Motors");
    OLEDScreen.copy_to_lcd();
    L6470 **motors = MotorController.get_components();
    TricklerMotor = motors[0];
    ThrowerMotor = motors[1];

    ThrowerMotor->set_home();
    ThrowerMotor->wait_while_active();
    OLEDScreen.printf(" -- done\n");
    OLEDScreen.copy_to_lcd();

    // Initialize vibotary trickler
    OLEDScreen.printf("Init Trickler");
    OLEDScreen.copy_to_lcd();
    Trickler.period_ms(1);  // 1khz output
    Trickler.write(0.0f);  // 0 duty cycle
    OLEDScreen.printf(" -- done\n");
    OLEDScreen.copy_to_lcd();
    
    // Initialize thread
    OLEDScreen.printf("Init Button");
    OLEDScreen.copy_to_lcd();
    ButtonPollThread.start(button_poll_handler);
    OLEDScreen.printf(" -- done\n");
    OLEDScreen.copy_to_lcd();

    // Initialize the trickler state    
    TricklerState_t TricklerState = MAIN_MENU;

    while (true)
    {
        // printf("%d\r\n", TricklerState);
        if (TricklerState == MAIN_MENU){
            TricklerState = main_menu();
        }
        else if (TricklerState == CLEANUP_MODE_MENU){
            TricklerState = cleanup_mode_menu();
        }
        else if (TricklerState == CLEANUP_MODE_WAIT_FOR_COMPLETE) {
            TricklerState = cleanup_mode_wait_for_complete();
        }
        else if (TricklerState == CHARGE_MODE_SELECT_WEIGHT) {
            TricklerState = charge_mode_select_weight();
        }
        else if (TricklerState == CHARGE_MODE_POWDER_THROW) {
            TricklerState = charge_mode_powder_throw();
        }
        else{
            OLEDScreen.cls();
            _render_undefined_state(TricklerState);
            OLEDScreen.copy_to_lcd();
            ThisThread::sleep_for(2s);
            TricklerState = MAIN_MENU;
        }
    }
}



// int main(void) {
//     // turn on the back light (it's off by default)
//     lcd.setBackLight(true);
//     lcd.setCursor(false, false);
//     lcd.cls();
    
//     lcd.setCursorPosition(0, 0);
//     lcd.printf("OpenTrickler");
//     lcd.setCursorPosition(0, 0);
//     lcd.printf("OpenTrickler");
//     ThisThread::sleep_for(1s);
//     lcd.cls();
    
//     // Initialize thread
//     ButtonPollThread.start(button_poll_handler);
//      
    
//     // Initialize the scale
//     ScaleSerial.write("SIR\r\n", 5);  // put the scale to continuous reporting mode
    
//     // Initialize stepper motor
//     StepMotor.enableHold(false);
//     StepMotor.setRPM(170);

//     Trickler.period_ms(1);  // 1khz output
//     Trickler.write(0.0f);  // 0 duty cycle
    
//     TricklerState_t TricklerState = MAIN_MENU;

    
//     while (true)
//     {
//         freetronicsLCDShield::ButtonType_t *button_press = NULL;
        
//         // Block reading ButtonEvent
//         ButtonQueue.try_get_for(20ms, &button_press);

//         // printf("%d\r\n", TricklerState);
//         if (TricklerState == MAIN_MENU){
//             TricklerState = main_menu();
//         }
//         else if (TricklerState == MAIN_MENU_WAIT_FOR_INPUT){
//             if (*button_press == NULL){
//                 continue;
//             }
                
//             TricklerState = main_menu_wait_for_input(button_press);
//         }

//         else if (TricklerState == CLEANUP_MODE_MENU){
//             TricklerState = cleanup_mode_menu();
//         }

//         else if (TricklerState == CLEANUP_MODE_WAIT_FOR_INPUT) {
//             if (*button_press == NULL){
//                 continue;
//             }

//             TricklerState = cleanup_mode_wait_for_input(button_press);
//         }

//         else if (TricklerState == CLEANUP_MODE_WAIT_FOR_COMPLETE) {
//             TricklerState = cleanup_mode_wait_for_complete();
//         }
        
//         else if (TricklerState == CHARGE_MODE_SELECT_WEIGHT) {
//             TricklerState = charge_mode_select_weight();
//         }

//         else if (TricklerState == CHARGE_MODE_SELECT_WEIGHT_WAIT_FOR_INPUT) {
//             if (*button_press == NULL){
//                 continue;
//             }
                
//             TricklerState = charge_mode_select_weight_wait_for_input(button_press);
//         }

//         else if (TricklerState == CHARGE_MODE_POWDER_THROW) {
//             TricklerState = charge_mode_powder_throw();
//         }

//         else if (TricklerState == CHARGE_MODE_POWDER_THROW_WAIT_FOR_INPUT){
//             // Enable weight display regardless the user input
//             lcdWeightPrintEnable.release();

//             if (*button_press == NULL){
//                 continue;
//             }
//             TricklerState = charge_mode_powder_throw_wait_for_input(button_press);
//         }

//         else if (TricklerState == CHARGE_MODE_POWDER_THROW_WAIT_FOR_COMPLETE){
//             TricklerState = charge_mode_powder_throw_wait_for_complete();
//         }

//         else if (TricklerState == CHARGE_MODE_POWDER_TRICKLE_WAIT_FOR_COMPLETE){
//             TricklerState = charge_mode_powder_trickle_wait_for_complete();
//         }

//         else if (TricklerState == CHARGE_MODE_POWDER_TRICKLE_WAIT_FOR_CUP_REMOVAL){
//             TricklerState = charge_mode_powder_trickle_wait_for_cup_removal();
//         }

//         else if (TricklerState == CHARGE_MODE_POWDER_TRICKLE_WAIT_FOR_CUP_REMOVAL){
//             TricklerState = charge_mode_powder_trickle_wait_for_cup_removal();
//         }

//         else if (TricklerState == CHARGE_MODE_POWDER_TRICKLE_WAIT_FOR_CUP_RETURNED){
//             TricklerState = charge_mode_powder_trickle_wait_for_cup_removal();
//         }

//         else{
//             lcd.cls();
//             lcd.setCursorPosition(0, 0);
//             lcd.printf("Not Implemented");

//             ThisThread::sleep_for(1s);
//             TricklerState = MAIN_MENU;
//         }
        
//         ButtonQueueMemoryPool.free(button_press);
//     }
// }







// int main(void){
//     oled.set_auto_up(false);
//     oled.set_orientation(3); 
//     // oled.foreground(Black);
//     // oled.background(White);
//     oled.set_font((unsigned char*)Arial12x12);
//     oled.cls();

//     oled.printf("SetPoint 43.00\n");

//     oled.set_font((unsigned char*)Arial28x28);
//     oled.printf("42.5");

//     oled.rect(0, 40, 127, 63, White);
//     oled.fillrect(0, 40, 52, 63, White);

//     oled.copy_to_lcd();

    

//     while (true){
//         char key = keypad.getKey();
//         // If any key was pressed
//         if( key )
//         {
//             // Display the key pressed on serial port
//             printf( "%c" , key );
//             printf( "\n\r" );
//         }
//     }
//     return 0;
// }