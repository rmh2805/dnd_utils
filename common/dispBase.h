#ifndef _DISPBASE_H_
#define _DISPBASE_H_

#include <stdlib.h>
#include <stdio.h>
#include <curses.h>

// Define common palette numbers
#define kBlackPalette 1
#define kWhitePalette 2
#define kRedPalette 3
#define kGreenPalette 4
#define kBluePalette 5
#define kYellowPalette 6
#define kMagentaPalette 7
#define kCyanPalette 8

#define kDefPalette kWhitePalette
#define kMinPalette 1
#define kMaxPalette 8

typedef struct drawPair_s {
    short palette;
    char ch;
} drawPair_t;

// Define a persistent data structure
typedef struct dispData_s {
    int screenRows;
    int screenCols;

    drawPair_t ** data;
} dispData_t;

//=============================<Init and Cleanup>=============================//
/**
 * Core display initialization
 * 
 * @param data A return pointer for a display data struct
 * @return 0 iff display was initialized correctly
 */
int initDisp(dispData_t* data);

/**
 * Core display close
 * 
 * @return 0 iff display was closed correctly
 */
int closeDisp(dispData_t data);

//=============================<Buffer Handling>==============================//
/**
 * Adds text to the frame buffer
 * 
 * @param data The display data struct
 * @param palette The pallette to print the text in
 * @param text The text to print to screen
 * @param row The starting row for text
 * @param col The starting col for text
 */
void addText(dispData_t * data, short palette, const char * text, int row, int col);

/**
 * Clears the fame buffer and adds a menu to it
 * 
 * @param data The display data struct
 * @param prompt The menu prompt
 * @param items A list of menu item strings
 * @param nItems The number of items in the menu list
 * @param selected The menu item selected for highlight (<0 to disable)
 */
void addMenu(dispData_t * data, const char * prompt, const char ** items, int nItems, int selected);

/**
 * Clears the screen and prints out the data stored in the buffer
 * 
 * @param data The display data struct
 */
void printBuffer(dispData_t data);

/**
 * Clears out any data already in the buffer
 * 
 * @param data The display data struct
 */
void clearBuffer(dispData_t * data);

//=================================<Misc IO>==================================//
/**
 * Prints the provided text to terminal
 * 
 * @param palette The pallette to print the text in
 * @param text The text to print to screen
 * @param row The starting row for text
 * @param col The starting col for text
 */
void printText(short palette, const char * text, int row, int col);

/**
 * Gets text from the terminal
 * 
 * @param row The starting row for the cursor
 * @param col The starting col for the cursor
 * @param buf The string return buffer
 * @param nBuf The length of the return buffer
 */
void getText(int row, int col, char* buf, unsigned int nBuf);

#endif