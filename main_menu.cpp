#include "app.h"
#include "freetronicsLCDShield.h"
#include "Stepper.h"
#include "mbed.h"
#include "main_menu.h"


// Invoke peripheral declared in main
extern freetronicsLCDShield lcd;


// Configs and control variables
// const char *cfg_main_menu_items[] = {
//     "Charge Mode",
//     "Cleanup Mode",
//     "Calibrate Mode",
//     "Scale Setup Mode",
//     NULL,
// };

typedef struct {
    char menu_name[15];  // includes \0
    TricklerState_t state;
} main_menu_item_t;

const main_menu_item_t cfg_main_menu_items[] = {
    {"Charge", CHARGE_MODE_SELECT_WEIGHT},
    {"Cleanup", CLEANUP_MODE_MENU},
    {"Calibrate", CALIBRATION_MODE_MENU},
    {"Scale Setup", SCALE_SETUP_MODE_MENU},
};

const int _main_menu_item_count = sizeof(cfg_main_menu_items) / sizeof(main_menu_item_t);
static int _current_main_menu_selection = 0;


void _render_menu(void){
    int next_main_menu_selection = _current_main_menu_selection + 1;
    if (next_main_menu_selection >= _main_menu_item_count){
        next_main_menu_selection = 0;
    }

    lcd.cls();
    lcd.setCursorPosition(0, 0);
    lcd.printf("> %s", cfg_main_menu_items[_current_main_menu_selection].menu_name);
    lcd.setCursorPosition(1, 0);
    lcd.printf("%s", cfg_main_menu_items[next_main_menu_selection].menu_name);
}


TricklerState_t main_menu(void){
    _render_menu();

    return MAIN_MENU_WAIT_FOR_INPUT;
}


TricklerState_t main_menu_wait_for_input(freetronicsLCDShield::ButtonType_t *button_press){
    TricklerState_t next_state = MAIN_MENU_WAIT_FOR_INPUT;

    if (*button_press == freetronicsLCDShield::BTN_UP){
        _current_main_menu_selection -= 1;
        if (_current_main_menu_selection < 0){
            _current_main_menu_selection = _main_menu_item_count - 1;
        }
    }
    else if (*button_press == freetronicsLCDShield::BTN_DOWN){
        _current_main_menu_selection += 1;
        if (_current_main_menu_selection >= _main_menu_item_count){
            _current_main_menu_selection = 0;
        }
    }
    else if (*button_press == freetronicsLCDShield::BTN_SELECT){
        next_state = cfg_main_menu_items[_current_main_menu_selection].state;
    }
    
    // Roll over
    int next_main_menu_selection = _current_main_menu_selection + 1;
    if (next_main_menu_selection >=_main_menu_item_count){
        next_main_menu_selection = 0;
    }

    // Display current mode selection
    _render_menu();

    return next_state;
}