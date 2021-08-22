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
    "3. Load Sprite Sheet",
    "4. Save Sprite Sheet",
    "5. Exit Program"
};

const mode_t menuModes[] = {
    sel,
    view,
    load,
    save,
    quit
};

const unsigned char menuSize = sizeof(menuItems) / sizeof(menuItems[0]);

void dispMenu(unsigned int selected) {
    clear();

    printText(kBlackPalette, "Sprite Editor", 0, 0);

    for(unsigned int i = 0; i < menuSize; i++) {
        printText((i == selected) ? kBlackPalette : kWhitePalette, menuItems[i], 2 + i, 0);
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

//==============================<Main Execution>==============================//
int main() {
    int ret, ch;
    FILE* fp;
    sprite_t tile, sprite;
    list_t spriteList = mkList();

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
    unsigned int selX = 0, selY = 0;
    mode_t mode = menu;
    while(mode != quit) {
        switch(mode) {
            case menu:
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
                        selX = 0;
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
            
            case view:
                if(listLen(spriteList) == 0) { // Exit if no sprites to display
                    selY = 0;
                    mode = menu;
                    break;
                }

                // Display the currently selected sprite
                sprite = *(sprite_t *) listGet(spriteList, selY);
                clear();
                drawSprite(data, sprite, data.screenRows/2-sprite.height/2,
                    data.screenCols/2 - sprite.width/2);

                ch = getch();
                switch(ch) {
                    case KEY_HOME:
                    case '`':
                        selY = 0;
                        mode = menu;
                        break;

                    case KEY_UP:
                        selY -= (selY == 0) ? 0 : 1;
                        break;
                    case KEY_DOWN:
                        selY += (selY + 1 == listLen(spriteList)) ? 0 : 1;
                        break;
                }
                break;



            case load:
                //Attempt to open the file to read sprites
                fp = getSpriteFile(true);

                if(fp == NULL) {
                    mode = menu;
                    break;
                }

                // Create a new list to store the sprites from file
                rmList(spriteList, freeSpriteEntry);
                spriteList = mkList();
                if(spriteList == NULL) {
                    printText(kDefPalette, "*ERROR* Unable to create a new sprite list", 4, 0);
                    getch();

                    fclose(fp);
                    mode = quit;
                    break;
                }

                // Load sprites from file
                sprite_t sprite;
                bool failed = false;

                for(sprite = readSprite(fp); sprite.data != NULL && !failed; sprite = readSprite(fp)) {
                    sprite_t * entry = mkSpriteEntry(sprite);
                    if(entry == NULL) {
                        failed = true;
                        continue;
                    }
                    listAppend(spriteList, entry);
                }

                fclose(fp);
                if(failed) {
                    printText(kDefPalette, "*ERROR* Unable to fully read to sprite list", 4, 0);
                    getch();

                    mode = quit;
                } else {
                    mode = menu;
                }
                break;
            
            case save:
                if(listLen(spriteList) == 0) {
                    mode = menu;
                    break;
                }

                // Attempt to open a file to write out
                fp = getSpriteFile(false);

                if(fp == NULL) {
                    mode = menu;
                    break;
                }

                for(unsigned i = 0; i < listLen(spriteList); i++) {
                    writeSprite(fp, *(sprite_t *)listGet(spriteList, i));
                }

                mode = menu;
                fclose(fp);
                break;

            default:
                mode = quit;

        }
    }

    // Cleanup and exit
    closeDisp();
    if(spriteList != NULL) rmList(spriteList, freeSpriteEntry);

    return EXIT_SUCCESS;
}