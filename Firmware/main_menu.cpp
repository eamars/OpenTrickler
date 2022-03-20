#include "app.h"
#include "mbed.h"
#include "main_menu.h"
#include "menu.h"

#define MENU_LENGTH sizeof(cfg_main_menu_items) / sizeof(MenuItem_t)


// Invoke peripheral declared in main
extern Queue<char, 1> ButtonQueue;
extern MemoryPool<char, 2> ButtonQueueMemoryPool;


const MenuItem_t cfg_main_menu_items[] = {
    {"Charge Mode", CHARGE_MODE_SELECT_WEIGHT},
    {"Cleanup Thrower", CLEANUP_THROWER},
    {"Cleanup Trickler", CLEANUP_TRICKLER},
    {"Calibration Mode", CALIBRATION_MODE_MENU},
    {"Configuration", SCALE_SETUP_MODE_MENU},
};

static int _current_main_menu_selection = 0;



TricklerState_t main_menu(void){
    render_menu(cfg_main_menu_items, MENU_LENGTH, _current_main_menu_selection);

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
            next_state = cfg_main_menu_items[_current_main_menu_selection].state;
        }
        else if (button == 'E'){
            _current_main_menu_selection += 1;
            if (_current_main_menu_selection >= MENU_LENGTH){
                _current_main_menu_selection = 0;
            }
        }
        else if (button == 'F'){
            _current_main_menu_selection -= 1;
            if (_current_main_menu_selection < 0){
                _current_main_menu_selection = MENU_LENGTH - 1;
            }
        }

        render_menu(cfg_main_menu_items, MENU_LENGTH, _current_main_menu_selection);
    }

    return next_state;
}
