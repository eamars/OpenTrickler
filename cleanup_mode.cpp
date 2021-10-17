#include "app.h"
#include "mbed.h"
#include "cleanup_mode.h"
#include "SH1106.h"
#include "Terminal6x8.h"
#include "Arial12x12.h"
#include "L6470.h"
#include "math.h"
#include <cmath>    


// Invoke peripheral declared in main
extern SH1106 OLEDScreen;
extern Queue<char, 1> ButtonQueue;
extern MemoryPool<char, 2> ButtonQueueMemoryPool;
extern L6470 *ThrowerMotor;
extern void thrower_dispense(uint32_t stay_ms=250, void (*cb)(int)=NULL);
extern void thrower_discharge(void (*cb)(int)=NULL);
extern void thrower_charge(void (*cb)(int)=NULL);

// Configs and control variables
const extern int cfg_thrower_microstepping;
const int cfg_discharge_cycles = 5;
static int _current_discharge_cycle_count = 1;

void _render_cleanup_mode_title(){
    OLEDScreen.locate(0, 0);
    OLEDScreen.set_font((unsigned char *) Terminal6x8);

    OLEDScreen.background(White);
    OLEDScreen.foreground(Black);
    OLEDScreen.printf("      Cleanup Mode     \n");
    OLEDScreen.background(Black);
    OLEDScreen.foreground(White);
}

TricklerState_t cleanup_mode_menu(void){
    OLEDScreen.cls();
    _render_cleanup_mode_title();

    OLEDScreen.locate(0, 10);
    OLEDScreen.printf("Please close powder\nhopper valve\n");

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
            next_state = MAIN_MENU;
        }
        else if (button == 'C') {
            next_state = CLEANUP_MODE_WAIT_FOR_COMPLETE;
        }
    }

    return next_state;
}


void _render_dispenser_position(int x0, int y0, int r, int motor_position){
    // Draw gauge
    OLEDScreen.circle(x0, y0, r, White);
    OLEDScreen.circle(x0, y0, r - 2, White);
    
    float angle = (float) motor_position / cfg_thrower_microstepping / 200 * 2 * 3.14;

    float x1 = sin(angle) * r + x0;
    float y1 = cos(angle) * r + y0;

    // Draw needle
    OLEDScreen.line(x0, y0, x1, y1, White);
}

void _render_discharge(int position){
    OLEDScreen.cls();

    _render_cleanup_mode_title();

    _render_dispenser_position(61, 30, 20, position);

    OLEDScreen.set_font((unsigned char *) Arial12x12);
    OLEDScreen.locate(35, 51);
    OLEDScreen.printf("Discharge");
    OLEDScreen.copy_to_lcd();
}

void _render_charge(int position){
    OLEDScreen.cls();

    _render_cleanup_mode_title();

    _render_dispenser_position(61, 30, 20, position);

    OLEDScreen.set_font((unsigned char *) Arial12x12);
    OLEDScreen.locate(35, 51);
    OLEDScreen.printf("Charge");
    OLEDScreen.copy_to_lcd();
}


TricklerState_t cleanup_mode_wait_for_complete(void){
    // TODO: use scale to determine whether the powder dispenser is empty
    while (_current_discharge_cycle_count <= cfg_discharge_cycles){

        thrower_discharge(_render_discharge);
        thread_sleep_for(250);
        thrower_charge(_render_charge);
        thread_sleep_for(250);

        _current_discharge_cycle_count += 1;
    }

    // Discharge complete
    _current_discharge_cycle_count = 1;

    // Disable hold
    ThrowerMotor->soft_hiz();

    OLEDScreen.cls();
    _render_cleanup_mode_title();
    OLEDScreen.locate(0, 10);
    OLEDScreen.printf("Discharge Complete\n");
    OLEDScreen.copy_to_lcd();
    ThisThread::sleep_for(1s);
    
    return MAIN_MENU;
}
