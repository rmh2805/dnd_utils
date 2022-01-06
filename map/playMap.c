#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <ncurses.h>
#include "mapDisp.h"

#include "sprite.h"
#include "tile.h"
#include "map.h"

//==============================<Misc Constants>==============================//

// Argument definitions
#define kUsageArg "-?"
#define kMapArg "-m"

//=============================<Menu Definitions>=============================//
typedef enum mode_e {menu, mapLoad, nav, quit} mode_t;

const char * menuItems[] = {
    "1. Navigate around the map",
    "2. Load a map",
    "3. Quit"
};

mode_t menuModes[] = {
    nav,
    mapLoad,
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

#define streq(a, b) (strcmp(a, b) == 0)

void printHelp(mode_t mode);
void printUsage(const char * firstArg);

FILE* promptFile(bool readable, const char * prompt); 

//================================<Main Code>=================================//
int main(int argc, char** argv) {
    //=======================<Variable Declaration>=======================//
    int status = EXIT_FAILURE;
    int ret, ch;
    char buf[128];

    tileData_t data;
    bool dataLoaded = false;
    bool displayOpen = false;

    map_t map;
    bool mapLoaded = false;

    FILE* fp;

    //==========================<Initialization>==========================//
    // Load in the tile data
    if((ret = loadTileData(&data)) < 0) {
        fprintf(stderr, "*FATAL ERROR* Failed to load the tile data (%d)\n", ret);
        goto main_cleanup;
    }
    dataLoaded = true;

    // Parse all arguments
    for(int i = 1; i < argc; ++i) {
        if(streq(kUsageArg, argv[i])) {         //Usage arg
            printUsage(argv[0]);
            status = EXIT_SUCCESS;
            goto main_cleanup;

        } else if(streq(kMapArg, argv[i])) {    // Map arg
            // Ensure that only one map file is loaded
            if(mapLoaded) {
                fprintf(stderr, "*FATAL ERROR* in argument %d: Can load only one map by argument\n", i);
                printUsage(argv[0]);
                goto main_cleanup;
            }

            // Ensure that a map file was specified
            if(++i >= argc) {
                fprintf(stderr, "*FATAL ERROR* in argument %d: No map file specified\n", i-1);
                printUsage(argv[0]);
                goto main_cleanup;
            }

            // Open the specified map file
            fp = fopen(argv[i], "r");
            if(fp == NULL) {
                fprintf(stderr, "*FATAL ERROR* in argument %d: Unable to open map file \"%s\"", i, argv[i]);
                printUsage(argv[0]);
                goto main_cleanup;
            }

            // Attempt to load the map from file
            if((ret = loadMap(&map, &data.spriteList, fp)) < 0) {
                fclose(fp);
                fprintf(stderr, "*FATAL ERROR* in argument %d: Failed to load map from \"%s\"", i, argv[i]);
                printUsage(argv[0]);
                goto main_cleanup;
            }
            mapLoaded = true;

            // Close the map file
            fclose(fp);
        } else {
            fprintf(stderr, "*FATAL ERROR* in argument %d: Unrecongized argument \"%s\"\n", i, argv[i]);
            printUsage(argv[0]);
            goto main_cleanup;
        }
    }

    // Initialize the display
    if((ret = initDisp(&data.dispData)) < 0) {
        fprintf(stderr, "*FATAL ERROR* Failed to open the display (%d)\n", ret);
        goto main_cleanup;
    }
    displayOpen = true;

    // Set the initial mode value
    int x = 0, y = 0;

    mode_t mode = menu, prevMode = quit;
    //============================<Main Loop>=============================//

    while(mode != quit) {
        if(mode != prevMode) {
            if(mode == menu && !mapLoaded) {
                for(y = 0; menuModes[y] != mapLoad && y < menuSize; ++y);
                y = y % menuSize;
            } else if (mode == nav) {
                x = map.nCols/2;
                y = map.nRows/2;
            } else {
                x = 0;
                y = 0;
            }
        }

        prevMode = mode;

        switch(mode) {
            //======================<Main Menu>=======================//
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

            //======================<Load a Map>======================//
            case mapLoad:
                // Prompt the user for the map file location
                if((fp = promptFile(true, "Enter the map file location")) == NULL) {
                    printError("*ERROR* Failed to open the map file");
                    mode = menu;
                    break;
                }

                // If a map or sprites are loaded, unload them
                if(mapLoaded) {
                    rmMap(map);
                    mapLoaded = false;
                }

                if(data.spriteList != NULL) {
                    rmList(data.spriteList, freeSpriteEntry);
                    data.spriteList = NULL;
                }

                // Perform the map load
                if(loadMap(&map, &data.spriteList, fp) < 0) {
                    printError("*ERROR* Failed to load map from file");
                } else {
                    mapLoaded = true;
                }

                // Close the map file and return to main menu
                fclose(fp);
                mode = menu;
                break;
            //=====================<Navigate Map>=====================//
            case nav:
                // Ensure the map is loaded
                if(!mapLoaded) {
                    mode = menu;
                    break;
                }
                
                // Add the map to the buffer and print
                clearBuffer(&data.dispData);
                addMap(&data, map, x, y);
                printBuffer(data.dispData);
                setCursor(data, map, x, y);
                curs_set(1);

                // Handle inputs
                ch = getch();

                switch(ch) {
                    // Navigation controls
                    case KEY_UP:
                        y = max(y-1, 0);
                        break;
                    case KEY_DOWN:
                        y = min(y+1, map.nRows-1);
                        break;
                    case KEY_LEFT:
                        x = max(x-1, 0);
                        break;
                    case KEY_RIGHT:
                        x = min(x+1, map.nCols-1);
                        break;

                    // Misc controls
                    case '?':
                    case KEY_F(1):
                        printHelp(mode);
                        break;
                    case KEY_HOME:
                    case '`':
                    case '~':
                        mode = menu;
                        break;
                }
                break;

            //=====================<Other States>=====================//
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

//===============================<File Helpers>===============================//
FILE* promptFile(bool openRead, const char * prompt) {
    char buf[128];

    clear();
    printText(kBlackPalette, (prompt == NULL) ? "Enter the file path" : prompt, 0, 0);
    getText(2, 0, buf, 128);

    return fopen(buf, (openRead) ? "r" : "w");
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

        case nav:
            helpPrinter("Use the arrow keys to navigate", 2);
            helpPrinter("Use the '~' key to return to the menu", 3);

            newRow = 5;
            break;

        default:
            break;
    }

    printText(kBlackPalette, "Press enter to continue...", newRow, 0);
    getch();
}

void printUsage(const char * firstArg) {
    printf("Usage: %s [%s <mapFile>]\n", firstArg, kMapArg);
}
