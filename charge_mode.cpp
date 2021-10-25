#include <cstring>
#include <cmath>
#include "charge_mode.h"
#include "mbed.h"
#include "FloatRingBuffer.h"
#include "SH1106.h"
#include "L6470.h"
#include "Arial28x28.h"

// Invoke peripheral declared in main
extern Semaphore lcdWeightPrintEnable;
extern BufferedSerial ScaleSerial;
extern Queue<ScaleMeasurement_t, 1> ScaleMeasurementQueue;
extern MemoryPool<ScaleMeasurement_t, 2> ScaleMeasurementQueueMemoryPool;
extern ScaleMeasurement_t latched_scale_measurement;
extern PwmOut Trickler;

extern SH1106 OLEDScreen;
extern Queue<char, 1> ButtonQueue;
extern MemoryPool<char, 2> ButtonQueueMemoryPool;
extern L6470 *ThrowerMotor;
extern L6470 *TricklerMotor;
extern void thrower_discharge(void (*cb)(int)=NULL, bool wait=true);
extern void thrower_charge(void (*cb)(int)=NULL, bool wait=true);


// Local static variables
static char _charge_weight_string[6];
static int _cursor_loc = 0;
static float _charge_weight_set_point = 0;
const float cfg_cup_removal_sd_threshold = 5;
const float cfg_cup_returned_sd_threshold = 0.02;
const float cfg_cup_returned_zero_threshold = 0.04;
Thread ScaleWeightDisplayThread;
Semaphore ScaleWeightDisplayEnable(0, 1);


void _render_charge_mode_title(){
    OLEDScreen.locate(0, 0);
    OLEDScreen.set_font((unsigned char *) Terminal6x8);

    OLEDScreen.background(White);
    OLEDScreen.foreground(Black);
    OLEDScreen.printf("      Charge Mode      \n");
    OLEDScreen.background(Black);
    OLEDScreen.foreground(White);   
}

void _render_charge_weight_prompt(){
    OLEDScreen.locate(0, 10);
    OLEDScreen.printf("Provide Charge Weight\n");

    OLEDScreen.locate(0, 55);
    OLEDScreen.printf("Press C to continue\n");
}


void _render_charge_weight_string(void){
    OLEDScreen.cls();
    _render_charge_mode_title();
    _render_charge_weight_prompt();
    OLEDScreen.set_font((unsigned char *) Arial28x28);
    OLEDScreen.locate(0, 22);
    OLEDScreen.printf("%s", _charge_weight_string);
}


void _render_charge_weight_progress(float current_weight, float set_weight){
    OLEDScreen.cls();
    _render_charge_mode_title();
    OLEDScreen.locate(0, 45);
    OLEDScreen.set_font((unsigned char *) Terminal6x8);
    OLEDScreen.printf("%.2f/%.2f", current_weight, set_weight);
    OLEDScreen.rect(0, 55, 127, 63, White);

    // Fill bar
    float ratio = min(1.0f, max(0.0f, current_weight / set_weight));
    int x1 = int(round(127.0f * ratio));
    OLEDScreen.fillrect(0, 55, x1, 63, White);
}


TricklerState_t charge_mode_select_weight(void){
    OLEDScreen.cls();
    _render_charge_weight_string();
    
    OLEDScreen.copy_to_lcd();

    OpenTricklerStateFlag_e next_state = UNDEFINED_STATE;

    while (next_state == UNDEFINED_STATE){
        char *button_press = NULL;
        
        // Block reading ButtonEvent
        ButtonQueue.try_get_for(20ms, &button_press);

        if (button_press == NULL){
            continue;
        }

        char button = *button_press;
        ButtonQueueMemoryPool.free(button_press);

        if (button == 'D') {
            next_state = MAIN_MENU;
        }
        else if (button == 'C') {
            _charge_weight_set_point = float(atof(_charge_weight_string));
            next_state = CHARGE_MODE_POWDER_THROW;
        }
        else{
            if (button >= '0' && button <= '9'){
                if (_cursor_loc < 5) {
                    _charge_weight_string[_cursor_loc++] = button;

                    if (_cursor_loc == 2){
                        _charge_weight_string[_cursor_loc++] = '.';
                    }
                }
            }
            else if (button == 'B') {
                if (_cursor_loc > 0){
                    _charge_weight_string[--_cursor_loc] = '\0';
                    if (_charge_weight_string[_cursor_loc - 1] == '.'){
                        _charge_weight_string[--_cursor_loc] = '\0';
                    }
                }
            }
            _render_charge_weight_string();
            OLEDScreen.copy_to_lcd();
        }
    }

    return next_state;
}



TricklerState_t charge_mode_powder_throw(void){
    OLEDScreen.cls();
    _render_charge_mode_title();

    OLEDScreen.locate(0, 10);
    OLEDScreen.printf("Setpoint: %.2f\n", _charge_weight_set_point);

    OLEDScreen.locate(0, 30);
    OLEDScreen.printf("Present CUP");
    OLEDScreen.locate(0, 55);
    OLEDScreen.printf("Press C to continue\n");

    OLEDScreen.copy_to_lcd();

    OpenTricklerStateFlag_e next_state = UNDEFINED_STATE;

    while (next_state == UNDEFINED_STATE){
        char *button_press = NULL;
        
        // Block reading ButtonEvent
        ButtonQueue.try_get_for(20ms, &button_press);

        if (button_press == NULL){
            continue;
        }

        char button = *button_press;
        ButtonQueueMemoryPool.free(button_press);

        if (button == 'D') {
            next_state = CHARGE_MODE_SELECT_WEIGHT;
        }
        else if (button == 'C') {
            next_state = CHARGE_MODE_POWDER_THROW_WAIT_FOR_COMPLETE;
        }
    }
    return next_state;
}


void _render_discharge_progress(int arg=0){
    _render_charge_weight_progress(latched_scale_measurement.measurement, _charge_weight_set_point);
    OLEDScreen.copy_to_lcd();
}


void scale_weight_display_handler(void){
    while (true){
        if (ScaleWeightDisplayEnable.try_acquire_for(1s)){
            _render_discharge_progress();
        }
    }
}


void _refresh_scale_weight_display(int arg=0){
    ScaleWeightDisplayEnable.release();
}


TricklerState_t charge_mode_powder_throw_wait_for_complete(void){
    // Start display thread
    if (ScaleWeightDisplayThread.get_state() != Thread::Running){
        ScaleWeightDisplayThread.start(scale_weight_display_handler);
    }

    _render_charge_weight_progress(0.0f, _charge_weight_set_point);
    OLEDScreen.copy_to_lcd();

    thrower_discharge(_refresh_scale_weight_display, true);
    thread_sleep_for(100);

    // We don't need to wait for the charge to complete to save some time
    thrower_charge(NULL, false);

    return CHARGE_MODE_POWDER_TRICKLE_WAIT_FOR_COMPLETE;
}


TricklerState_t charge_mode_powder_trickle_wait_for_complete(void){
    TricklerMotor->set_max_speed(500);
    TricklerMotor->set_min_speed(1);
    // PID related
    float kp = 100.0f;
    float ki = 0.0f;
    float kd = 30.0f;

    float integral = 0.0f;
    float last_error = 0.0f;

    Timer timer;
    timer.start();

    // Control variables
    bool trickle_complete = false;

    while (true){
        _refresh_scale_weight_display();

        ScaleMeasurement_t *measurement_ptr = NULL;
        ScaleMeasurementQueue.try_get_for(20ms, &measurement_ptr);

        if (measurement_ptr == NULL){
            continue;
        }

        ScaleMeasurement_t measurement;
        memcpy(&measurement, measurement_ptr, sizeof(ScaleMeasurement_t));
        ScaleMeasurementQueueMemoryPool.free(measurement_ptr);

        float error = _charge_weight_set_point - measurement.measurement;

        // Stop condition
        if (error < 0 || abs(error) < 0.03) {
            TricklerMotor->soft_stop();
            break;
        }

        // Update PID variables
        timer.stop();
        float elapse_time = timer.read();
        timer.start();

        integral += error;
        float derivative = (error - last_error) / elapse_time;
        last_error = error;

        float p_term = kp * error;
        float i_term = ki * integral;
        float d_term = kd * derivative;

        // Feed to motor
        unsigned int new_motor_speed = int(round(p_term + i_term + d_term));
        TricklerMotor->run(StepperMotor::FWD, new_motor_speed);
    }

    TricklerMotor->soft_hiz();

    // Make sure the display thread will pause
    while (ScaleWeightDisplayEnable.try_acquire()){}
    thread_sleep_for(500);

    return CHARGE_MODE_POWDER_TRICKLE_WAIT_FOR_CUP_REMOVAL;
}


TricklerState_t charge_mode_powder_trickle_wait_for_cup_removal(void){
    OLEDScreen.cls();
    _render_charge_mode_title();
    OLEDScreen.locate(0, 10);
    OLEDScreen.printf("Waiting for cup to be\nremoved\n");
    OLEDScreen.copy_to_lcd();

    FloatRingBuffer data_buffer(5);

    while (true) {
        ScaleMeasurement_t *measurement_ptr = NULL;
        ScaleMeasurementQueue.try_get_for(20ms, &measurement_ptr);

        if (measurement_ptr == NULL){
            continue;
        }

        ScaleMeasurement_t measurement;
        memcpy(&measurement, measurement_ptr, sizeof(ScaleMeasurement_t));
        ScaleMeasurementQueueMemoryPool.free(measurement_ptr);

        // Determine the removal of the cup by reading 5 with 0.5 apart, with relative stable reading and all 
        // measuring negative, then the cup is determined removed. 
        data_buffer.enqueue(measurement.measurement);
        if (data_buffer.getCounter() == 5){
            if (data_buffer.getSd() < cfg_cup_removal_sd_threshold && data_buffer.getMean() < 0){
                break;
            }
        }
    }

    return CHARGE_MODE_POWDER_TRICKLE_WAIT_FOR_CUP_RETURNED;
}


TricklerState_t charge_mode_powder_trickle_wait_for_cup_returned(void){
    OLEDScreen.cls();
    _render_charge_mode_title();
    OLEDScreen.locate(0, 10);
    OLEDScreen.printf("Waiting for cup to be\nreturned\n");
    OLEDScreen.copy_to_lcd();

    FloatRingBuffer data_buffer(5);

    while (true) {
        ScaleMeasurement_t *measurement_ptr = NULL;
        ScaleMeasurementQueue.try_get_for(20ms, &measurement_ptr);

        if (measurement_ptr == NULL){
            continue;
        }

        ScaleMeasurement_t measurement;
        memcpy(&measurement, measurement_ptr, sizeof(ScaleMeasurement_t));
        ScaleMeasurementQueueMemoryPool.free(measurement_ptr);

        // Determine the removal of the cup by reading 5 with 0.5s apart, with relative stable reading and all 
        // measuring negative, then the cup is determined removed. 
        data_buffer.enqueue(measurement.measurement);
        if (data_buffer.getCounter() == 5){
            if (data_buffer.getSd() < cfg_cup_returned_sd_threshold && 
                abs(data_buffer.getMean()) < cfg_cup_returned_zero_threshold){
                break;
            }
        }
    }

    return CHARGE_MODE_POWDER_THROW;
}


// TricklerState_t charge_mode_powder_trickle_wait_for_complete(void){
//     lcd.cls();
//     lcd.setCursorPosition(0, 0);
//     lcd.printf("Trickling...");

//     bool trickle_complete = false;

//     // PID related
//     Timer timer;
//     float max_on_time_ms = 500;
//     float min_on_time_ms = 140;

//     float max_integral = 10;

//     float kp = 480.0f;
//     // float ki = 6.0f;
//     // float kd = 400.0f;
//     float ki = 0.0f;
//     float kd = 0.0f;

//     float integral = 0.0f;
//     float last_error = 0.0;

//     timer.start();

//     while (!trickle_complete) {
//         ScaleMeasurement_t *measurement_ptr = NULL;
//         ScaleMeasurementQueue.try_get_for(20ms, &measurement_ptr);

//         if (measurement_ptr != NULL) {
//             // Make a copy of the measurement
//             ScaleMeasurement_t measurement;
//             memcpy(&measurement, measurement_ptr, sizeof(measurement));
//             ScaleMeasurementQueueMemoryPool.free(measurement_ptr);

//             float error = _charge_weight_set_point - measurement.measurement;

//             if (error < 0 || abs(error) < 0.02) {
//                 Trickler.write(0.0f);
//                 trickle_complete = true;
//                 continue;
//             }

//             timer.stop();
//             float elapse_time = timer.read();
//             timer.start();

//             integral += error;
//             float derivative = (error - last_error) / elapse_time;
//             last_error = error;

//             if (integral > max_integral){
//                 integral = max_integral;
//             }

//             // Final trickling stage
//             float p_term = kp * error;
//             float i_term = ki * integral;
//             float d_term = kd * derivative;

//             float new_on_time = p_term + i_term + d_term;

//             if (new_on_time > max_on_time_ms){
//                 new_on_time = max_on_time_ms;
//             }
//             else if (new_on_time < min_on_time_ms) {
//                 new_on_time = min_on_time_ms;
//             }

//             Trickler.write(0.23f);
//             thread_sleep_for(new_on_time);
//             Trickler.write(0);

//             // Enable weighting display. The display can be called while waiting for stable
//             lcdWeightPrintEnable.release();

//             if (error < 0.05){
//                 thread_sleep_for(1800);
//             }
//             else if (error < 0.1) {
//                 thread_sleep_for(1400);
//             }
//             else if (error < 0.5){
//                 thread_sleep_for(1000);
//             }
//             else{
//                 thread_sleep_for(500);
//             }

//             // printf("P=%f, I=%f, D=%f, CTRL=%f\r\n", p_term, i_term, d_term, new_on_time);
//         }
//     }

//     // Finish trickling, display final data
//     lcd.setCursorPosition(0, 0);
//     lcd.printf("Trickle Complete");

//     return CHARGE_MODE_POWDER_TRICKLE_WAIT_FOR_CUP_REMOVAL;
// }


// TricklerState_t charge_mode_powder_trickle_wait_for_cup_removal(void){

//     FloatRingBuffer data_buffer(5);

//     while (true) {
//         ScaleMeasurement_t *measurement_ptr = NULL;
//         ScaleMeasurementQueue.try_get_for(20ms, &measurement_ptr);

//         if (measurement_ptr) {
//             // Make a copy of the measurement
//             ScaleMeasurement_t measurement;
//             memcpy(&measurement, measurement_ptr, sizeof(measurement));
//             ScaleMeasurementQueueMemoryPool.free(measurement_ptr);

//             // Determine the removal of the cup by reading 5 with 0.5 apart, with relative stable reading and all 
//             // measuring negative, then the cup is determined removed. 
//             data_buffer.enqueue(measurement.measurement);
//             if (data_buffer.getCounter() == 5){
//                 if (data_buffer.getSd() < cfg_cup_removal_sd_threshold && data_buffer.getMean() < 0){
//                     break;
//                 }
//             }

//             // Enable weighting display
//             lcdWeightPrintEnable.release();

//             // Wait for next measurement
//             thread_sleep_for(500);
//         }
//     }

//     lcd.setCursorPosition(0, 0);
//     lcd.printf("Cup Removed     ");

//     return CHARGE_MODE_POWDER_TRICKLE_WAIT_FOR_CUP_RETURNED;
// }


// TricklerState_t charge_mode_powder_trickle_wait_for_cup_returned(void){
//     FloatRingBuffer data_buffer(5);

//     while (true) {
//         ScaleMeasurement_t *measurement_ptr = NULL;
//         ScaleMeasurementQueue.try_get_for(20ms, &measurement_ptr);

//         if (measurement_ptr) {
//             // Make a copy of the measurement
//             ScaleMeasurement_t measurement;
//             memcpy(&measurement, measurement_ptr, sizeof(measurement));
//             ScaleMeasurementQueueMemoryPool.free(measurement_ptr);

//             // Determine the removal of the cup by reading 5 with 0.5s apart, with relative stable reading and all 
//             // measuring negative, then the cup is determined removed. 
//             data_buffer.enqueue(measurement.measurement);
//             if (data_buffer.getCounter() == 5){
//                 if (data_buffer.getSd() < cfg_cup_returned_sd_threshold && 
//                     abs(data_buffer.getMean()) < cfg_cup_returned_zero_threshold){
//                     break;
//                 }
//             }

//             // Enable weighting display
//             lcdWeightPrintEnable.release();
//         }
//     }

//     // lcd.cls();
//     // lcd.setCursorPosition(0, 0);
//     // lcd.printf("Throwing...");

//     // ScaleSerial.write("Z\r\n", 3);
//     // ThisThread::sleep_for(1s);  // Zero takes about 1s to take in effect

//     return CHARGE_MODE_POWDER_THROW;
// }