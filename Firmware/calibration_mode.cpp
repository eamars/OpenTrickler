#include "mbed.h"
#include "calibration_mode.h"
#include "menu.h"
#include "SH1106.h"
#include "L6470.h"


#define M_PI 3.1415926535897932
#define MENU_LENGTH sizeof(cfg_calibration_mode_menu_items) / sizeof(MenuItem_t)


// Invoke peripheral declared in main
extern SH1106 OLEDScreen;
extern Queue<char, 1> ButtonQueue;
extern MemoryPool<char, 2> ButtonQueueMemoryPool;
extern L6470 *TricklerMotor;
extern L6470 *CoarseTricklerMotor;
extern BufferedSerial ScaleSerial;

extern Queue<ScaleMeasurement_t, 1> ScaleMeasurementQueue;
extern MemoryPool<ScaleMeasurement_t, 2> ScaleMeasurementQueueMemoryPool;

extern const int cfg_fine_trickler_max_speed;
extern const int cfg_fine_trickler_min_speed;

extern const int cfg_coarse_trickler_max_speed;
extern const int cfg_coarse_trickler_min_speed;

extern const int cfg_scale_sampling_period_ms;

extern ScaleMeasurement_t latched_scale_measurement;


// Configuraitons
const int cfg_pid_tuning_repeat = 3;


const MenuItem_t cfg_calibration_mode_menu_items[] = 
{
    {"Cal Fine Trickler", CALIBRATION_MODE_TUNE_FINE_TRICKLER},
    {"Cal Coarse Trickler", CALIBRATION_MODE_TUNE_COARSE_TRICKLER},   
};

static int _current_menu_selection = 0;


TricklerState_t calibration_mode_menu(void)
{
    render_menu(cfg_calibration_mode_menu_items, MENU_LENGTH, _current_menu_selection);

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

        if (button == 'C') {
            next_state = cfg_calibration_mode_menu_items[_current_menu_selection].state;
        }
        else if (button == 'D') {
            next_state = MAIN_MENU;
        }
        else if (button == 'E'){
            _current_menu_selection += 1;
            if (_current_menu_selection >= MENU_LENGTH){
                _current_menu_selection = 0;
            }
        }
        else if (button == 'F'){
            _current_menu_selection -= 1;
            if (_current_menu_selection < 0){
                _current_menu_selection = MENU_LENGTH - 1;
            }
        }

        render_menu(cfg_calibration_mode_menu_items, MENU_LENGTH, _current_menu_selection);
    }

    return next_state;
}


// Look for Ku, Ti and Td for Ziegler-Nichols tuning method
TricklerState_t calibration_mode_tune_fine_trickler(void)
{
    // Control variables
    float charge_weight_set_point = 3.00f;

    float kp_sum = 0;
    float ki_sum = 0;
    float kd_sum = 0;
    
    for (int repeat_count = 0; repeat_count < cfg_pid_tuning_repeat; repeat_count++){
        /* 
         * Wait for ZERO 
         */
        ScaleSerial.write("Z\r\n", 3);

        printf("Waiting for scale to ZERO\r\n");
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
        printf("Scale is ZERO now\r\n");

        /* 
         * Start Tuning
         */
        float next_sampling_time = 0;
        float raise_time = 0;

        Timer timer;
        timer.start();

        // Drive the fine trickler motor to max speed, then wait for measured weight to exceed the set point
        TricklerMotor->run(StepperMotor::FWD, 100);

        while (true){
            int time_ms = timer.read_ms();

            if (time_ms > next_sampling_time){
                next_sampling_time += cfg_scale_sampling_period_ms;
            
                float current_charge_weight = latched_scale_measurement.measurement;

                // When the current weight exceeds the set point then stop the motor immediately
                if (current_charge_weight >= charge_weight_set_point){
                    TricklerMotor->soft_hiz();
                    raise_time = time_ms;
                    break;
                }
            }
            else {
                thread_sleep_for(1);
            }
        }

        // Calculate Ku
        float ku = (4.0 * ((cfg_fine_trickler_max_speed - cfg_fine_trickler_min_speed) / 2.0f)) / (M_PI * latched_scale_measurement.measurement / 2.0f);
        float tu = raise_time;
        
        float kp = 0.8f * ku;
        float ki = 0;
        float kd = (0.1 * kp * tu) / cfg_scale_sampling_period_ms;
        
        printf("Kp=%f, Ki=%f, Kd=%f\r\n", kp, ki, kd);
        kp_sum += kp;
        ki_sum += ki;
        kd_sum += kd;
    }

    float averaged_kp = kp_sum / cfg_pid_tuning_repeat;
    float averaged_ki = ki_sum / cfg_pid_tuning_repeat;
    float averaged_kd = kd_sum / cfg_pid_tuning_repeat;

    printf("Averaged Kp=%f, Ki=%f, Kd=%f\r\n", averaged_kp, averaged_ki, averaged_kd);

    return CALIBRATION_MODE_MENU;
}


// Look for Ku, Ti and Td for Ziegler-Nichols tuning method
TricklerState_t calibration_mode_tune_coarse_trickler(void)
{
    // Control variables
    float charge_weight_set_point = 15.00f;

    float kp_sum = 0;
    float ki_sum = 0;
    float kd_sum = 0;
    
    for (int repeat_count = 0; repeat_count < cfg_pid_tuning_repeat; repeat_count++){
        /* 
         * Wait for ZERO 
         */
        ScaleSerial.write("Z\r\n", 3);

        printf("Waiting for scale to ZERO\r\n");
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
        printf("Scale is ZERO now\r\n");

        /* 
         * Start Tuning
         */
        float next_sampling_time = 0;
        float raise_time = 0;

        Timer timer;
        timer.start();

        // Drive the fine trickler motor to max speed, then wait for measured weight to exceed the set point
        CoarseTricklerMotor->run(StepperMotor::FWD, 100);

        while (true){
            int time_ms = timer.read_ms();

            if (time_ms > next_sampling_time){
                next_sampling_time += cfg_scale_sampling_period_ms;
            
                float current_charge_weight = latched_scale_measurement.measurement;

                // When the current weight exceeds the set point then stop the motor immediately
                if (current_charge_weight >= charge_weight_set_point){
                    CoarseTricklerMotor->soft_hiz();
                    raise_time = time_ms;
                    break;
                }
            }
            else {
                thread_sleep_for(1);
            }
        }

        // Calculate Ku
        float ku = (4.0 * ((cfg_fine_trickler_max_speed - cfg_fine_trickler_min_speed) / 2.0f)) / (M_PI * latched_scale_measurement.measurement / 2.0f);
        float tu = raise_time;
        
        float kp = 0.8f * ku;
        float ki = 0;
        float kd = (0.1 * kp * tu) / cfg_scale_sampling_period_ms;
        
        printf("Kp=%f, Ki=%f, Kd=%f\r\n", kp, ki, kd);
        kp_sum += kp;
        ki_sum += ki;
        kd_sum += kd;
    }

    float averaged_kp = kp_sum / cfg_pid_tuning_repeat;
    float averaged_ki = ki_sum / cfg_pid_tuning_repeat;
    float averaged_kd = kd_sum / cfg_pid_tuning_repeat;

    printf("Averaged Kp=%f, Ki=%f, Kd=%f\r\n", averaged_kp, averaged_ki, averaged_kd);

    return CALIBRATION_MODE_MENU;
}