#include "menu.h"
#include "SH1106.h"


extern SH1106 OLEDScreen;


void render_menu(const MenuItem_t * menu, size_t menu_length, int menu_selection)
{
    OLEDScreen.cls();
    OLEDScreen.locate(0, 0);
    OLEDScreen.set_font((unsigned char *) Terminal6x8);
    // Iterate over all menu items
    for (int idx=0; idx < menu_length; idx++){
        if (idx == menu_selection){
            OLEDScreen.background(White);
            OLEDScreen.foreground(Black);
        }
        else{
            OLEDScreen.background(Black);
            OLEDScreen.foreground(White);
        }
        const char *text = menu[idx].menu_name;
        OLEDScreen.printf("%s", text);

        // Append space
        for (int i = 0; i < 22 - strlen(text); i++){
            OLEDScreen.printf("%c", ' ');
        }
        OLEDScreen.printf("%c", '\n');
    }

    OLEDScreen.copy_to_lcd();
}
