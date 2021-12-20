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
typedef enum mode_e {menu, quit} mode_t;

const char * menuItems[] = {
    "1. Exit Program"
};

const mode_t menuModes[] = {
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