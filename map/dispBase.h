#ifndef _DISPBASE_H_
#define _DISPBASE_H_

#include <stdlib.h>
#include <stdio.h>
#include <curses.h>

#include "sprite.h"

// Define common palette numbers
#define kBlackPalette 1
#define kWhitePalette 2
#define kRedPalette 3
#define kGreenPalette 4
#define kBluePalette 5
#define kYellowPalette 6
#define kMagentaPalette 7
#define kCyanPalette 8

#define kDefPalette kBlackPalette

// Define a persistent data structure
typedef struct dispData_s {
    int screenRows;
    int screenCols;
} dispData_t;

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
int closeDisp();

/**
 * Draws the specified sprite on screen
 * 
 * @param data The display data struct
 * @param sprite The sprite to draw
 * @param screenRow The top row to draw in
 * @param screenCol The left column to draw in
 */
void drawSprite(dispData_t data, sprite_t sprite, int screenRow, int screenCol);

/**
 * Prints the provided text to terminal
 * 
 * @param palette The pallette to print the text in
 * @param text The text to print to screen
 * @param row The starting row for text
 * @param col The starting col for text
 */
void printText(short palette, const char * text, int row, int col);

#endif