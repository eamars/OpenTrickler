#ifndef CHARGE_MODE_H_
#define CHARGE_MODE_H_


#include "app.h"


TricklerState_t charge_mode_select_weight(void);
TricklerState_t charge_mode_powder_throw(void);
TricklerState_t charge_mode_powder_throw_wait_for_complete(void);

TricklerState_t charge_mode_coarse_trickle(void);
TricklerState_t charge_mode_coarse_trickle_wait_for_complete(void);
TricklerState_t charge_mode_powder_trickle_wait_for_complete(void);
TricklerState_t charge_mode_powder_trickle_wait_for_cup_removal(void);
TricklerState_t charge_mode_powder_trickle_wait_for_cup_returned(void);


#endif  // CHARGE_MODE_H_