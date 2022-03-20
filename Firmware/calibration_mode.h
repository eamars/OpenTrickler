#ifndef CALIBRATION_MODE_H_
#define CALIBRATION_MODE_H_

#include "app.h"

TricklerState_t calibration_mode_menu(void);
TricklerState_t calibration_mode_tune_fine_trickler(void);
TricklerState_t calibration_mode_tune_coarse_trickler(void);

#endif  // CALIBRATION_MODE_H_
