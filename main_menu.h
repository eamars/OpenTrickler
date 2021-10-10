#ifndef MAIN_MENU_H_
#define MAIN_MENU_H_


#include "app.h"
#include "freetronicsLCDShield.h"


TricklerState_t main_menu(void);
TricklerState_t main_menu_wait_for_input(freetronicsLCDShield::ButtonType_t *button_press);


#endif  // MAIN_MENU_H_