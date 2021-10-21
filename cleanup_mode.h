#ifndef CLEANUP_MODE_H_
#define CLEANUP_MODE_H_

#include "app.h"


OpenTricklerStateFlag_e cleanup_trickler_menu(void);
OpenTricklerStateFlag_e cleanup_trickler_wait_for_complete(void);

OpenTricklerStateFlag_e cleanup_thrower_menu(void);
OpenTricklerStateFlag_e cleanup_thrower_wait_for_complete(void);


#endif  // CLEANUP_MODE_H_