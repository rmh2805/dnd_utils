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
// Data file names
#define kTileFile "walls.out"

// global allocation of large arrays (let the compiler sort this one out)
#define kDefMapRows 32
#define kDefMapCols 32

#define kMapFileFlag "-m"
#define kSpriteFileFlag "-s"
#define kUsageFlag "-?"

//===============================<Menu Helpers>===============================//
typedef enum mode_e {
    menu, quit, new, load, save, nav, file, getClip
} mode_t;

const char * menuItems[] = {
    "1. New Map",
    "2. Load Map",
    "3. Save Map",
    "4. Edit Map",
    "5. Make Printable",
    "6. Load a Clip",
    "7. Quit"
};

mode_t menuModes[] = {
    new,
    load,
    save,
    nav,
    file,
    getClip,
    quit
};

const int menuSize = sizeof(menuItems) / sizeof(menuItems[0]);

//===============================<Misc Helpers>===============================//

#define printError(msg) clear();printText(kRedPalette, msg, 0, 0); getch()

#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a > b) ? a : b)


FILE* promptFile(bool openRead) {
    char buf[128];

    clear();
    printText(kBlackPalette, "Enter the file path", 0, 0);
    getText(2, 0, buf, 128);

    FILE* fp = fopen(buf, (openRead) ? "r" : "w");
    return fp;
}

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

//================================<Main Code>=================================//

int main(int argc, char** argv) {
    tileData_t data;
    int ret, ch;
    int x = 0, y = 0;
    char buf[80];

    FILE * fp;

    bool mapLoaded = false;
    map_t map;
    bool clipLoaded = false;
    map_t clip;


    //=========================<Argument Parsing>=========================//
    for(int i = 1; i < argc; i++) {
        if(strcmp(kUsageFlag, argv[i]) == 0) {
            if(mapLoaded) rmMap(map);
            printf("Usage: %s [%s <Map File>] [%s <Sprite File>]\n", argv[0], kMapFileFlag, kSpriteFileFlag);
            return EXIT_SUCCESS;
        } else if (strcmp(kMapFileFlag, argv[i]) == 0) {
            if(mapLoaded) {
                rmMap(map);
                fprintf(stderr, "*FATAL ERROR* Attempted to load 2 maps by arg\n");
                return EXIT_FAILURE;
            }
            if(++i >= argc) {
                fprintf(stderr, "*FATAL ERROR* No map file specified\n");
                return EXIT_FAILURE;
            }

            fp = fopen(argv[i], "r");
            if(fp == NULL) {
                fprintf(stderr, "*FATAL ERROR* Unable to open map file \"%s\"\n", argv[i]);
                return EXIT_FAILURE;
            }

            if(loadMap(&map, fp) < 0) {
                fclose(fp);
                fprintf(stderr, "*FATAL ERROR* Unable to read map file \"%s\"\n", argv[i]);
                return EXIT_FAILURE;
            }
            mapLoaded = true;

        } else if(strcmp(kSpriteFileFlag, argv[i]) == 0) {
            if(++i > argc) {
                if(mapLoaded) rmMap(map);
                fprintf(stderr, "*FATAL ERROR* No sprite sheet specified\n");
                return EXIT_FAILURE;
            }
            continue;
        } else {
            fprintf(stderr, "*FATAL ERROR* Unkown argument \"%s\"\n", argv[i]);
            if(mapLoaded) rmMap(map);
            return EXIT_FAILURE;
        }

    }

    //==========================<Initialization>==========================//
    // Initialize the display
    if(initDisp(&data.dispData) != 0) {
        fprintf(stderr, "*FATAL ERROR* Failed to initialize the display\n");
        return EXIT_FAILURE;
    }

    // Load in the tile file
    fp = fopen(kTileFile, "r");
    if(fp == NULL) {
        printError("*FATAL ERROR* Failed to load tile data file");
        closeDisp(data.dispData);
        return EXIT_FAILURE;
    }

    if(loadTileData(fp, &data)) {
        printError("*FATAL ERROR* Failed to read tile data from file");
        closeDisp(data.dispData);
        return EXIT_FAILURE;
    }
    fclose(fp);

    //============================<Main Loop>=============================//
    mode_t mode = (mapLoaded) ? nav : menu;
    while(mode != quit) {
        switch(mode) {
            case menu:  // Main menu selection mode
                curs_set(1);
                addMenu(&data.dispData, "Make Map", menuItems, menuSize, y);
                addText(&data.dispData, kBlackPalette, (mapLoaded) ? "A map is loaded" : "No map loaded", menuSize+3, 0);
                printBuffer(data.dispData);

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
                    case '?':
                        printHelp(menu);
                        break;
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
                        } else if (mode == nav && mapLoaded) {
                            x = map.nCols/2;
                            y = map.nRows/2;
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
                
                fp = promptFile(true);
                if(fp == NULL) {
                    printError("*ERROR* Unable to open map file");
                    y = 0;
                    mode = menu;
                    break;
                }
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
                
                fp = promptFile(false);
                if(fp == NULL) {
                    printError("*ERROR* Unable to open map file");
                    y = 0;
                    mode = menu;
                    break;
                }
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
                    break;
                }

                clear();
                clearBuffer(&data.dispData);
                addMap(&data, map, x, y);
                printBuffer(data.dispData);
                setCursor(data, map, x, y);

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
                        map.data[y][x].isEmpty = false;
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
                        if(x == map.nCols - 1) {
                            map.data[y][x].rWall=(map.data[y][x].rWall+1)%3;
                        } else {
                            map.data[y][x+1].lWall=(map.data[y][x+1].lWall+1)%3;
                        }
                        break;
                    case 'w':
                    case 'W':
                        map.data[y][x].uWall = (map.data[y][x].uWall+1)%3;
                        break;
                    case 's':
                    case 'S':
                        if(y == map.nRows - 1) {
                            map.data[y][x].dWall=(map.data[y][x].dWall+1)%3;
                        } else {
                            map.data[y+1][x].uWall=(map.data[y+1][x].uWall+1)%3;
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
                    case KEY_DC:    // If delete is pressed empty cell
                    case 27:
                    case 'q':
                    case 'Q':
                        map.data[y][x].isEmpty = true;
                        break;
                    case '?':       // Display the help text
                        printHelp(mode);
                        break;
                }
                break;
            
            case file:
                if(!mapLoaded) {
                    x = 0;
                    y = 0;
                    mode = menu;
                    break;
                }

                clear();
                printText(kBlackPalette, "Include sprites [Y/n]? ", 0, 0);
                ch = getch();

                fp = promptFile(false);
                if(fp == NULL) {
                    printError("*ERROR* Unable to open output file");
                    mode = menu;
                    y = 0;
                    break;
                }
                ret = mapToSections(data, map, fp, 80, 64, !(ch == 'N' || ch == 'n'));
                if(ret < 0) {
                    sprintf(buf, "*ERROR* Failed to write out to file (%d)", ret);
                    printError(buf);
                }
                fclose(fp);
                mode = menu;
                break;
            case getClip:
                mode = menu;
                
                break;
            default:    // All other modes should simply quit
                mode = quit;
                break;
        }
    }

    // Cleanup and exit successfully
    closeDisp(data.dispData);
    rmTileData(data);
    if(mapLoaded) rmMap(map);
    if(clipLoaded) rmMap(clip);

    return EXIT_SUCCESS;
}