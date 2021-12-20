#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <ncurses.h>
#include "mapDisp.h"

#include "sprite.h"
#include "tile.h"
#include "map.h"

//================================<Misc Data>=================================//
//todo Replace this, so walls can be linked in and exe can move
// Data file names
#define kTileFile "walls.spt"

// Define default values
#define kDefMapRows 32
#define kDefMapCols 32

// Define argument values
#define kMapFileFlag "-m"
#define kUsageFlag "-?"

//===============================<Menu Helpers>===============================//
typedef enum mode_e {
    menu, quit, new, load, save, nav, file
} mode_t;

const char * menuItems[] = {
    "1. New Map",
    "2. Load Map",
    "3. Save Map",
    "4. Edit Map",
    "5. Make Printable",
    "6. Quit"
};

mode_t menuModes[] = {
    new,
    load,
    save,
    nav,
    file,
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

void floodRoom(map_t * map, int x, int y);

void printHelp(mode_t mode);

FILE* promptFile(bool openRead);
int doLoad(map_t * map, bool * isLoaded);

//================================<Main Code>=================================//
int main(int argc, char** argv) {
    int status = EXIT_FAILURE;

    int ret, ch;
    int x = 0, y = 0;
    char buf[80];

    FILE * fp;

    bool mapLoaded = false;
    bool dispOpen = false;
    map_t map;

    bool tilesLoaded = false;
    tileData_t data;

    //=========================<Argument Parsing>=========================//
    for(int i = 1; i < argc; i++) {
        if(strcmp(kUsageFlag, argv[i]) == 0) {  // Argument to print usage msg
            printf("Usage: %s [%s <Map File>]\n", argv[0], kMapFileFlag);
            status = EXIT_SUCCESS;
            goto main_cleanup;
        } else if (strcmp(kMapFileFlag, argv[i]) == 0) { // Argument to pre-load map
            if(mapLoaded) {
                fprintf(stderr, "*FATAL ERROR* Attempted to load 2 maps by arg\n");
                goto main_cleanup;
            }
            if(++i >= argc) {
                fprintf(stderr, "*FATAL ERROR* No map file specified\n");
                goto main_cleanup;
            }

            fp = fopen(argv[i], "r");
            if(fp == NULL) {
                fprintf(stderr, "*FATAL ERROR* Unable to open map file \"%s\"\n", argv[i]);
                goto main_cleanup;
            }

            if(loadMap(&map, fp) < 0) {
                fclose(fp);
                fprintf(stderr, "*FATAL ERROR* Unable to read map file \"%s\"\n", argv[i]);
                goto main_cleanup;
            }

            // Mark the map as loaded
            mapLoaded = true;
        } else {
            fprintf(stderr, "*FATAL ERROR* Unkown argument \"%s\"\n", argv[i]);
            goto main_cleanup;
        }

    }

    //==========================<Initialization>==========================//
    // Initialize the display
    if(initDisp(&data.dispData) != 0) {
        fprintf(stderr, "*FATAL ERROR* Failed to initialize the display\n");
        goto main_cleanup;
    }
    dispOpen = true;

    // Load in the tile file
    fp = fopen(kTileFile, "r");
    if(fp == NULL) {
        printError("*FATAL ERROR* Failed to load tile data file");
        goto main_cleanup;
    }

    if(loadTileData(fp, &data)) {
        printError("*FATAL ERROR* Failed to read tile data from file");
        goto main_cleanup;
    }
    fclose(fp);

    tilesLoaded = true;

    // Set the initial mode and navigation
    mode_t mode = menu;

    if(mapLoaded) { // if map loaded, go to nav from center of map
        mode = nav;
        x = map.nCols/2;
        y = map.nRows/2;
    }

    mode_t prevMode = mode;
    //============================<Main Loop>=============================//
    while(mode != quit) {
        // If the mode has changed, update the cursor coords to their default values
        if(prevMode != mode) {
            switch(mode) {
                case new:
                    x = kDefMapCols;
                    y = kDefMapRows;
                    break;
                case nav:
                    x = mapLoaded ? map.nCols/2 : 0;
                    y = mapLoaded ? map.nRows/2 : 0;
                    break;
                default:
                    x = 0;
                    y = 0;
            }
        }

        // Update the previous mode tracker
        prevMode = mode;

        // Break out functions by mode
        switch(mode) {
            //======================<Main Menu>=======================//
            case menu:
                // Print the menu text
                addMenu(&data.dispData, "Make Map", menuItems, menuSize, y);
                addText(&data.dispData, kBlackPalette, (mapLoaded) ? "A map is loaded" : "No map loaded", menuSize+3, 0);
                printBuffer(data.dispData);

                // Get the next input
                ch = getch();

                // If the input is a valid menu option, take it
                if(ch > '0' && ch <= '0' + menuSize) {
                    y = 0;
                    mode = menuModes[ch - '1'];
                    break;
                }

                // Otherwise, break out for handling of individual keys
                switch(ch) {
                    // Print help splash with '?' or F1
                    case '?':
                    case KEY_F(1):
                        printHelp(menu);
                        break;

                    // Change selected item with up and down arrow keys
                    case KEY_UP:
                        if(y > 0) --y;
                        break;
                    case KEY_DOWN:
                        if(y + 1 < menuSize) ++y;
                        break;

                    // Make a selection with enter
                    case KEY_ENTER:
                    case '\n':
                        mode = menuModes[y];
                        break;
                    
                    // Quit with '`' or '~'
                    case '`':
                    case '~':
                        mode = quit;
                }
                break;

            //===================<New Map Creation>===================//
            case new:
                // Update the display
                clear();
                curs_set(1);
                printText(kBlackPalette, "Use Arrow keys to resize the map and enter to confirm (home or '`' to quit)", 
                            data.dispData.screenRows/3, data.dispData.screenCols/2-34);
                
                sprintf(buf, "rows: %2d     cols: %2d", y, x);
                printText(kBlackPalette, buf, 2 * data.dispData.screenRows / 3, 
                            data.dispData.screenCols / 2 - strlen(buf)/2);

                // Get and act on input
                ch = getch();
                switch(ch) {
                    // Arrow keys modify size
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

                    // This case returns to the main menu
                    case KEY_HOME:
                    case '`':
                    case '~':
                        mode = menu;
                        break;

                    // This case handles when dimensions are confirmed
                    case KEY_ENTER:
                    case '\n':
                        if(mapLoaded) {
                            rmMap(map);
                            mapLoaded = false;
                        }

                        if(mkMap(y, x, &map) < 0) {
                            printError("*ERROR* Unable to allocate new map");
                            mode = menu;
                        } else {
                            mapLoaded = true;
                            mode = nav;
                        }
                        break;
                }
                break;

            //========================<Load Map>=========================//
            case load:
                doLoad(&map, &mapLoaded);
                y = 0;
                mode = menu;
                break;

            //========================<Save Map>=========================//
            case save:
                // Ensure that a map is loaded
                if(!mapLoaded) {
                    mode = menu;
                    break;
                }

                // Prompt the user for a (writable) file
                fp = promptFile(false);
                if(fp == NULL) {
                    printError("*ERROR* Unable to open map file");
                    mode = menu;
                    break;
                }

                // Attempt to writhe the map to file and close it
                if(writeMap(map, fp) < 0) {
                    printError("*ERROR* Unable to write map to file");
                }
                fclose(fp);

                mode = menu;
                break;

            //======================<Navigate Map>=======================//
            case nav:
                // Ensure that a map is loaded
                if(!mapLoaded) {
                    mode = menu;
                    break;
                }

                // Add the map to the buffer and print
                clearBuffer(&data.dispData);
                addMap(&data, map, x, y);
                clear();
                printBuffer(data.dispData);
                setCursor(data, map, x, y);

                // Get and act on input
                ch = getch();
                switch(ch) {
                    // Change modes
                    case KEY_HOME:
                    case '`':
                    case '~':
                        mode = menu;
                        break;

                    // Tile set/unset
                    case KEY_ENTER:
                    case '\n':
                    case 'e':
                        map.data[y][x].isEmpty = false;
                        break;
                    case KEY_DC:    // If delete is pressed empty cell
                    case 27:
                    case 'q':
                    case 'Q':
                        map.data[y][x].isEmpty = true;
                        break;
                    
                    // Cursor Control with arrows
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
                    
                    // Set walls on current cell with WASD
                    case 'w':
                    case 'W':
                        map.data[y][x].uWall = (map.data[y][x].uWall+1)%3;
                        break;
                    case 'a':
                    case 'A':
                        map.data[y][x].lWall = (map.data[y][x].lWall+1)%3;
                        break;
                    case 's':   // Try to set top wall on cell below first
                    case 'S':
                        if(y == map.nRows - 1) {
                            map.data[y][x].dWall=(map.data[y][x].dWall+1)%3;
                        } else {
                            map.data[y+1][x].uWall=(map.data[y+1][x].uWall+1)%3;
                        }
                        break;
                    case 'd':   // Try to set left wall on cell to right first
                    case 'D':
                        if(x == map.nCols - 1) {
                            map.data[y][x].rWall=(map.data[y][x].rWall+1)%3;
                        } else {
                            map.data[y][x+1].lWall=(map.data[y][x+1].lWall+1)%3;
                        }
                        break;
                    
                    // Palette Setting
                    case 'c':   // Cycle tile palettes
                    case 'C':
                        if(map.data[y][x].bgPalette == 0) {
                            map.data[y][x].bgPalette = kDefPalette;
                        }

                        map.data[y][x].bgPalette += 1;
                        if(map.data[y][x].bgPalette >= kMaxPalette) {
                            map.data[y][x].bgPalette = kMinPalette;
                        }
                        break;
                    
                    case 'v':   // Cycle sprite palettes
                    case 'V':
                        if(map.data[y][x].spritePalette == 0) {
                            map.data[y][x].spritePalette = kDefPalette;
                        }

                        map.data[y][x].spritePalette += 1;
                        if(map.data[y][x].spritePalette >= kMaxPalette) {
                            map.data[y][x].spritePalette = kMinPalette;
                        }
                        break;
                    
                    case 'p':   // Fill the current room w/ the current palette
                    case 'P':
                        floodRoom(&map, x, y);
                        break;

                    // Sprite setting
                    case 'z':   // Remove sprites
                    case 'Z':
                        clearTileSprite(&map.data[y][x]);
                        break;
                    
                    case 'r':   // Next loaded sprite
                    case 'R':
                        break;

                    case 'g':   // Character sprite
                    case 'G':
                        setCharSprite(&map.data[y][x], getch(), kDefPalette);
                        break;

                    // Misc Controls
                    case '?':       // Display the help text
                    case KEY_F(1):
                        printHelp(mode);
                        break;
                }
                break;

            //================<Output to printable files>================//
            case file:
                // Ensure that a map is loaded
                if(!mapLoaded) {
                    mode = menu;
                    break;
                }

                // Prompt the user for the inclusion of sprites
                clear();
                printText(kBlackPalette, "Include sprites [Y/n]? ", 0, 0);
                ch = getch();

                // Prompt the user for a (writeable) filename
                fp = promptFile(false);
                if(fp == NULL) {
                    printError("*ERROR* Unable to open output file");
                    mode = menu;
                    break;
                }

                // Attempt to output the file in printable sections
                ret = mapToSections(data, map, fp, 80, 64, !(ch == 'N' || ch == 'n'));
                if(ret < 0) {
                    sprintf(buf, "*ERROR* Failed to write out to file (%d)", ret);
                    printError(buf);
                }

                // Close the file and return to the menu
                fclose(fp);
                mode = menu;
                break;
            
            //=========================<Default>=========================//
            default:    // All other modes should simply quit
                mode = quit;
                break;
        }
    }
    
    status = EXIT_SUCCESS;
main_cleanup:
    // Cleanup and exit successfully
    if(dispOpen) closeDisp(data.dispData);
    if(tilesLoaded) rmTileData(data);
    if(mapLoaded) rmMap(map);

    return status;
}

//===========================<Color Flood Helpers>============================//
void floodRoomRec(map_t * map, bool** visited, short palette, int x, int y) {
    // Caller checks position for wall detection
    if(visited[y][x] || map->data[y][x].isEmpty) { 
        return;
    }

    // Mark this cell as visited and update its palette
    visited[y][x] = true;
    map->data[y][x].bgPalette = palette;

    // Call recursively to all adjacent cells not blocked by walls
    // Cell above
    if(y > 0 && !(map->data[y][x].uWall || map->data[y-1][x].dWall)) {
        floodRoomRec(map, visited, palette, x, y-1);
    }
    // Cell below
    if(y+1 < map->nRows && !(map->data[y][x].dWall || map->data[y+1][x].uWall)) {
        floodRoomRec(map, visited, palette, x, y+1);
    }
    // Cell left
    if(x > 0 && !(map->data[y][x].lWall || map->data[y][x-1].rWall)) {
        floodRoomRec(map, visited, palette, x-1, y);
    }
    // Cell right
    if(x+1 < map->nCols && !(map->data[y][x].rWall || map->data[y][x+1].lWall)) {
        floodRoomRec(map, visited, palette, x+1, y);
    }


}

void floodRoom(map_t * map, int x, int y) {
    // First of all, ensure that the map exists and that the first square is enabled
    if(map == NULL || y < 0 || y >= map->nRows || x < 0 || x >= map->nCols || 
            map->data[y][x].isEmpty) {
        return;
    }

    // First allocated a visited array
    bool ** visited = calloc(map->nRows, sizeof(bool *));
    if(visited == NULL) {
        return;
    }

    for(int i = 0; i < map->nRows; i++) {
        visited[i] = calloc(map->nCols, sizeof(bool));
        if(visited[i] == NULL) goto floodRoomCleanup;
    }

    // Next grab the palette and begin the flood process
    short palette = map->data[y][x].bgPalette;
    floodRoomRec(map, visited, palette, x, y);


floodRoomCleanup:
    if(visited != NULL) {
        for(int i = 0; i < map->nRows; i++) {
            if(visited[i] != NULL) free(visited[i]);
        }
        free(visited);
    }
}

//===============================<File Helper>================================//
FILE* promptFile(bool openRead) {
    char buf[128];

    clear();
    printText(kBlackPalette, "Enter the file path", 0, 0);
    getText(2, 0, buf, 128);

    FILE* fp = fopen(buf, (openRead) ? "r" : "w");
    return fp;
}

int doLoad(map_t * map, bool * isLoaded) {
    FILE * fp = promptFile(true);
    if(fp == NULL) {
        printError("*ERROR* Unable to open map file");
        return -1;
    }

    if(*isLoaded) {
        rmMap(*map);
        *isLoaded = false;
    }

    if(loadMap(map, fp) < 0) {
        printError("*ERROR* Unable to read map from file");
    } else {
        *isLoaded = true;
    }
    fclose(fp);

    return (*isLoaded) ? -1 : 0;
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

            helpPrinter("New Map will prompt you to select dimensions for a new map", 5);
            helpPrinter("Load Map will prompt you for a file to load a map from", 6);
            helpPrinter("Save Map will prmopt you for a filename to save out the current map", 7);
            helpPrinter("Edit map will allow you to edit the loaded map", 8);
            helpPrinter("Make Printable will generate a map to be printed as plaintext", 9);
            helpPrinter("Quit... quits (real original, I know)", 10);

            helpPrinter("Worth noting, attempting to backspace in text entry currently results in", 12);
            helpPrinter("corruption. Instead, the delete key currently fills its role", 13);
            newRow = 15;
            break;
        case nav:
            helpPrinter("Use the Arrow keys to select a tile to modify", 2);
            helpPrinter("Enter or 'e' will mark the tile as habitable", 3);
            helpPrinter("Delete or 'q' will mark the tile as uninhabitable", 4);
            helpPrinter("WASD cycles the walls on each side of the cell b/n empty, solid, and door", 5);
            helpPrinter("Home or '`' will return you to the main menu", 6);
            helpPrinter("'c' cycles the color palette of the tile ", 7);
            helpPrinter("'v' cycles the color palette of the tile's sprite", 8);
            helpPrinter("'r' cycles to the next loaded sprite (not currently implemented)", 9);
            helpPrinter("'g' places a char sprite of your chosing", 10);
            helpPrinter("'z' removes any sprite from the selected cell", 11);
            helpPrinter("'p' fills the selected room with the current color", 13);
            newRow = 15;
            break;
        default:
            newRow = 2;
    }
    printText(kBlackPalette, "Press enter to continue...", newRow, 0);
    getch();

}