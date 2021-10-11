#include <cstring>
#include "charge_mode.h"
#include "freetronicsLCDShield.h"
#include "Stepper.h"
#include "mbed.h"
#include "FloatRingBuffer.h"


// Invoke peripheral declared in main
extern freetronicsLCDShield lcd;
extern Stepper StepMotor;
extern Semaphore lcdWeightPrintEnable;
extern BufferedSerial ScaleSerial;
extern Queue<ScaleMeasurement_t, 1> ScaleMeasurementQueue;
extern MemoryPool<ScaleMeasurement_t, 2> ScaleMeasurementQueueMemoryPool;
extern PwmOut Trickler;

extern int cfg_powder_measure_discharge_angle_step;


// Local static variables
static char _charge_weight_string[6];
static int _cursor_loc = 0;
static float _charge_weight_set_point = 0;
const float cfg_cup_removal_sd_threshold = 5;
const float cfg_cup_returned_sd_threshold = 0.02;
const float cfg_cup_returned_zero_threshold = 0.04;


TricklerState_t charge_mode_select_weight(void){
    lcd.cls();
    lcd.setCursorPosition(0, 0);
    lcd.printf("Select Weight");

    lcd.setCursorPosition(1, 0);
    lcd.printf("00.00 gn");
    memset(_charge_weight_string, '0', sizeof(_charge_weight_string));

    lcd.setCursor(true, false);
    lcd.setCursorPosition(1, 0);

    return CHARGE_MODE_SELECT_WEIGHT_WAIT_FOR_INPUT;
}

TricklerState_t charge_mode_select_weight_wait_for_input(freetronicsLCDShield::ButtonType_t *button_press){
    TricklerState_t next_state = CHARGE_MODE_SELECT_WEIGHT_WAIT_FOR_INPUT;
    if (*button_press == freetronicsLCDShield::BTN_LEFT){
        _cursor_loc -= 1;
        if (_cursor_loc < 0){
            _cursor_loc = 4;
        }
        else if (_cursor_loc == 2){
            // Skip the dot
            _cursor_loc = 1;
        }
    }
    else if (*button_press == freetronicsLCDShield::BTN_RIGHT) {
        _cursor_loc += 1;
        if (_cursor_loc > 4){
            _cursor_loc = 0;
        }
        else if (_cursor_loc == 2){
            // Skip the dot
            _cursor_loc = 3;
        }
    }
    else if (*button_press == freetronicsLCDShield::BTN_UP){
        _charge_weight_string[_cursor_loc] += 1;
        if (_charge_weight_string[_cursor_loc] > '9'){
            _charge_weight_string[_cursor_loc] = '0';
        }
    }
    else if (*button_press == freetronicsLCDShield::BTN_DOWN){
        _charge_weight_string[_cursor_loc] -= 1;
        if (_charge_weight_string[_cursor_loc] < '0'){
            _charge_weight_string[_cursor_loc] = '9';
        }
    }
    else if (*button_press == freetronicsLCDShield::BTN_SELECT){
        // Turn charge weight to trickler setpoint
        _charge_weight_string[2] = '.';
        _charge_weight_set_point = float(atof(_charge_weight_string));
        printf("Charge Weight Setpoint: %f\r\n", _charge_weight_set_point);

        // disable cursor from now on
        lcd.setCursor(false, false);
        next_state = CHARGE_MODE_POWDER_THROW;
    }
    
    // Display current cursor
    lcd.setCursorPosition(1, _cursor_loc);
    lcd.printf("%c", _charge_weight_string[_cursor_loc]);
    // printf will auto shift 
    lcd.setCursorPosition(1, _cursor_loc);

    return next_state;
}


TricklerState_t charge_mode_powder_throw(void){
    lcd.cls();
    lcd.setCursorPosition(0, 0);
    lcd.printf("Present Cup");

    lcd.setCursorPosition(1, 0);
    lcd.printf("> Press Select");

    return CHARGE_MODE_POWDER_THROW_WAIT_FOR_INPUT;
}


TricklerState_t charge_mode_powder_throw_wait_for_input(freetronicsLCDShield::ButtonType_t *button_press){
    TricklerState_t next_state = CHARGE_MODE_POWDER_THROW_WAIT_FOR_INPUT;

    if (*button_press == freetronicsLCDShield::BTN_SELECT){
        lcd.cls();
        lcd.setCursorPosition(0, 0);
        lcd.printf("Throwing...");

        ScaleSerial.write("Z\r\n", 3);
        ThisThread::sleep_for(1s);  // Zero takes about 1s to take in effect

        // Show weight after trickle
        lcdWeightPrintEnable.release();

        next_state = CHARGE_MODE_POWDER_THROW_WAIT_FOR_COMPLETE;
    }

    return next_state;
}


TricklerState_t charge_mode_powder_throw_wait_for_complete(void){
    StepMotor.enableHold(true);
    StepMotor.step(cfg_powder_measure_discharge_angle_step);
    ThisThread::sleep_for(500ms);
    StepMotor.step(-cfg_powder_measure_discharge_angle_step);
    ThisThread::sleep_for(500ms);
    StepMotor.enableHold(false);
    
    // Show weight after trickle
    lcdWeightPrintEnable.release();

    return CHARGE_MODE_POWDER_TRICKLE_WAIT_FOR_COMPLETE;
}


TricklerState_t charge_mode_powder_trickle_wait_for_complete(void){
    lcd.cls();
    lcd.setCursorPosition(0, 0);
    lcd.printf("Trickling...");

    bool trickle_complete = false;

    // PID related
    Timer timer;
    float max_on_time_ms = 500;
    float min_on_time_ms = 140;

    float max_integral = 10;

    float kp = 480.0f;
    // float ki = 6.0f;
    // float kd = 400.0f;
    float ki = 0.0f;
    float kd = 0.0f;

    float integral = 0.0f;
    float last_error = 0.0;

    timer.start();

    while (!trickle_complete) {
        ScaleMeasurement_t *measurement_ptr = NULL;
        ScaleMeasurementQueue.try_get_for(20ms, &measurement_ptr);

        if (measurement_ptr != NULL) {
            // Make a copy of the measurement
            ScaleMeasurement_t measurement;
            memcpy(&measurement, measurement_ptr, sizeof(measurement));
            ScaleMeasurementQueueMemoryPool.free(measurement_ptr);

            float error = _charge_weight_set_point - measurement.measurement;

            if (error < 0 || abs(error) < 0.02) {
                Trickler.write(0.0f);
                trickle_complete = true;
                continue;
            }

            timer.stop();
            float elapse_time = timer.read();
            timer.start();

            integral += error;
            float derivative = (error - last_error) / elapse_time;
            last_error = error;

            if (integral > max_integral){
                integral = max_integral;
            }

            // Final trickling stage
            float p_term = kp * error;
            float i_term = ki * integral;
            float d_term = kd * derivative;

            float new_on_time = p_term + i_term + d_term;

            if (new_on_time > max_on_time_ms){
                new_on_time = max_on_time_ms;
            }
            else if (new_on_time < min_on_time_ms) {
                new_on_time = min_on_time_ms;
            }

            Trickler.write(0.23f);
            thread_sleep_for(new_on_time);
            Trickler.write(0);

            // Enable weighting display. The display can be called while waiting for stable
            lcdWeightPrintEnable.release();

            if (error < 0.05){
                thread_sleep_for(1800);
            }
            else if (error < 0.1) {
                thread_sleep_for(1400);
            }
            else if (error < 0.5){
                thread_sleep_for(1000);
            }
            else{
                thread_sleep_for(500);
            }

            // printf("P=%f, I=%f, D=%f, CTRL=%f\r\n", p_term, i_term, d_term, new_on_time);
        }
    }

    // Finish trickling, display final data
    lcd.setCursorPosition(0, 0);
    lcd.printf("Trickle Complete");

    return CHARGE_MODE_POWDER_TRICKLE_WAIT_FOR_CUP_REMOVAL;
}


TricklerState_t charge_mode_powder_trickle_wait_for_cup_removal(void){

    FloatRingBuffer data_buffer(5);

    while (true) {
        ScaleMeasurement_t *measurement_ptr = NULL;
        ScaleMeasurementQueue.try_get_for(20ms, &measurement_ptr);

        if (measurement_ptr) {
            // Make a copy of the measurement
            ScaleMeasurement_t measurement;
            memcpy(&measurement, measurement_ptr, sizeof(measurement));
            ScaleMeasurementQueueMemoryPool.free(measurement_ptr);

            // Determine the removal of the cup by reading 5 with 0.5 apart, with relative stable reading and all 
            // measuring negative, then the cup is determined removed. 
            data_buffer.enqueue(measurement.measurement);
            if (data_buffer.getCounter() == 5){
                if (data_buffer.getSd() < cfg_cup_removal_sd_threshold && data_buffer.getMean() < 0){
                    break;
                }
            }

            // Enable weighting display
            lcdWeightPrintEnable.release();

            // Wait for next measurement
            thread_sleep_for(500);
        }
    }

    lcd.setCursorPosition(0, 0);
    lcd.printf("Cup Removed     ");

    return CHARGE_MODE_POWDER_TRICKLE_WAIT_FOR_CUP_RETURNED;
}


TricklerState_t charge_mode_powder_trickle_wait_for_cup_returned(void){
    FloatRingBuffer data_buffer(5);

    while (true) {
        ScaleMeasurement_t *measurement_ptr = NULL;
        ScaleMeasurementQueue.try_get_for(20ms, &measurement_ptr);

        if (measurement_ptr) {
            // Make a copy of the measurement
            ScaleMeasurement_t measurement;
            memcpy(&measurement, measurement_ptr, sizeof(measurement));
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

            // Enable weighting display
            lcdWeightPrintEnable.release();
        }
    }

    // lcd.cls();
    // lcd.setCursorPosition(0, 0);
    // lcd.printf("Throwing...");

    // ScaleSerial.write("Z\r\n", 3);
    // ThisThread::sleep_for(1s);  // Zero takes about 1s to take in effect

    return CHARGE_MODE_POWDER_THROW;
}