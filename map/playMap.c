#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <ncurses.h>
#include "mapDisp.h"

#include "sprite.h"
#include "tile.h"
#include "map.h"

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

//============================<Helper Definitions>============================//
#define printError(msg) clear();printText(kRedPalette, msg, 0, 0); getch()

#ifndef min
#define min(a, b) ((a < b) ? a : b)
#endif

#ifndef max
#define max(a, b) ((a > b) ? a : b)
#endif

void printHelp(mode_t mode);

//================================<Main Code>=================================//
int main() {
    //=======================<Variable Declaration>=======================//
    int status = EXIT_FAILURE;
    int ret, ch;
    char buf[128];

    tileData_t data;
    bool dataLoaded = false;
    bool displayOpen = false;

    map_t map;
    bool mapLoaded = false;

    //==========================<Initialization>==========================//
    // Load in the tile data
    if((ret = loadTileData(&data)) < 0) {
        fprintf(stderr, "*FATAL ERROR* Failed to load the tile data (%d)\n", ret);
        goto main_cleanup;
    }
    dataLoaded = true;

    // Initialize the display
    if((ret = initDisp(&data.dispData)) < 0) {
        fprintf(stderr, "*FATAL ERROR* Failed to open the display (%d)\n", ret);
        goto main_cleanup;
    }
    displayOpen = true;

    // Set the initial mode value
    int x = 0, y = 0;

    mode_t mode = menu, prevMode = mode;
    //============================<Main Loop>=============================//

    while(mode != quit) {
        if(mode != prevMode) {
            x = 0;
            y = 0;
        }

        prevMode = mode;

        switch(mode) {
            case menu:
                // Update the display
                clearBuffer(&data.dispData);
                addMenu(&data.dispData, "Play Map", menuItems, menuSize, y);

                sprintf(buf, "%sMap Loaded", (mapLoaded) ? "" : "No ");
                addText(&data.dispData, kBlackPalette, buf, menuSize+3, 0);
                
                printBuffer(data.dispData);

                // Get and input
                ch = getch();

                // If a menu item's number was entered, select it and queue enter
                if(ch >= '1' && ch <= '0' + min(menuSize, 9)) {
                    y = ch - '0';
                    ch = '\n';
                }

                // Act on input
                switch(ch) {
                    // Navigation
                    case KEY_UP:
                        y = min(y+1, 0);
                        break;
                    case KEY_DOWN:
                        y = max(y-1, 0);
                        break;

                    // Selection
                    case '\n':
                    case KEY_ENTER:
                        mode = menuModes[y];
                        break;

                    // Misc inputs
                    case KEY_F(1):
                    case '?':
                        printHelp(mode);
                        break;

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
    if(displayOpen) {
        closeDisp(data.dispData);
    }
    if(dataLoaded) {
        rmTileData(data);
    }
    if(mapLoaded) {
        rmMap(map);
    }
    return status;
}

//===============================<Misc Helpers>===============================//
#define helpPrinter(msg, row) printText(kBlackPalette, msg, row, 0)

void printHelp(mode_t mode) {
    clear();
    printText(kBlackPalette, "Help Prompt", 0, 0);
    int newRow = 2;

    switch(mode) {
        case menu:
            helpPrinter("Use the arrow keys and enter to select an option", 2);
            helpPrinter("Alternatively, you can select the numbered options with their keys", 3);

            newRow = 5;
            break;

        default:
            break;
    }

    printText(kBlackPalette, "Press enter to continue...", newRow, 0);
    getch();
}