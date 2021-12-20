#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <curses.h>
#include "mapDisp.h"

#include "sprite.h"

#include "../common/list.h"
#include "../common/dispBase.h"

// todo This whole program needs an overhaul. It builds, but it's old

//==============================<Menu Handling>===============================//
typedef enum mode_e {menu, sel, new, quit} mode_t;

const char * menuItems[] = {
    "1. Select a sprite to edit",
    "2. Create a new sprite",
    "3. Exit Program"
};

const mode_t menuModes[] = {
    sel,
    new,
    quit
};

const unsigned char menuSize = sizeof(menuItems) / sizeof(menuItems[0]);

//===========================<Helper Declarations>============================//

#ifndef min
#define min(a, b) ((a < b) ? a : b)
#endif

#ifndef max
#define max(a, b) ((a > b) ? a : b)
#endif

FILE* promptFile(bool openRead, const char * prompt);

#define printError(msg) clear();printText(kRedPalette, msg, 0, 0); getch()
void printHelp(mode_t mode);

//==============================<Main Execution>==============================//
int main(int argc, char** argv) {
    int status = EXIT_FAILURE;  // The exit status to return

    int ch, ret;    // Ints to hold input data and return values respectively
    char buf[80];   // A character buffer

    dispData_t dispData;
    bool dispOpen = false;

    list_t spriteList = NULL;
    bool listLoaded = false;

    //=========================<Argument Parsing>=========================//
    // Todo load each file provided as an arg in order

    //===========================<Initialization>=========================//
    // Initialize the display
    if((ret = initDisp(&dispData)) < 0) {
        fprintf(stderr, "*Fatal Error: Failed to initialize the display (%d)\n", ret);
        goto main_cleanup;
    }
    dispOpen = true;

    // Initialize the mode and cursor storage
    mode_t mode = menu, prevMode = menu;
    int x = 0, y = 0;

    //============================<Main Loop>=============================//
    while(mode != quit) {
        // Reset navigation on change
        if(prevMode != mode) {
            x = 0;
            y = 0;
        }
        prevMode = mode;

        switch(mode) {
            //=======================<Main Menu>======================//
            case menu:
                // Display the menu
                clearBuffer(&dispData);
                addMenu(&dispData, "MakeSprite", menuItems, menuSize, y);
                printBuffer(dispData);

                // Get the next input
                ch = getch();

                // If the input was a number on the menu, select it and treat input as enter
                if(ch >= '1' && ch < min('9', '0' + menuSize)) {
                    y = ch - '1';
                    ch = '\n';
                }

                // Handle inputs
                switch(ch) {
                    // Selection inputs
                    case KEY_UP:
                        y = max(0, y-1);
                        break;
                    case KEY_DOWN:
                        y = min(menuSize-1, y+1);
                        break;
                    case KEY_ENTER:
                    case '\n':
                        mode = menuModes[y];
                        break;

                    // Misc Inputs
                    case '?':
                    case KEY_F(1):
                        printHelp(mode);
                        break;
                    case '`':
                    case '~':
                        mode = quit;
                        break;
                }
                break;

            //=======================<Default>========================//
            default:
                mode = quit;
                break;
        }
    }

    //=============================<Cleanup>==============================//
    status = EXIT_SUCCESS;
main_cleanup:
    if(dispOpen) {
        closeDisp(dispData);
    }
    if(listLoaded) {
        rmList(spriteList, freeSpriteEntry);
    }
    return status;
}

//===============================<File Helpers>===============================//
FILE* promptFile(bool openRead, const char * prompt) {
    char buf[128];

    clear();
    printText(kBlackPalette, (prompt == NULL) ? "Enter the file path" : prompt, 0, 0);
    getText(2, 0, buf, 128);

    FILE* fp = fopen(buf, (openRead) ? "r" : "w");
    return fp;
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

            newRow = 12;
            break;
        
        default:
            newRow = 2;
            break;
    }

    printText(kBlackPalette, "Press enter to continue...", newRow, 0);
    getch();
}