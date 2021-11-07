#ifndef MENU_H_
#define MENU_H_

#include "app.h"
#include "mbed.h"


typedef struct {
    char menu_name[22];  // includes \0
    TricklerState_t state;
} MenuItem_t;


void render_menu(const MenuItem_t * menu, size_t menu_length, int menu_selection);


#endif  // MENU_H_