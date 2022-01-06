#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <ncurses.h>
#include "mapDisp.h"

#include "actor.h"

//==============================<Misc Constants>==============================//

//=============================<Menu Definitions>=============================//
typedef enum mode_e {menu, quit} mode_t;

const char * menuItems[] = {
    "1. Quit"
};

mode_t menuModes[] = {
    quit
};

const int menuSize = sizeof(menuItems) / sizeof(menuItems[0]);

//===========================<Helper Declarations>============================//
#ifndef min
#define min(a, b) ((a < b) ? a : b)
#endif

#ifndef max
#define max(a, b) ((a > b) ? a : b)
#endif

//================================<Main Code>=================================//
int main() {
    //=======================<Variable Definitions>=======================//
    int status = EXIT_FAILURE;
    int ch, ret;

    dispData_t data;
    bool dispOpen = false;

    actorData_t actors;
    bool actorsLoaded = false;

    //==============================<Setup>===============================//

    // Initialize the display
    if((ret = initDisp(&data)) < 0) {
        fprintf(stderr, "*FATAL ERROR* Unable to initialize display (%d)\n", ret);
        goto main_cleanup;
    }

    //Set the current mode
    mode_t mode = menu, prevMode = quit;
    int y;
    //============================<Main Loop>=============================//
    while(mode != quit) {
        // Check for mode change updates
        if(mode != prevMode) {
            y = 0;
        }
        prevMode = mode;

        // Switch out for modes
        switch(mode) {
            case menu:
                // Update the display
                clearBuffer(&data);
                addMenu(&data, "Make Cast", menuItems, menuSize, y);

                printBuffer(data);

                // Get an input
                ch = getch();// If a menu item's number was entered, select it and queue enter
                if(ch >= '1' && ch <= '0' + min(menuSize, 9)) {
                    y = ch - '1';
                    ch = '\n';
                }

                // Act on input
                switch(ch) {
                    // Navigation
                    case KEY_UP:
                        y = max(y-1, 0);
                        break;
                    case KEY_DOWN:
                        y = min(y+1, menuSize-1);
                        break;

                    // Selection
                    case '\n':
                    case KEY_ENTER:
                        mode = menuModes[y];
                        break;

                    // Misc inputs
                    case KEY_HOME:
                    case '`':
                    case '~':
                        mode = quit;
                        break;
                }
                break;

            default:
                mode = quit;
                break;
        }
    }

    //=============================<Cleanup>==============================//
    status = EXIT_SUCCESS;
main_cleanup:
    if(dispOpen) {
        closeDisp(data);
    }
    if(actorsLoaded) {
        rmActorData(actors);
    }
    return status;
}

//============================<Helper Definitions>============================//