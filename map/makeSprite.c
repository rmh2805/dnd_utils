#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <curses.h>
#include "mapDisp.h"

#include "sprite.h"

#include "wallSprites.h"

#include "../common/list.h"
#include "../common/dispBase.h"

//==============================<Menu Handling>===============================//
typedef enum mode_e {menu, sel, new, edit, quit} mode_t;

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
#define kDefSpriteWidth     kTileWidth-1
#define kDefSpriteHeight    kTileHeight-1

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
void addSpriteCenter(dispData_t * data, sprite_t * sprite);

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
    // Create the background sprite
    bg = mkSprite(kDefPalette, kTileWidth, kTileHeight, 0, 0);
    if(bg.data == NULL) {
        fprintf(stderr, "*Fatal Error* Failed to initialize the BG sprite\n");
        goto main_cleanup;
    }
    bgLoaded = true;

    for(int row = 0; row < bg.height; ++row) {
        for(int col = 0; col < bg.width; ++col) {
            ch = ' ';
            if(row == 0 && col == 0) {
                ch = kCellCornerChar;
            } else if (row == 0) {
                ch = kCellHorizChar;
            } else if (col == 0) {
                ch = kCellVertiChar;
            }

            bg.data[row][col] = ch;
        }
    }

    // Initialize the display
    if((ret = initDisp(&dispData)) < 0) {
        fprintf(stderr, "*Fatal Error* Failed to initialize the display (%d)\n", ret);
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
                case edit:
                    ret = false;
                    // fall through
                default:
                    x = 0;
                    y = 0;
            }
        }
        prevMode = mode;
        curs_set(1);

        // Quickly cleanup before main "action"
        if(entryUnlisted) {
            freeSpriteEntry(entry);
            entry = NULL;
            entryUnlisted = false;
        }

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
                    case KEY_HOME:
                        mode = quit;
                        break;
                }
                break;
            //====================<Select a Sprite>===================//
            case sel:
                // Ensure that a list is loaded and has entries
                if(!listLoaded || (ret = listLen(list)) <= 0) {
                    mode = menu;
                    break;
                }

                // Get the currently selected entry
                entry = listGet(list, x);
                if(entry == NULL) {
                    printError("*ERROR* Unable to grab the selected list entry");
                    mode = menu;
                    break;
                }

                // Print the currently selected sprite
                clearBuffer(&dispData);
                addText(&dispData, kBlackPalette, "Use arrow keys to select a sprite", 0, 0);
                sprintf(buf, "%d/%d", x + 1, ret);
                addText(&dispData, kBlackPalette, buf, dispData.screenRows-1, 0);
                addSpriteCenter(&dispData, entry);
                printBuffer(dispData);
                curs_set(0);

                // Handle inputs
                ch = getch();
                switch(ch) {
                    // Handle navigation
                    case KEY_LEFT:
                        x = max(x-1, 0);
                        break;
                    case KEY_RIGHT:
                        x = min(x+1, ret - 1);
                        break;

                    // Sprite manipulation
                    case KEY_ENTER:
                    case '\n':
                        mode = edit;
                        break;

                    // Misc Controls
                    case KEY_HOME:
                    case '`':
                    case '~':
                        mode = menu;
                        break;
                    case '?':
                    case KEY_F(1):
                        printHelp(mode);
                        break;
                }
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

                        mode = edit;
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

            //==================== <Edit "entry">=====================//
            case edit:
            // Make sure that a listed entry is selected
            if(entryUnlisted || entry == NULL || entry->data == NULL) {
                mode = menu;
                break;
            }

            // Draw the edit screen
            clearBuffer(&dispData);
            if(ret) {
                addSpriteCenter(&dispData, &bg);
            }
            addSpriteCenter(&dispData, entry);
            printBuffer(dispData);
            move((dispData.screenRows/2-entry->height/2+entry->yOff) + y,
                    (dispData.screenCols/2-entry->width/2+entry->xOff) + x);

            // Get and act on input
            ch = getch();
            switch(ch) {
                // Navigation inputs
                case KEY_UP:
                    y = max(y-1, 0);
                    break;
                case KEY_DOWN:
                    y = min(y+1, entry->height-1);
                    break;
                case KEY_LEFT:
                    x = max(x-1, 0);
                    break;
                case KEY_RIGHT:
                    x = min(x+1, entry->width-1);
                    break;

                // Control Inputs
                case '\t':
                    ret = !ret;
                    break;
                case KEY_F(10):
                    bg.defPalette = max(bg.defPalette+1, kMinPalette);
                    if(bg.defPalette >= kMaxPalette) {
                        bg.defPalette = kMinPalette;
                    }
                    break;
                case KEY_F(12):
                    entry->defPalette = max(entry->defPalette+1, kMinPalette);
                    if(entry->defPalette >= kMaxPalette) {
                        entry->defPalette = kMinPalette;
                    }
                    break;

                // Offset Inputs
                case KEY_HOME:
                    --entry->yOff;
                    break;
                case KEY_END:
                    ++entry->yOff;
                    break;
                case KEY_DC:
                case 27:
                    --entry->xOff;
                    break;
                case KEY_NPAGE:
                    ++entry->xOff;
                    break;

                // Misc Inputs
                case KEY_F(1):
                    printHelp(mode);
                    break;
                case KEY_F(2):
                case KEY_ENTER:
                case '\n':
                    mode = menu;
                    break;
                
                // Character entry
                case KEY_BACKSPACE:
                case '\b':
                    entry->data[y][x] = '\0';
                    break;
                default:
                    if(ch >= 0x20 && ch <= 0x7E) {
                        entry->data[y][x] = ch;
                    }
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
    if(entryUnlisted) {
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

//===============================<Display Helpers>===============================//
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

        case sel:
            helpPrinter("Use the arrow keys to select an option", 2);
            helpPrinter("Use the home or '~' keys to return to the main menu", 3);

            newRow = 5;
            break;
        
        case edit:
            helpPrinter("Use the arrow keys to navigate", 2);
            helpPrinter("Visible characters will be added to the sprite", 3);
            helpPrinter("Backspace or Delete will put a hole in the sprite", 4);

            helpPrinter("Use the tab key to tobble background visibility", 6);
            helpPrinter("Use the F10 key to cycle the background palette", 7);
            helpPrinter("Use the F12 key to cycle sprite palette", 8);

            helpPrinter("Use Home and End to change y offset", 10);
            helpPrinter("Use Delete and PgDn to change x offset", 11);

            helpPrinter("Use the F2 key or enter to return to the menu", 13);

            newRow = 15;
            break;
        
        default:
            newRow = 2;
            break;
    }

    printText(kBlackPalette, "Press enter to continue...", newRow, 0);
    getch();
}


void addSpriteCenter(dispData_t * data, sprite_t * sprite) {
    addSprite(data, *sprite, 0, 
                data->screenRows/2 - sprite->height/2,
                data->screenCols/2 - sprite->width/2);
}