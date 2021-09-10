#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <curses.h>

#include "sprite.h"
#include "dispBase.h"

#include "../common/list.h"

//==============================<Menu Handling>===============================//
typedef enum mode_e {menu, sel, new, dim, edit, view, load, save, quit} mode_t;

const char * menuItems[] = {
    "1. Edit Sprite",
    "2. View Sprites",
    "3. New Sprite Sheet",
    "4. Load Sprite Sheet",
    "5. Save Sprite Sheet",
    "6. Exit Program"
};

const mode_t menuModes[] = {
    sel,
    view,
    new,
    load,
    save,
    quit
};

const unsigned char menuSize = sizeof(menuItems) / sizeof(menuItems[0]);

void dispMenu(unsigned int selected) {
    clear();

    printText(kBlackPalette, "Sprite Editor", 0, 0);

    for(unsigned int i = 0; i < menuSize; i++) {
        printText((i == selected) ? kWhitePalette : kBlackPalette, menuItems[i], 2 + i, 0);
    }
}

//===============================<Misc Helpers>===============================//
sprite_t * mkSpriteEntry(sprite_t sprite) {
    sprite_t * ptr = malloc(sizeof(sprite_t));
    if(ptr == NULL) return NULL;
    *ptr = sprite;
    return ptr;
}

void freeSpriteEntry(void * data) {
    if(data == NULL) return;
    rmSprite(*(sprite_t *) data);
    free(data);
}

FILE* getSpriteFile(bool loadFile) {
    char buf[128];

    clear();
    printText(kBlackPalette, (loadFile) ? "Enter the sprite sheet path" : "Enter the save path", 0, 0);
    getText(2, 0, buf, 128);

    FILE* fp = fopen(buf, (loadFile) ? "r" : "w");
    return fp;
}

#define printError(msg) clear();printText(kRedPalette, msg, 0, 0); getch()

int doSelSprite(dispData_t * data, list_t spriteList, int selection, const char * prompt) {
    if(spriteList == NULL) {
        return -1;
    }

    // Grab the proper sprite
    sprite_t * entry = listGet(spriteList, selection);
    if(entry == NULL) {
        printError("*FATAL ERROR* Unexpected dead entry in sprite list");
        return -1;
    }

    // Update the display itself
    printText(kBlackPalette, prompt, 0, 0);
    clearBuffer(data);
    addSprite(data, *entry, data->screenRows/2 - entry->height/2,
        data->screenCols/2 - entry->width/2);
    printBuffer(*data);
    
    
    return 0;
}


#define kSelPrompt "Use left and right arrows to navigate, enter to select, and home to escape"
#define kViewPrompt "Use left and right arrows to navigate and home to escape"

#define helpPrinter(msg, row) printText(kBlackPalette, msg, row, 0)

void printHelp(mode_t mode) {
    clear();
    printText(kBlackPalette, "Help Prompt", 0, 0);
    int newRow = 2;
    
    switch(mode) {
        case menu:
            helpPrinter("Use the arrow keys and enter to select an option", 2);
            helpPrinter("Alternatively, you can select the numbered options with their keys", 3);

            helpPrinter("Edit Sprite will have you select a sprite in the loaded sheet to edit", 5);
            helpPrinter("View Sprite will allow you to view all entries in the list", 6);
            helpPrinter("New Sprite Sheet will have you generate a new sheet of sprites", 7);
            helpPrinter("Load Sprite Sheet will prompt you to load a sprite sheet from file", 8);
            helpPrinter("Save Sprite Sheet saves the current sprite sheet out to file", 9);
            helpPrinter("Quit... quits (real original, I know)", 10);

            helpPrinter("Worth noting, attempting to backspace in text entry currently results in", 12);
            helpPrinter("corruption. Instead, the delete key currently fills its role", 13);
            newRow = 15;
            break;
        
        case edit:
            helpPrinter("The enter key will finalize the current sprite", 2);
            helpPrinter("Arrow keys move the cursor in the sprite", 3);
            helpPrinter("The Home key offsets the sprite down (shift to reverse)", 4);
            helpPrinter("The End key offsets the sprite to the right (shift to reverse)", 5);
            helpPrinter("Page Up shifts the sprite to the next palette", 6);
            helpPrinter("Delete makes the selected char transparent", 7);
            helpPrinter("Visible character keys place chars in the sprite", 8);
            newRow = 10;
            break;
        
        default:
            newRow = 2;
            break;
    }

    printText(kBlackPalette, "Press enter to continue...", newRow, 0);
    getch();
}

//==============================<Main Execution>==============================//
int main() {
    // Basic definitions
    int ret, ch;
    FILE* fp = NULL;
    sprite_t sprite;
    sprite_t * tile = NULL, * entry = NULL;
    list_t spriteList = NULL;

    // Initialize the display
    dispData_t data;
    ret = initDisp(&data);
    if(ret != 0) {
        fprintf(stderr, "*ERROR* in main: Failed to initialized display\n");
        return EXIT_FAILURE;
    }

    // Main loop
    unsigned int selX = 0, selY = 0;
    mode_t mode = menu;
    while(mode != quit) {
        switch(mode) {
            case menu:  // Main menu
                dispMenu(selY);

                ch = getch();
                if(ch > '0' && ch <= '0' + menuSize) {
                    mode = menuModes[ch - '1'];
                    break;
                } else if (ch == KEY_HOME) {
                    mode = quit;
                    break;
                }

                switch(ch) {
                    case KEY_ENTER:
                    case '\n':
                    case ' ':
                        mode = menuModes[selY];
                        selY = 0;
                        break;
                    case KEY_UP:
                        selY = (selY == 0) ? selY : selY - 1;
                        break;
                    case KEY_DOWN:
                        selY = (selY + 1 == menuSize) ? selY : selY + 1;
                        break;
                    case '?':
                        printHelp(menu);
                        break;
                }
                break;
            
            case sel:   // Select entries from the current sheet
                if(spriteList == NULL) {    // Can only be used w/ a sheet
                    mode = menu;
                    break;
                }

                // Display the currently selected sprite
                if(selY == 0) {
                    // Prompt to create a new sprite
                    clear();
                    printText(kBlackPalette, kSelPrompt, 0, 0);
                    printText(kBlackPalette, "New Sprite", data.screenRows/2, data.screenCols/2-5);
                } else if(doSelSprite(&data, spriteList, selY, kSelPrompt) < 0) {
                    mode = quit;
                    break;
                }


                ch = getch();
                switch(ch) {
                    case KEY_ENTER:
                    case '\n':
                        entry = listGet(spriteList, selY);
                        if(selY == 0) {
                            // Make a new sprite with the same size as the basic tile
                            sprite = mkBlankTile(kDefPalette, entry->width, entry->height);
                            if(sprite.data == NULL) {
                                printError("*ERROR* Failed to allocate data for new sprite");
                                mode = menu;
                                break;
                            }

                            // Wrap the new sprite into an entry
                            entry = mkSpriteEntry(sprite);
                            if(entry == NULL) {
                                rmSprite(sprite);
                                printError("*ERROR* Failed to allocate data for new sprite entry");
                                mode = menu;
                                break;
                            }

                            // Append the entry to list and shift to resize mode
                            if(listAppend(spriteList, entry) < 0) {
                                freeSpriteEntry(entry);
                                printError("*ERROR* Failed to put new sprite on list");
                                mode = menu;
                                break;
                            }
                            mode = dim;
                        } else {
                            selX = 0;
                            selY = 0;
                            mode = edit;
                        }
                        break;
                    case KEY_HOME:
                    case 'q':
                        mode = menu;
                        break;
                    case KEY_LEFT:
                        selY = (selY == 0) ? listLen(spriteList)-1 : selY-1;
                        break;
                    case KEY_RIGHT:
                        selY = (selY + 1 == listLen(spriteList)) ? 0 : selY+1;
                        break;
                }
                break;
            
            case new:   // Create a new sprite sheet
                // Free the old sprite list and create a new one
                rmList(spriteList, freeSpriteEntry);
                spriteList = mkList();
                if(spriteList == NULL) {
                    printError("*FATAL ERROR* Failed to allocate new sprite list");
                    mode = quit;
                    break;
                }

                // Create a new basic tile to resize
                entry = mkSpriteEntry(mkBlankTile(kDefPalette, 8, 5));
                if(entry == NULL || entry->data == NULL) {
                    printError("*FATAL ERROR* Failed to allocate a new tile");
                    mode = quit;
                    break;
                }

                // Insert the new tile at the start of the list
                if(listAppend(spriteList, entry) < 0) {
                    printError("*FATAL ERROR* Failed to insert blank tile at start of list");
                    mode = quit;
                }
                
                // Switch mode to dimension setting and continue loop
                mode = dim;
                break;

            case dim:   // Set the dimensions of a sprite
                if(spriteList == NULL || entry == NULL) {    // Can only be used w/ a sheet & entry
                    mode = menu;
                    break;
                }

                // Print the current size of the sprite
                clear();
                printText(kBlackPalette, "Use arrows to resize, and enter to confirm", 0, 0);
                clearBuffer(&data);
                addSprite(&data, *entry, data.screenRows/2 - entry->height/2, data.screenCols/2 - entry->width/2);
                printBuffer(data);

                // todo Handle input 
                unsigned char height = entry->height, width = entry->width;
                ch = getch();
                switch(ch) {
                    case KEY_UP:
                        height = (height <= 1) ? 1 : height - 1;
                        break;
                    case KEY_DOWN:
                        height += 1;
                        break;
                    case KEY_LEFT:
                        width = (width <= 1) ? 1 : width - 1;
                        break;
                    case KEY_RIGHT:
                        width += 1;
                        break;
                    case KEY_ENTER:
                    case '\n':
                        // Break out the transition cases
                        switch(listLen(spriteList)) {
                            case 0: // Error out on empty list (undefined behaviour)
                                printError("*FATAL ERROR* Tried to set entry in an empty tile list");
                                mode = quit;
                                break;
                            case 1: // Return to menu if this is a tile
                                mode = menu;
                                entry = NULL;
                                selY = 0;
                                break;
                            default: // Otherwise go on to edit
                                selX = 0;
                                selY = 0;
                                mode = edit;
                                break;
                        }
                        continue;
                }

                rmSprite(*entry);
                *entry = mkBlankTile(kDefPalette, width, height);
                if(entry == NULL) {
                    //todo make this more graceful
                    printError("*FATAL ERROR* Failed to allocate memory for resize");
                    mode = quit;
                    break;
                }

                break;

            case edit:  // Actually edit an element from the sheet
                curs_set(1);
                if(spriteList == NULL || entry == NULL) {   // Can only be used w/ a sheet & entry
                    mode = menu;
                    break;
                }

                // Ensure that the tile is loaded for a background
                if(tile == NULL) {
                    tile = listGet(spriteList, 0);
                    if(tile == NULL) {
                        printError("*FATAL ERROR* Failed to extract blank tile from list");
                        mode = quit;
                        break;
                    }
                }

                // Update the display
                clearBuffer(&data);
                addSprite(&data, *tile, data.screenRows/2-tile->height/2, 
                                        data.screenCols/2-tile->width/2);
                addSprite(&data, *entry, data.screenRows/2-tile->height/2, 
                                          data.screenCols/2-tile->width/2);
                printBuffer(data);
                move(data.screenRows/2 - tile->height/2 + entry->yOff + selY, 
                     data.screenCols/2 - tile->width/2 + entry->xOff + selX);

                // Handle input
                ch = getch();
                switch(ch) {
                    // Misc keys
                    case KEY_ENTER:
                    case '\n':
                        tile = NULL;
                        entry = NULL;
                        selY = 0;
                        mode = menu;
                        break;
                    
                    case '?':
                        printHelp(edit);
                        break;
                    
                    // Cursor keys
                    case KEY_UP:
                        if(selY > 0) --selY;
                        break;
                    case KEY_DOWN:
                        ++selY;
                        if(selY == entry->height) --selY;
                        break;
                    case KEY_LEFT:
                        if(selX > 0) --selX;
                        break;
                    case KEY_RIGHT:
                        ++selX;
                        if(selX == entry->width) --selX;
                        break;

                    // Offset keys
                    case KEY_HOME:
                        entry->yOff += 1;
                        if(entry->height + entry->yOff > tile->height) entry->yOff -= 1;
                        break;
                    case KEY_SHOME:
                        if(entry->yOff > 0) entry->yOff -= 1;
                        break;
                    case KEY_END:
                        entry->xOff += 1;
                        if(entry->width + entry->xOff > tile->width) entry->xOff -= 1;
                        break;
                    case KEY_SEND:
                        if(entry->xOff > 0) entry->xOff -= 1;
                        break;
                            
                    // Palette swap keys
                    case KEY_PPAGE: // Entry palette swap
                        entry->palette += 1;
                        if(entry->palette > kMaxPalette) entry->palette = kMinPalette;
                        break;
                    
                    // Data entry keys
                    case KEY_DC:
                    case 27:
                        entry->data[selY][selX] = 0;
                        break;
                    
                    default:
                        if(ch < 0x20 || ch > 0x7e) { // If char is non-visible
                            break;
                        }
                        entry->data[selY][selX] = ch;
                }
                break;

            case view:  // View through loaded sprites
                if(spriteList == NULL) {    // Can only be used w/ a sheet
                    mode = menu;
                    break;
                }

                // Grab the proper sprite
                ret = doSelSprite(&data, spriteList, selY, kViewPrompt);
                if(ret < 0) {
                    mode = quit;
                    break;
                }
                
                // Handle input
                ch = getch();
                switch(ch) {
                    case KEY_HOME:
                    case 'q':
                        mode = menu;
                        break;
                    case KEY_LEFT:
                        ret = listLen(spriteList);
                        selY = ((selY == 0) ? (unsigned)ret : selY) - 1;
                        break;
                    case KEY_RIGHT:
                        selY = (selY + 1) % listLen(spriteList);
                        break;
                }
                break;

            case load:  // Load a sprite sheet from file
                // Open the sprite sheet
                fp = getSpriteFile(true);
                if(fp == NULL) {
                    printError("Failed to open specified file");
                    mode = menu;
                    break;
                }

                // Create the new sprite list
                rmList(spriteList, freeSpriteEntry);

                spriteList = mkList();
                if(spriteList == NULL) {
                    printError("*FATAL ERROR* Unable to allocate new sprite list");
                    mode = quit;
                    break;
                }

                // For each sprite in the sheet
                for(sprite = readSprite(fp); sprite.data != NULL; sprite = readSprite(fp)) {
                    entry = mkSpriteEntry(sprite);

                    
                    if(entry == NULL) { // On failure to place sprite in heap...
                        // Clear out the existing menu items
                        rmList(spriteList, freeSpriteEntry);
                        spriteList = NULL;

                        // Inform the user and drop back to menu
                        printError("*ERROR* File size too large");
                        break;
                    } else {    // On success in placing sprite in heap...
                        listAppend(spriteList, entry);  // Add sprite to list
                    }
                }
                
                // Regargless of success, clean up and return to menu
                fclose(fp);
                fp = NULL;
                entry = NULL;
                mode = menu;
                break;

            case save:  // Write the current sprite sheet out to file
                if(spriteList == NULL) {    // Can only be used w/ a sheet
                    mode = menu;
                    break;
                }

                // Open the save file
                fp = getSpriteFile(false);

                // Write out all sprites
                for(unsigned i = 0; i < listLen(spriteList); i++) {
                    entry = listGet(spriteList, i);
                    if(entry == NULL) continue;
                    writeSprite(fp, *entry);
                }

                // Clean up and return to main menu
                fclose(fp);
                fp = NULL;
                entry = NULL;
                mode = menu;
                break;
            
            case quit:  // Nothing to do but wait for the loop to finish
                break;
        }
    }

    // Cleanup and exit
    closeDisp(data);
    rmList(spriteList, freeSpriteEntry);
    freeSpriteEntry(entry);
    freeSpriteEntry(tile);
    return EXIT_SUCCESS;
}