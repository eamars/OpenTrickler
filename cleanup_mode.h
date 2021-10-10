#ifndef CLEANUP_MODE_H_
#define CLEANUP_MODE_H_

#include "app.h"
#include "freetronicsLCDShield.h"


TricklerState_t cleanup_mode_menu(void);
TricklerState_t cleanup_mode_wait_for_input(freetronicsLCDShield::ButtonType_t *button_press);
TricklerState_t cleanup_mode_wait_for_complete(void);

#endif  // CLEANUP_MODE_H_