#ifndef CHARGE_MODE_H_
#define CHARGE_MODE_H_


#include "app.h"
#include "freetronicsLCDShield.h"


TricklerState_t charge_mode_select_weight(void);
TricklerState_t charge_mode_select_weight_wait_for_input(freetronicsLCDShield::ButtonType_t *button_press);
TricklerState_t charge_mode_powder_throw(void);
TricklerState_t charge_mode_powder_throw_wait_for_input(freetronicsLCDShield::ButtonType_t *button_press);
TricklerState_t charge_mode_powder_throw_wait_for_complete(void);
TricklerState_t charge_mode_powder_trickle_wait_for_complete(void);
TricklerState_t charge_mode_powder_trickle_wait_for_cup_removal(void);
TricklerState_t charge_mode_powder_trickle_wait_for_cup_returned(void);


#endif  // CHARGE_MODE_H_