#ifndef _DISP_CHAR_H_
#define _DISP_CHAR_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "charData.h"
#include "../common/dispBase.h"

typedef enum direction_e {
    up, down, left, right
} direction_t;

/**
 * Adds the stat block to the screen buffer
 * 
 * @param dispData The display data buffer
 * @param charData The character data to display from
 * @param row The top row to display them in
 * @param col The left column to display them in
 * @param doVert Arrange stats vertically iff this is true
 */
void addStats(dispData_t * dispData, charData_t charData, int row, int col, 
                bool doVert);

/**
 * Adds the stat block to the screen buffer with one element selected
 * 
 * @param dispData The display data buffer
 * @param charData The character data to display from
 * @param row The top row to display them in
 * @param col The left column to display them in
 * @param doVert Arrange stats vertically iff this is true
 * @param sel The index of the element to select (order as in the struct) 
 *            (<0 to disable)
 */
void addStatSel(dispData_t * dispData, charData_t charData, int row, int col, 
                    bool doVert, int sel);

#endif