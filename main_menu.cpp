#include "app.h"
#include "freetronicsLCDShield.h"
#include "Stepper.h"
#include "mbed.h"
#include "main_menu.h"


// Invoke peripheral declared in main
extern freetronicsLCDShield lcd;


// Configs and control variables
const char *cfg_main_menu_items[] = {
    "Charge Mode",
    "Cleanup Mode",
    "Calibrate Mode",
    NULL,
};

static int _main_menu_item_count = 0;  // Starting with invalid count
static int _current_main_menu_selection = 0;


void _render_menu(void){
    // Calculate the number if items from the main menu. This will only execute once
    if (_main_menu_item_count == 0){
        while (cfg_main_menu_items[++_main_menu_item_count] != NULL);
    }

    int next_main_menu_selection = _current_main_menu_selection + 1;
    if (next_main_menu_selection >= _main_menu_item_count){
        next_main_menu_selection = 0;
    }

    lcd.cls();
    lcd.setCursorPosition(0, 0);
    lcd.printf("> %s", cfg_main_menu_items[_current_main_menu_selection]);
    lcd.setCursorPosition(1, 0);
    lcd.printf("%s", cfg_main_menu_items[next_main_menu_selection]);
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
        if (_current_main_menu_selection == 0) {
            next_state = CHARGE_MODE_SELECT_WEIGHT;
        }
        else if (_current_main_menu_selection == 1){
            next_state = CLEANUP_MODE_MENU;
        }
        else if (_current_main_menu_selection == 2){
            next_state = CALIBRATION_MODE_MENU;
        }
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