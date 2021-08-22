#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <curses.h>

#include "sprite.h"
#include "dispBase.h"

#include "../common/list.h"

//==============================<Menu Handling>===============================//
typedef enum mode_e {menu, sel, new, edit, view, load, save, quit} mode_t;

const char * menuItems[] = {
    "1. Edit Sprite",
    "2. View Sprites",
    "3. New Sprite Sheet"
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
    printText(kDefPalette, (loadFile) ? "Enter the sprite sheet path" : "Enter the save path", 0, 0);
    getText(2, 0, buf, 128);

    FILE* fp = fopen(buf, (loadFile) ? "r" : "w");

    if(fp == NULL) {
        printText(kDefPalette, "*ERROR* Unable to open specified file", 4, 0);
        getch();
    }

    return fp;
}

bool resizeAction(sprite_t * sprite, int ch, bool isTile) {
    if(sprite == NULL) return false;
    if(ch == '\n' || ch == KEY_ENTER) return true;

    short palette = sprite->palette;
    unsigned char height = sprite->height, width = sprite->width;
    unsigned char xOff = sprite->xOff, yOff = sprite->yOff;

    rmSprite(*sprite);
    switch(ch) {
        case KEY_UP:
            height = (height <= 1) ? 1 : height - 1;
            break;
        case KEY_DOWN:
            ++height;
            break;
        case KEY_LEFT:
            width = (width <= 1) ? 1 : width - 1;
            break;
        case KEY_RIGHT:
            ++width;
    }
    if(isTile)
        *sprite = mkBlankTile(palette, width, height);
    else
        *sprite = mkSprite(palette, width, height, xOff, yOff);
    return false;
}

#define printError(msg) clear();printText(kRedPalette, msg, 0, 0); getch()

//==============================<Main Execution>==============================//
int main() {
    // Basic definitions
    int ret, ch;
    FILE* fp;
    sprite_t sprite;
    sprite_t * entry = NULL;
    list_t spriteList = NULL;

    if(spriteList == NULL) {
        fprintf(stderr, "*ERROR* in main: Failed to allocate sprite list\n");
        return EXIT_FAILURE;
    }

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
            case menu: // Main menu
                dispMenu(selY);

                ch = getch();
                if(ch > '0' && ch <= '0' + menuSize) {
                    mode = menuModes[ch - '1'];
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
            
            case sel: // Select entries from the current sheet
                if(spriteList == NULL) {    // Can only be used w/ a sheet
                    mode = menu;
                    break;
                }

                mode = menu;
                break;
            
            case new:   // Create new sprite entries (and potentially new lists)
                mode = menu;
                break;

            case edit:  // Actually edit an element from the sheet
                mode = menu;
                break;

            case view:  // View through loaded sprites
                if(spriteList == NULL) {    // Can only be used w/ a sheet
                    mode = menu;
                    break;
                }
                
                mode = menu;
                break;

            case load:
                // Create the new sprite list
                rmList(spriteList, freeSpriteEntry);
                spriteList = mkList();
                if(spriteList == NULL) {
                    printError("*FATAL ERROR* Unable to allocate new sprite list");
                    mode = quit;
                    break;
                }

                // Open the sprite sheet
                fp = getSpriteFile(true);
                if(fp == NULL) {
                    printError("Failed to open specified file");
                    mode = menu;
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
                
                mode = menu; // Regardless, of success, return to menu
                break;

            case save:
                mode = menu;
                break;
            
            case quit:
                break;
        }
    }

    // Cleanup and exit
    closeDisp();
    rmList(spriteList, freeSpriteEntry);
    return EXIT_SUCCESS;
}