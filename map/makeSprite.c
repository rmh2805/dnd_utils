#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <curses.h>

#include "sprite.h"
#include "dispBase.h"

#include "../common/list.h"

//==============================<Menu Handling>===============================//
typedef enum mode_e {menu, tile, sprite, load, save, quit} mode_t;

const char * menuItems[] = {
    "1. Edit Tile",
    "2. Edit Sprite",
    "3. Save Sprite Sheet",
    "4. Load Sprite Sheet",
    "5. Exit Program"
};

const mode_t menuModes[] = {
    tile,
    sprite,
    save,
    load,
    quit
};

const unsigned int menuSize = sizeof(menuItems) / sizeof(menuItems[0]);

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

//==============================<Main Execution>==============================//
int main() {
    char buf[128];
    int ret;
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

                int ch = getch();
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
            
            case load:
                clear();
                printText(kDefPalette, "Enter the path of the sprite sheet", 0, 0);
                getText(2, 0, buf, 128);

                //Attempt to open the file to read sprites
                FILE* fp = fopen(buf, "r");

                if(fp == NULL) {
                    printText(kDefPalette, "*ERROR* Unable to open specified file", 4, 0);
                    getch();

                    mode = menu;
                    break;
                }

                // todo Clear old sprite list and allocate new one
                // todo Load sprites from file

                fclose(fp);
                mode = menu;
                break;
            
            default:
                mode = quit;

        }
    }

    // Cleanup and exit
    closeDisp();
    rmList(spriteList, freeSpriteEntry);

    return EXIT_SUCCESS;
}