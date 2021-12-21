#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <curses.h>
#include "mapDisp.h"

#include "sprite.h"

#include "../common/list.h"
#include "../common/dispBase.h"

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

//===========================<Constant Definitions>===========================//
#define kDefSpriteWidth     3
#define kDefSpriteHeight    3

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
int main() {
    int status = EXIT_FAILURE;  // The exit status to return

    int ch, ret;    // Ints to hold input data and return values respectively
    char buf[80];   // A character buffer

    dispData_t dispData;        // Display Data store
    bool dispOpen = false;      // Set to true only between disp open and close

    list_t list = NULL;         // A list to store all sprites
    bool listLoaded = false;    // Set true iff the list is loaded & not cleaned

    sprite_t sprite;            // A variable to hold a single sprite on stack

    sprite_t bg;                // A variable to hold a bacground sprite on stack
    bool bgLoaded = false;      // Set true iff bg is loaded & not cleaned

    sprite_t * entry = NULL;    // A variable to hold a single sprite from heap
    bool entryUnlisted = false; // Set to true iff the entry is not in the list

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
            switch(mode) {
                case new:
                    x = kDefSpriteWidth;
                    y = kDefSpriteHeight;
                    break;
                default:
                    x = 0;
                    y = 0;
            }
        }
        prevMode = mode;
        curs_set(1);

        switch(mode) {
            //=======================<Main Menu>======================//
            case menu:
                // Display the menu
                clearBuffer(&dispData);
                addMenu(&dispData, "MakeSprite", menuItems, menuSize, y);
                sprintf(buf, "%d sprites in the list", (list == NULL) ? 0 : listLen(list));
                addText(&dispData, kBlackPalette, buf, menuSize + 3, 0);
                printBuffer(dispData);

                // Get the next input
                ch = getch();

                // If the input was a number on the menu, select it and treat input as enter
                if(ch >= '1' && ch <= min('9', '0' + menuSize)) {
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
            //====================<Select a Sprite>===================//
            case sel:
                // Ensure that a list is loaded
                if(!listLoaded) {
                    mode = menu;
                    break;
                }

                mode = menu;
                break;
            
            //=================<Create a New Sprite>==================//
            case new:
                // Add a block with the same size as the sprite to the display
                clearBuffer(&dispData);
                addText(&dispData, kBlackPalette, "Use arrow keys to resize, enter to confirm", 0, 0);

                for(int dRow = 0; dRow < y; ++dRow) {
                    int row = (dispData.screenRows/2 - y/2) + dRow;
                    for(int dCol = 0; dCol < x; ++dCol) {
                        int col = (dispData.screenCols/2 - x/2) + dCol;
                        addText(&dispData, kWhitePalette, " ", row, col);
                    }
                }

                printBuffer(dispData);
                curs_set(0);

                // Handle input
                ch = getch();
                switch(ch) {
                    // Dimension modification
                    case KEY_DOWN:
                        y = min(y+1, dispData.screenRows);
                        break;
                    case KEY_UP:
                        y = max(y-1, 1);
                        break;
                    case KEY_RIGHT:
                        x = min(x+1, dispData.screenCols);
                        break;
                    case KEY_LEFT:
                        x = max(x-1, 1);
                        break;

                    // Dimension accept
                    case KEY_ENTER:
                    case '\n':
                        // If there is no list yet, create one
                        if(!listLoaded) {
                            list = mkList();
                            if(list == NULL) {
                                printError("*ERROR* Unable to allocate a new list");
                                mode = menu;
                                break;
                            }
                            listLoaded = true;
                        }


                        // Ensure that the entry variable is available
                        if(entry != NULL && entryUnlisted) {
                            freeSpriteEntry(entry);
                            entryUnlisted = false;
                        }

                        // Create a new sprite on the heap
                        entry = mkSpriteEntry(kEmptySprite);
                        if(entry == NULL) {
                            printError("*ERROR* unable to put a new sprite on the heap");
                            mode = menu;
                            break;
                        }
                        entryUnlisted = true;

                        // Create a new sprite with the given dimensions
                        sprite = mkSprite(kDefPalette, x, y, 0, 0);
                        if(sprite.data == NULL) {
                            printError("*ERROR* Unable to allocate a new sprite");
                            mode = menu;
                            break;
                        }

                        // Copy that sprite onto the heap
                        *entry = sprite;

                        // Append that entry onto the list
                        ret = listAppend(list, entry);
                        if(ret < 0) {
                            printError("*ERROR* Failed to place entry on list");
                            mode = menu;
                            break;
                        }
                        entryUnlisted = false;

                        mode = menu;
                        break;

                    // Misc Controls
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
        rmList(list, freeSpriteEntry);
    }
    if(entry != NULL && entryUnlisted) {
        freeSpriteEntry(entry);
    }
    if(bgLoaded) {
        rmSprite(bg);
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

            newRow = 5;
            break;
        
        case new:
            helpPrinter("Use the arrow keys to resize the sprite", 2);
            helpPrinter("Use enter to accept the current dimensions", 3);
            helpPrinter("Use the home or '~' keys to return to main menu", 4);

            newRow = 6;
            break;
        
        default:
            newRow = 2;
            break;
    }

    printText(kBlackPalette, "Press enter to continue...", newRow, 0);
    getch();
}