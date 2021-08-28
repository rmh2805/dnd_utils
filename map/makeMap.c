#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <ncurses.h>

#include "dispBase.h"
#include "sprite.h"
#include "tile.h"
#include "map.h"

//================================<Misc Data>=================================//
// Data file names
#define kTileFile "walls.out"

// global allocation of large arrays (let the compiler sort this one out)
#define kDefMapRows 6
#define kDefMapCols 6

//===============================<Menu Helpers>===============================//
typedef enum mode_e {
    menu, quit, new, load, save, nav
} mode_t;

const char * menuItems[] = {
    "1. New Map",
    "2. Load Map",
    "3. Save Map",
    "4. Edit Map",
    "5. Quit"
};

mode_t menuModes[] = {
    new,
    load,
    save,
    nav, 
    quit
};

const int menuSize = sizeof(menuItems) / sizeof(menuItems[0]);

void printMenu(int selection) {
    clear();
    printText(kBlackPalette, "Map Editor", 0, 0);

    for(int i = 0; i < menuSize; i++) {
        short palette = (i == selection) ? kWhitePalette : kBlackPalette;
        printText(palette, menuItems[i], i + 2, 0);
    }
}

//===============================<Misc Helpers>===============================//

#define printError(msg) clear();printText(kRedPalette, msg, 0, 0); getch()

#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a > b) ? a : b)


FILE* getMapFile(bool loadFile) {
    char buf[128];

    clear();
    printText(kBlackPalette, "Enter the save path", 0, 0);
    getText(2, 0, buf, 128);

    FILE* fp = fopen(buf, (loadFile) ? "r" : "w");
    return fp;
}

//================================<Main Code>=================================//

int main() {
    tileData_t data;
    int ch;
    int x = 0, y = 0;
    char buf[80];

    bool mapLoaded = false;
    map_t map;

    // Initialize the display
    if(initDisp(&data.dispData) != 0) {
        fprintf(stderr, "*FATAL ERROR* Failed to initialize the display\n");
        return EXIT_FAILURE;
    }

    // Load in the tile file
    FILE* fp = fopen(kTileFile, "r");
    if(fp == NULL) {
        printError("*FATAL ERROR* Failed to load tile data file");
        closeDisp();
        return EXIT_FAILURE;
    }

    if(loadTileData(fp, &data)) {
        printError("*FATAL ERROR* Failed to read tile data from file");
        closeDisp();
        return EXIT_FAILURE;
    }

    // Main Loop
    mode_t mode = menu;
    while(mode != quit) {
        switch(mode) {
            case menu:  // Main menu selection mode
                curs_set(1);
                printMenu(y);

                ch = getch();
                if(ch > '0' && ch <= '0' + menuSize) {
                    y = 0;
                    mode = menuModes[ch - '1'];
                    if(mode == new) {
                        x = kDefMapCols;
                        y = kDefMapRows;
                    }
                    break;
                }

                switch(ch) {
                    case KEY_UP:
                        if(y > 0) --y;
                        break;
                    case KEY_DOWN:
                        if(y + 1 < menuSize) ++y;
                        break;
                    
                    case KEY_ENTER:
                    case '\n':
                        mode = menuModes[y];
                        if(mode == new) {
                            x = kDefMapCols;
                            y = kDefMapRows;
                        } else {
                            x = 0;
                            y = 0;
                        }
                        break;
                }
                break;

            case new:   // Creates a new blank map and transitions to edit screen
                if(mapLoaded) {
                    rmMap(map);
                    mapLoaded = false;
                }

                // Update the display
                clear();
                curs_set(1);
                printText(kBlackPalette, "Use Arrow keys to resize the map and enter to confirm (home or '`' to quit)", 
                            data.dispData.screenRows/3, data.dispData.screenCols/2-34);
                
                sprintf(buf, "rows: %2d     cols: %2d", y, x);
                printText(kBlackPalette, buf, 2 * data.dispData.screenRows / 3, 
                            data.dispData.screenCols / 2 - strlen(buf)/2);

                ch = getch();
                switch(ch) {
                    case KEY_UP:
                        y = (y <= 1) ? 1 : y-1;
                        break;
                    case KEY_DOWN:
                        ++y;
                        break;
                    case KEY_LEFT:
                        x = (x <= 1) ? 1 : x-1;
                        break;
                    case KEY_RIGHT:
                        ++x;
                        break;

                    case KEY_HOME:
                    case '`':
                        x = 0; 
                        y = 0;
                        mode = menu;
                        break;

                    case KEY_ENTER:
                    case '\n':
                        if(mkMap(y, x, &map) < 0) {
                            printError("*ERROR* Unable to allocate new map");
                            mode = menu;
                        } else {
                            mapLoaded = true;
                            mode = nav;
                        }
                        x = 0; 
                        y = 0;
                        break;
                }
                break;
            case load:  // Loads a saved map from file
                if(mapLoaded) {
                    rmMap(map);
                    mapLoaded = false;
                }
                
                fp = getMapFile(true);
                if(loadMap(&map, fp) < 0) {
                    printError("*ERROR* Unable to read map from file");
                } else {
                    mapLoaded = true;
                }
                fclose(fp);

                mode = menu;
                break;

            case save:  // Saves a map to file
                if(!mapLoaded) {
                    mode = menu;
                    break;
                }
                
                fp = getMapFile(false);
                if(writeMap(map, fp) < 0) {
                    printError("*ERROR* Unable to write map to file");
                }
                fclose(fp);

                mode = menu;
                break;

            case nav:   // Navigate around the map
                if(!mapLoaded) {
                    x = 0;
                    y = 0;
                    mode = menu;
                }

                clear();
                drawMap(data, map, x, y);

                ch = getch();
                switch(ch) {
                    // Mode changes
                    case KEY_HOME:
                    case '`':
                        x = 0; 
                        y = 0;
                        mode = menu;
                        break;

                    case KEY_ENTER:
                    case '\n':
                    case 'e':
                        if(map.data[y][x].isEmpty) {
                            map.data[y][x] = mkTile();
                        }
                        break;
                    
                    // Cursor Control
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
                    
                    // Set walls
                    case 'a':
                    case 'A':
                        map.data[y][x].lWall = (map.data[y][x].lWall+1)%3;
                        break;
                    case 'd':
                    case 'D':
                        map.data[y][x].rWall = (map.data[y][x].rWall+1)%3;
                        break;
                    case 'w':
                    case 'W':
                        map.data[y][x].uWall = (map.data[y][x].uWall+1)%3;
                        break;
                    case 's':
                    case 'S':
                        map.data[y][x].dWall = (map.data[y][x].dWall+1)%3;
                        break;
                    
                    // Misc Controls
                    case KEY_DC:    // If delete is pressed...
                    case 27:
                    case 'q':
                        map.data[y][x] = mkEmptyTile();
                        break;
                }
                break;
            default:    // All other modes should simply quit
                mode = quit;
                break;
        }
    }

    // Cleanup and exit successfully
    closeDisp();
    rmTileData(data);
    if(mapLoaded) rmMap(map);

    return EXIT_SUCCESS;
}