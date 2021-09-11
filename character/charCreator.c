#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <curses.h>

#include "stringUtils.h"
#include "charData.h"
#include "dispChar.h"

//=============================<Menu Definition>==============================//
#define kMenuPrompt "Character Creator"

typedef enum mode_e {
    menu, edit, new, load, save, quit
} mode_t;

const char * menuItems[] = {
    "1. Edit Character",
    "2. New Character",
    "3. Load Character",
    "4. Save Character",
    "5. Quit"
};

const mode_t menuModes[] = {
    edit,
    new,
    load,
    save,
    quit
};

const int menuSize = sizeof(menuItems)/sizeof(menuItems[0]);

//===============================<Global State>===============================//
// Active character data
charData_t curChar;
bool charLoaded = false;

// Generic string buffer
#define bufSize 256
char buf[bufSize];

// Navigation variables
mode_t mode = menu;
int menuSel = 0;

// Display Variables
bool dispInitialized = false;
dispData_t dispData;

//=============================<Helper Functions>=============================//
/**
 * Loads a new current character from a specified file
 * 
 * @param fileName The name of the file to load from
 * 
 * @return 0 on success, <0 on failure
 */
int loadChar(const char * fileName) {
    if(fileName == NULL) return -1;

    if(charLoaded) {
        rmCharData(curChar);
        charLoaded = false;
    }

    FILE * fp = fopen(fileName, "r");
    if(fp == NULL) {
        return -1;
    }

    int ret = loadCharData(fp, &curChar) != 0;
    charLoaded = (ret == 0);

    fclose(fp);
    return ret;
}

/**
 * Saves the current character to the specified file
 * 
 * @param fileName The name of the file to save to
 * 
 * @return 0 on success, < 0 on failure
 */
int saveChar(const char * fileName) {
    if(fileName == NULL || !charLoaded) return -1;

    FILE* fp = fopen(fileName, "w");
    if(fp == NULL) return -1;

    int ret = saveCharData(fp, curChar);
    fclose(fp);
    return ret;

}

/**
 * Prompts the user for text and gets it (returned through buf)
 * 
 * @param prompt The prompt for the user
 */
void promptText(const char * prompt) {
    clearBuffer(&dispData);
    addText(&dispData, kBlackPalette, prompt, 0, 0);
    printBuffer(dispData);

    getText(2, 0, buf, bufSize);
}

//================================<Main Code>=================================//
int main(int argc, char** argv) {
    //==============================<Setup>===============================//
    // Define some local variables
    int status = EXIT_SUCCESS;
    int ch = 0;

    // Either pre-load a character from args or null-initialize the character
    if(argc >= 2) {
        loadChar(argv[1]);
    } 
    if (!charLoaded) {
        curChar = mkCharData();
    }

    // Null-out the character buffer
    for(int i = 0; i < bufSize; i++) {
        buf[i] = 0;
    }

    // Initialize the display
    if(initDisp(&dispData) != 0) {
        fprintf(stderr, "*FATAL ERROR* Failed to initialize display\n");
        status = EXIT_FAILURE;
        goto charCreatorCleanup;
    }
    dispInitialized = true;


    //===============================<Loop>===============================//
    while(mode != quit) {
        switch(mode) {
            case menu:
                // Buffer a print of the menu
                addMenu(&dispData, kMenuPrompt, menuItems, menuSize, menuSel);
                
                // Also buffer a print of load status
                if(!charLoaded) {
                    sprintf(buf, "No Character Loaded");
                } else {
                    sprintf(buf, "Character \"%s\" loaded", 
                            (curChar.name == NULL) ? "<NULL>" : curChar.name);
                }
                addText(&dispData, kBlackPalette, buf, 3 + menuSize, 0);

                printBuffer(dispData);

                ch = getch();
                if(ch >= '1' && ch < '1' + menuSize) {
                    menuSel = ch - '1';
                    mode = menuModes[menuSel];
                    break;
                }

                switch(ch) {
                    case KEY_UP:
                        if(menuSel > 0) menuSel -= 1;
                        break;
                    case KEY_DOWN:
                        if(menuSel < menuSize-1) menuSel += 1;
                        break;
                    case KEY_ENTER:
                    case '\n':
                        mode = menuModes[menuSel];
                }
                break;
            
            //edit, new, load, save,
            case edit:  // Edit a loaded character
                if(!charLoaded) {
                    mode = menu;
                    break;
                }

                mode = menu;
                break;

            case new:
                if(charLoaded) {
                    rmCharData(curChar);
                    charLoaded = false;
                }
                curChar = mkCharData();
                
                // First, get the minimum for a full character: the name
                promptText("What is your character's name?");
                strip(buf);
                if(strlen(buf) == 0) {
                    mode = menu;
                    break;
                }

                curChar.name = calloc(strlen(buf) + 1, sizeof(char));
                charLoaded = true;

                //todo prompt for other strings


                mode = menu;
                break;

            case load:
                promptText("Enter the character file name");
                loadChar(buf);
                mode = menu;
                break;

            case save:
                if(!charLoaded) {
                    mode = menu;
                    break;
                }

                promptText("Enter the save file name");
                saveChar(buf);

                mode = menu;
                break;

            case quit:
                break;

            default:
                mode = menu;
                break;
        }
    }

    //=============================<Cleanup>==============================//
charCreatorCleanup:
    if(charLoaded) {
        rmCharData(curChar);
    }
    if(dispInitialized) {
        closeDisp(dispData);
    }
    return status;
}