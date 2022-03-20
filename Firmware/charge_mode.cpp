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
extern L6470 *CoarseTricklerMotor;
extern void thrower_discharge(void (*cb)(int)=NULL, bool wait=true);
extern void thrower_charge(void (*cb)(int)=NULL, bool wait=true);
extern int cfg_thrower_microstepping;
extern int cfg_coarse_trickler_microstepping;

// Local static variables
static char _charge_weight_string[6];
static int _cursor_loc = 0;
static float _charge_weight_set_point = 0;
static unsigned int latched_fine_trickler_speed = 0;
static unsigned int latched_coarse_trickler_speed = 0;
static float latched_flow_rate = 0.0f;

float cfg_cup_removal_sd_threshold = 5;
float cfg_cup_returned_sd_threshold = 0.02;
float cfg_cup_returned_zero_threshold = 0.04;
CoarseMode_e cfg_coarse_mode = COARSE_MODE_USE_COARSE_TRICKLER;

int cfg_scale_sampling_period_ms = 100;

float cfg_fine_trickler_kp = 200.0f;
float cfg_fine_trickler_ki = 0.0f;
float cfg_fine_trickler_kd = 150.0f;

float cfg_coarse_trickler_kp = 4.5f;
float cfg_coarse_trickler_ki = 0.0f;
float cfg_coarse_trickler_kd = 250.0f;

int cfg_thrower_motor_max_speed = 1500;

int cfg_fine_trickler_max_speed = 400;
int cfg_fine_trickler_min_speed = 10;

int cfg_coarse_trickler_max_speed = 500;
int cfg_coarse_trickler_min_speed = 5;


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

    // Display the current motor speed
    OLEDScreen.locate(0, 10);
    OLEDScreen.printf("M1:%d M2:%d", latched_fine_trickler_speed, latched_coarse_trickler_speed);

    // Display flow rate
    OLEDScreen.locate(0, 20);
    OLEDScreen.printf("Flow Rate: %0.2f gn/s", latched_flow_rate);
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

            if (cfg_coarse_mode == COARSE_MODE_USE_COARSE_TRICKLER){
                next_state = CHARGE_MODE_COARSE_TRICKLE;
            }
            else if (cfg_coarse_mode == COARSE_MODE_USE_POWDER_MEASURE){
                next_state = CHARGE_MODE_POWDER_THROW;
            }
            else {
                printf("Undefined cfg_coarse_mode %d\r\n", cfg_coarse_mode);
            }
            
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
    /* 
    * Wait for ZERO 
    */
    ScaleSerial.write("Z\r\n", 3);

    while (true) {
        ScaleMeasurement_t *measurement_ptr = NULL;
        ScaleMeasurementQueue.try_get_for(20ms, &measurement_ptr);

        if (measurement_ptr == NULL){
            continue;
        }

        ScaleMeasurement_t measurement;
        memcpy(&measurement, measurement_ptr, sizeof(ScaleMeasurement_t));
        ScaleMeasurementQueueMemoryPool.free(measurement_ptr);

        if (measurement.measurement == 0.0f){
            break;
        }
    }

    /*
     * Start working on throwing from now on
     */
    ThrowerMotor->set_max_speed(cfg_thrower_motor_max_speed);

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


TricklerState_t charge_mode_coarse_trickle(void){
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
            next_state = CHARGE_MODE_ZERO_SCALE;
        }
    }
    return next_state;
}

TricklerState_t charge_mode_zero_scale(void){
    // Start display thread
    if (ScaleWeightDisplayThread.get_state() != Thread::Running){
        ScaleWeightDisplayThread.start(scale_weight_display_handler);
    }

    /* 
    * Wait for ZERO 
    */
    if (latched_scale_measurement.measurement != 0.0f) {
        ScaleSerial.write("Z\r\n", 3);

        while (true) {
            ScaleMeasurement_t *measurement_ptr = NULL;
            ScaleMeasurementQueue.try_get_for(20ms, &measurement_ptr);

            if (measurement_ptr == NULL){
                continue;
            }

            ScaleMeasurement_t measurement;
            memcpy(&measurement, measurement_ptr, sizeof(ScaleMeasurement_t));
            ScaleMeasurementQueueMemoryPool.free(measurement_ptr);

            if (measurement.measurement == 0.0f){
                break;
            }
        }
    }

    return CHARGE_MODE_POWDER_TRICKLE_WAIT_FOR_COMPLETE;
}


typedef enum{
    COARSE_TRICKLER_MOVE,
    COARSE_TRICKLER_STANDBY,
} CoarseTricklerMode_e;


TricklerState_t charge_mode_powder_trickle_wait_for_complete(void){
    TricklerMotor->set_max_speed(cfg_fine_trickler_max_speed);
    TricklerMotor->set_min_speed(cfg_fine_trickler_min_speed);

    CoarseTricklerMotor->set_max_speed(cfg_coarse_trickler_max_speed);
    CoarseTricklerMotor->set_min_speed(cfg_coarse_trickler_min_speed);
    // PID related
    int next_sampling_time = 0;
    float integral = 0.0f;
    float last_error = 0.0f;
    CoarseTricklerMode_e coarse_trickler_mode = COARSE_TRICKLER_MOVE;

    Timer timer;
    timer.start();

    while (true){
        int time_ms = timer.read_ms();
        float current_weight = latched_scale_measurement.measurement;
        float error = _charge_weight_set_point - current_weight;

        // Stop condition
        if (error < 0 || abs(error) < 0.03) {
            TricklerMotor->soft_hiz();
            CoarseTricklerMotor->soft_hiz();
            latched_fine_trickler_speed = 0;
            latched_coarse_trickler_speed = 0;
            break;
        }

        if (abs(error) < 5.0f && coarse_trickler_mode == COARSE_TRICKLER_MOVE){
            CoarseTricklerMotor->hard_stop();
            CoarseTricklerMotor->wait_while_active();
            CoarseTricklerMotor->set_max_speed(200);
            CoarseTricklerMotor->move(StepperMotor::BWD, 60 * cfg_coarse_trickler_microstepping);
            coarse_trickler_mode = COARSE_TRICKLER_STANDBY;
            latched_coarse_trickler_speed = 0;
        }

        if (time_ms > next_sampling_time){
            next_sampling_time += cfg_scale_sampling_period_ms;
            
            // Update PID variables
            float elapse_time = cfg_scale_sampling_period_ms;

            integral += error;
            float derivative = (error - last_error) / elapse_time;
            
            float fine_trickler_p_term = cfg_fine_trickler_kp * error;
            float fine_trickler_i_term = cfg_fine_trickler_ki * integral;
            float fine_trickler_d_term = cfg_fine_trickler_kd * derivative;

            unsigned int new_fine_motor_speed = min(int(round(fine_trickler_p_term + fine_trickler_i_term + fine_trickler_d_term)), 
                                                    cfg_fine_trickler_max_speed);
            TricklerMotor->run(StepperMotor::FWD, new_fine_motor_speed);
            latched_fine_trickler_speed = new_fine_motor_speed;
            

            if (coarse_trickler_mode == COARSE_TRICKLER_MOVE) {
                float coarse_trickler_p_term = cfg_coarse_trickler_kp * error;
                float coarse_trickler_i_term = cfg_coarse_trickler_ki * integral;
                float coarse_trickler_d_term = cfg_coarse_trickler_kd * derivative;

                unsigned int new_coarse_motor_speed = min(int(round(coarse_trickler_p_term + coarse_trickler_i_term + coarse_trickler_d_term)),
                                                          cfg_coarse_trickler_max_speed);
                CoarseTricklerMotor->run(StepperMotor::FWD, new_coarse_motor_speed);
                latched_coarse_trickler_speed = new_coarse_motor_speed;
            }


            latched_flow_rate = (last_error - error) / float(cfg_scale_sampling_period_ms) * 1000.0;
            last_error = error;

            _refresh_scale_weight_display();
        }
        else {
            _refresh_scale_weight_display();
            thread_sleep_for(1);
        }
    }

    TricklerMotor->soft_hiz();
    CoarseTricklerMotor->soft_hiz();

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

    // TODO: goes to a state which will determine the coarse trickle state
    TricklerState_t next_state = UNDEFINED_STATE;
    if (cfg_coarse_mode == COARSE_MODE_USE_COARSE_TRICKLER){
        next_state = CHARGE_MODE_COARSE_TRICKLE;
    }
    else if (cfg_coarse_mode == COARSE_MODE_USE_POWDER_MEASURE){
        next_state = CHARGE_MODE_POWDER_THROW;
    }
    else {
        printf("Undefined cfg_coarse_mode %d\r\n", cfg_coarse_mode);
    }
    return next_state;
}
