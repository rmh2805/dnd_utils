#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <curses.h>

#include "sprite.h"
#include "dispBase.h"

typedef enum mode_e {menu, tile, sprite, load, save, quit} mode_t;

const char * menuItems[] = {
    "1. Edit Tile",
    "2. Edit Sprite",
    "3. Save Sprite Sheet",
    "4. Load Sprite Sheet",
    "5. Exit Program"
};

const mode_t menuModes[] = {
    tile,
    sprite,
    save,
    load,
    quit
};

const unsigned int menuSize = sizeof(menuItems) / sizeof(menuItems[0]);

void dispMenu(unsigned int selected) {
    clear();

    printText(kBlackPalette, "Sprite Editor", 0, 0);

    for(unsigned int i = 0; i < menuSize; i++) {
        printText((i == selected) ? kBlackPalette : kWhitePalette, menuItems[i], 2 + i, 0);
    }
}

int main() {
    int ret;

    // Initialize the display
    dispData_t data;
    ret = initDisp(&data);
    if(ret != 0) {
        fprintf(stderr, "*ERROR* in main: Failed to initialized display\n");
        return EXIT_FAILURE;
    }

    // Main loop
    unsigned int selX = 0, selY = 0;
    mode_t mode = menu;
    while(mode != quit) {
        switch(mode) {
            case menu:
                dispMenu(selY);

                int ch = getch();
                if(ch > '0' && ch <= '0' + menuSize) {
                    mode = menuModes[ch - '1'];
                    break;
                }

                switch(ch) {
                    case KEY_ENTER:
                    case '\n':
                    case ' ':
                        mode = menuModes[selY];
                        selX = 0;
                        selY = 0;
                        break;
                    case KEY_UP:
                        selY = (selY == 0) ? selY : selY - 1;
                        break;
                    case KEY_DOWN:
                        selY = (selY + 1 == menuSize) ? selY : selY + 1;
                        break;
                }
                break;
            
            default:
                mode = quit;

        }
    }

    // Cleanup and exit
    closeDisp();

    return EXIT_SUCCESS;
}