#include "app.h"
#include "SH1106.h"
#include "mbed.h"
#include "main_menu.h"
#include "Terminal6x8.h"

#define MENU_LENGTH sizeof(cfg_main_menu_items) / sizeof(main_menu_item_t)


// Invoke peripheral declared in main
extern SH1106 OLEDScreen;
extern Queue<char, 1> ButtonQueue;
extern MemoryPool<char, 2> ButtonQueueMemoryPool;


typedef struct {
    char menu_name[22];  // includes \0
    TricklerState_t state;
} main_menu_item_t;

const main_menu_item_t cfg_main_menu_items[] = {
    {"Charge Mode", CHARGE_MODE_SELECT_WEIGHT},
    {"Cleanup Mode", CLEANUP_MODE_MENU},
    {"Calibrate Mode", CALIBRATION_MODE_MENU},
    {"Configuration", SCALE_SETUP_MODE_MENU},
};

const int _main_menu_item_count = sizeof(cfg_main_menu_items) / sizeof(main_menu_item_t);
static int _current_main_menu_selection = 0;


void _render_menu(void){
    OLEDScreen.cls();
    OLEDScreen.locate(0, 0);
    OLEDScreen.set_font((unsigned char *) Terminal6x8);
    // Iterate over all menu items
    for (int idx=0; idx < MENU_LENGTH; idx++){
        if (idx == _current_main_menu_selection){
            OLEDScreen.background(White);
            OLEDScreen.foreground(Black);
        }
        else{
            OLEDScreen.background(Black);
            OLEDScreen.foreground(White);
        }
        const char *text = cfg_main_menu_items[idx].menu_name;
        OLEDScreen.printf("%s", text);

        // Append space
        for (int i = 0; i < 22 - strlen(text); i++){
            OLEDScreen.printf("%c", ' ');
        }
        OLEDScreen.printf("%c", '\n');
    }

    OLEDScreen.copy_to_lcd();
}


TricklerState_t main_menu(void){
    _render_menu();

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
            if (_current_main_menu_selection >= _main_menu_item_count){
                _current_main_menu_selection = 0;
            }
        }
        else if (button == 'F'){
            _current_main_menu_selection -= 1;
            if (_current_main_menu_selection < 0){
                _current_main_menu_selection = _main_menu_item_count - 1;
            }
        }

        _render_menu();
    }

    return next_state;
}
