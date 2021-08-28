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

//==============================<Main Execution>==============================//
int main() {
    // Basic definitions
    int ret, ch;
    FILE* fp = NULL;
    sprite_t sprite;
    sprite_t * entry = NULL;
    list_t spriteList = NULL;

    // Initialize the display
    dispData_t data;
    ret = initDisp(&data);
    if(ret != 0) {
        fprintf(stderr, "*ERROR* in main: Failed to initialized display\n");
        return EXIT_FAILURE;
    }

    // Main loop
    unsigned int selY = 0;
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
                }
                break;
            
            case sel:   // Select entries from the current sheet
                if(spriteList == NULL) {    // Can only be used w/ a sheet
                    mode = menu;
                    break;
                }

                mode = menu;
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
                drawSprite(data, *entry, data.screenRows/2 - entry->height/2, data.screenCols/2 - entry->width/2);

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
                mode = menu;
                break;

            case view:  // View through loaded sprites
                if(spriteList == NULL) {    // Can only be used w/ a sheet
                    mode = menu;
                    break;
                }

                // Grab the proper sprite
                entry = listGet(spriteList, selY);
                if(entry == NULL) {
                    printError("*FATAL ERROR* Unexpected dead entry in sprite list");
                    mode = quit;
                    break;
                }

                // Update the display itself
                clear();
                printText(kBlackPalette, "Use left and right arrows to navigate, and home to escape", 0, 0);
                drawSprite(data, *entry, data.screenRows/2 - entry->height/2,
                    data.screenCols/2 - entry->width/2);
                
                // Handle input
                ch = getch();
                switch(ch) {
                    case KEY_HOME:
                    case 'q':
                        entry = NULL;   // Clean up after myself
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
    closeDisp();
    rmList(spriteList, freeSpriteEntry);
    freeSpriteEntry(entry);
    return EXIT_SUCCESS;
}