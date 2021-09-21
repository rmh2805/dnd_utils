#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <curses.h>

#include "stringUtils.h"
#include "charData.h"
#include "dispChar.h"

//=============================<Menu Definition>==============================//

typedef enum mode_e {
    menu, edit, new, load, save, quit, eBio, eStat, eProf, eMisc, eWeap
} mode_t;

// Main menu definition
#define kMenuPrompt "Character Creator"

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

// Edit menu definition
const char * editMenuItems[] = {
    "1. Edit Bio Information",
    "2. Edit Stats",
    "3. Edit Proficiencies",
    "4. Edit Weapons",
    "5. Edit Misc Properties",
    "6. Back to Main Menu"
};

const mode_t editMenuModes[] = {
    eBio,
    eStat,
    eProf,
    eWeap,
    eMisc,
    menu
};

const int editMenuSize = sizeof(editMenuItems)/sizeof(editMenuItems[0]);

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
int editSel = 0;
int sel = 0;
int sel2 = 0;

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

void doMenuUpdate(bool isEdit) {
    int ch = getch();

    int * sel = (isEdit) ? &editSel : &menuSel;
    int size = (isEdit) ? editMenuSize : menuSize;
    const mode_t * modes = (isEdit) ? editMenuModes : menuModes;

    if(ch >= '1' && ch < '1' + size) {
        *sel = ch - '1';
        mode = modes[*sel];
        return;
    }

    switch(ch) {
        case KEY_UP:
            if(*sel > 0) *sel -= 1;
            break;
        case KEY_DOWN:
            if(*sel < size-1) *sel += 1;
            break;
        case KEY_ENTER:
        case '\n':
            mode = modes[*sel];
    }
}

//================================<Main Code>=================================//
int main(int argc, char** argv) {
    //==============================<Setup>===============================//
    // Define some local variables
    int status = EXIT_SUCCESS;
    int ch = 0;
    weapon_t weapon;

    int * intRef = NULL;
    char ** strRef = NULL;

    // Either pre-load a character from args or null-initialize the character
    if(argc >= 2) {
        loadChar(argv[1]);
    } 
    if (!charLoaded) {
        curChar = mkCharData();
        if(argc >= 2) {
            curChar.name = calloc(strlen(argv[1]) + 1, sizeof(char));
            if(curChar.name == NULL) {
                fprintf(stderr, "*ERROR* Failed to allocate for provided name\n");
            } else {
                strcpy(curChar.name, argv[1]);
                charLoaded = true;
            }
        }
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

    // Set the initial mode
    mode = (charLoaded) ? edit : menu;

    //===============================<Loop>===============================//
    while(mode != quit) {
        if(!charLoaded && (mode == save || mode == edit || mode == eBio || 
                mode == eStat || mode == eProf || mode == eMisc || 
                mode == eWeap)) {
            mode = menu;
        }

        switch(mode) {
            case menu:  // Main menu
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

                doMenuUpdate(false);
                break;
            
            case edit:  // Edit a loaded character
                if(!charLoaded) {
                    mode = menu;
                    break;
                }
                
                sprintf(buf, "Editing character \"%s\"", curChar.name);
                addMenu(&dispData, buf, editMenuItems, editMenuSize, editSel);
                printBuffer(dispData);

                doMenuUpdate(true);
                if(menu != edit) {
                    sel = 0;
                    sel2 = 0; 
                }
                break;

            case new:   // Create a new character
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
                
                // Allocate space for the name in the char struct
                curChar.name = calloc(strlen(buf) + 1, sizeof(char));
                if(curChar.name == NULL) {
                    mode = menu;
                    break;
                }

                // Copy over the name, mark the new char as loaded, and jump to 
                // edit mode for further setup
                charLoaded = true;
                strcpy(curChar.name, buf);

                
                mode = edit;
                break;

            case load:
                promptText("Enter the character file name");
                loadChar(buf);
                mode = edit;
                break;

            case save:
                promptText("Enter the save file name");
                saveChar(buf);

                mode = menu;
                break;

            case eBio:
                // Print the current stat data
                clearBuffer(&dispData);

                sprintf(buf, "       Name: %s", (curChar.name == NULL) ? "" : curChar.name);
                addText(&dispData, (sel == 0) ? kWhitePalette : kBlackPalette, buf, 0, 0);
                sprintf(buf, "Player Name: %s", (curChar.playerName == NULL) ? "" : curChar.playerName);
                addText(&dispData, (sel == 1) ? kWhitePalette : kBlackPalette, buf, 1, 0);

                sprintf(buf, " Base Class: %s", (curChar.baseClass == NULL) ? "" : curChar.baseClass);
                addText(&dispData, (sel == 2) ? kWhitePalette : kBlackPalette, buf, 2, 0);
                sprintf(buf, " Background: %s", (curChar.background == NULL) ? "" : curChar.background);
                addText(&dispData, (sel == 3) ? kWhitePalette : kBlackPalette, buf, 3, 0);
                sprintf(buf, "       Race: %s", (curChar.race == NULL) ? "" : curChar.race);
                addText(&dispData, (sel == 4) ? kWhitePalette : kBlackPalette, buf, 4, 0);

                printBuffer(dispData);

                // Menu navigation
                ch = getch();
                switch(ch) {
                    case '`':
                        mode = edit;
                        break;
                    case KEY_DOWN:
                        sel += 1;
                        if(sel >= 5) sel = 5;
                        break;
                    case KEY_UP:
                        sel -= 1;
                        if(sel < 0) sel = 0;
                        break;
                    case KEY_ENTER:
                    case '\n':
                        // Get the selected field to edit
                        strRef = NULL;
                        switch(sel) {
                            case 0:
                                strRef = &curChar.name;
                                break;
                            case 1:
                                strRef = &curChar.playerName;
                                break;
                            case 2:
                                strRef = &curChar.baseClass;
                                break;
                            case 3:
                                strRef = &curChar.background;
                                break;
                            case 4:
                                strRef = &curChar.race;
                                break;
                            default:
                                strRef = NULL;
                        }
                        if(strRef == NULL) {
                            break;
                        }

                        // If the field is not null, free it
                        if(*strRef != NULL) {
                            free(*strRef);
                            *strRef = NULL;
                        }

                        // Prompt for the new value
                        promptText("Enter the field's new value");

                        // Attempt to alloc a string in the field
                        *strRef = calloc(strlen(buf) + 1, sizeof(char));
                        if(*strRef == NULL) {
                            break;
                        }

                        // Copy across the gathered value
                        strcpy(*strRef, buf);

                        break;
                }
                break;

            case eStat:
                clearBuffer(&dispData);
                addStatSel(&dispData, curChar, 0, 0, false, sel);
                printBuffer(dispData);

                ch = getch();
                switch(ch) {
                    case KEY_LEFT:
                        sel -= 1;
                        if(sel < 0) sel = 0;
                        break;
                    case KEY_RIGHT:
                        sel += 1;
                        if(sel >= kNStats) sel = kNStats - 1;
                        break;
                    case KEY_UP:
                        modStat(&curChar, sel, 1);
                        break;
                    case KEY_DOWN:
                        modStat(&curChar, sel, -1);
                        break;
                    case '`':
                    case 'q':
                    case 'Q':
                        mode = edit;
                        break;
                }
                break;

            case eProf:
                clearBuffer(&dispData);
                addProfSel(&dispData, curChar, 0, 0, sel);
                printBuffer(dispData);
                
                ch = getch();
                switch(ch) {
                    case KEY_UP:
                        sel -= 1;
                        if(sel < 0) sel = 0;
                        break;
                    case KEY_DOWN:
                        sel += 1;
                        if(sel >= kNProfs) sel = kNProfs - 1;
                        break;
                    case '`':
                        mode = edit;
                        break;
                    case KEY_ENTER:
                    case '\n':
                        setProfIdx(&curChar, sel, !getProfIdx(curChar, sel));
                        break;
                }
                break;
            
            case eMisc:
                // Print the misc lines on the screen
                intRef = NULL;
                clearBuffer(&dispData);
                
                sprintf(buf, "Level: %d", curChar.level);
                addText(&dispData, (sel == 0) ? kWhitePalette : kBlackPalette, buf, 0, 0);
                if(sel == 0) intRef = &curChar.level;
                
                sprintf(buf, "Proficiency Bonus: %d", curChar.profBonus);
                addText(&dispData, (sel == 1) ? kWhitePalette : kBlackPalette, buf, 1, 0);
                if(sel == 1) intRef = &curChar.profBonus;

                sprintf(buf, "Skill Bonus: %d", curChar.skillBonus);
                addText(&dispData, (sel == 2) ? kWhitePalette : kBlackPalette, buf, 2, 0);
                if(sel == 2) intRef = &curChar.skillBonus;

                sprintf(buf, "Max HP: %d", curChar.maxHP);
                addText(&dispData, (sel == 3) ? kWhitePalette : kBlackPalette, buf, 3, 0);
                if(sel == 3) intRef = &curChar.maxHP;

                sprintf(buf, "Current HP: %d", curChar.curHP);
                addText(&dispData, (sel == 4) ? kWhitePalette : kBlackPalette, buf, 4, 0);
                if(sel == 4) intRef = &curChar.curHP;

                sprintf(buf, "Temporary HP: %d", curChar.tmpHP);
                addText(&dispData, (sel == 5) ? kWhitePalette : kBlackPalette, buf, 5, 0);
                if(sel == 5) intRef = &curChar.tmpHP;

                printBuffer(dispData);

                // Handle input
                ch = getch();
                switch(ch) {
                    case KEY_UP:
                        sel -= 1;
                        if(sel < 0) sel = 0;
                        break;

                    case KEY_DOWN:
                        sel += 1;
                        if(sel > 5) sel = 5;
                        break;

                    case KEY_LEFT:
                        *intRef -= 1;
                        break;

                    case KEY_RIGHT:
                        *intRef += 1;
                        break;

                    case '`':
                        mode = edit;
                        break;
                    
                }

                break;
            
            case eWeap:
                weapon = curChar.weapons[sel];

                // Update the display
                clearBuffer(&dispData);
                
                sprintf(buf, "Weapon %d/3", sel + 1);
                addText(&dispData, kBlackPalette, buf, 0, 0);

                sprintf(buf, "Name: %s", weapon.name);
                addText(&dispData, (sel2 == 0) ? kWhitePalette : kBlackPalette, buf, 2, 0);

                sprintf(buf, "Attack Bonus: %hhd", weapon.atkBonus);
                addText(&dispData, (sel2 == 1) ? kWhitePalette : kBlackPalette, buf, 3, 0);

                sprintf(buf, "Damage Type: %s", weapon.dmgType);
                addText(&dispData, (sel2 == 2) ? kWhitePalette : kBlackPalette, buf, 4, 0);

                sprintf(buf, "Base Damage: %hhd", weapon.baseDamage);
                addText(&dispData, (sel2 == 3) ? kWhitePalette : kBlackPalette, buf, 5, 0);

                sprintf(buf, "Damage Die: %hhu", weapon.dmgDie);
                addText(&dispData, (sel2 == 4) ? kWhitePalette : kBlackPalette, buf, 6, 0);

                printBuffer(dispData);

                ch = getch();
                switch(ch) {
                    case KEY_UP:
                        sel2 += 1;
                        if(sel2 > 4) sel2 = 4;
                        break;
                    case KEY_DOWN:
                        sel2 -= 1;
                        if(sel2 < 0) sel2 = 0;
                        break;
                    case KEY_RIGHT:
                        sel += 1;
                        if(sel >= kNWeapons) sel = 0;
                        break;
                    case KEY_LEFT:
                        sel -= 1;
                        if(sel < 0) sel = kNWeapons - 1;
                        break;
                    case '`':
                        mode = edit;
                        break;
                    case '\n':
                    case KEY_ENTER:
                        //todo Edit here
                        break;
                }
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