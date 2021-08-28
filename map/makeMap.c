#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <ncurses.h>

#include "dispBase.h"
#include "sprite.h"
#include "tile.h"

//================================<Misc Data>=================================//
// Data file names
#define kTileFile "walls.out"

// global allocation of large arrays (let the compiler sort this one out)
#define kMapRows 30
#define kMapCols 30
tile_t mapBuf[kMapRows][kMapCols];

//===============================<Menu Helpers>===============================//
typedef enum mode_e {
    menu, quit, new, load, save, nav, edit
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

//================================<Main Code>=================================//

int main() {
    tileData_t data;
    int ch;
    int y = 0;

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
                    mode = menuModes[ch - '1'];
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
                        break;
                }
                break;

            case new:
            case load:
            case save:
            case nav:
            case edit:
            default:
                mode = quit;
                break;
        }
    }

    // Cleanup and exit successfully
    closeDisp();
    rmTileData(data);

    return EXIT_SUCCESS;
}