#ifndef _DISP_CHAR_H_
#define _DISP_CHAR_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "charData.h"
#include "../common/dispBase.h"

//==================================<Stats>===================================//
/**
 * Adds the stat block to the screen buffer
 * 
 * @param dispData The display data struct
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
 * @param dispData The display data struct
 * @param charData The character data to display from
 * @param row The top row to display them in
 * @param col The left column to display them in
 * @param doVert Arrange stats vertically iff this is true
 * @param sel The index of the element to select (order as in the struct) 
 *            (<0 to disable)
 */
void addStatSel(dispData_t * dispData, charData_t charData, int row, int col, 
                    bool doVert, int sel);


//==============================<Proficiencies>===============================//
/**
 * Adds a proficiency selection screen to the frame buffer
 * 
 * @param dispData The display data struct
 * @param charData The character data to display from
 * @param row The top row to display them in
 * @param col The left column to display them in
 * @param sel The index of the element to select (order as in the struct) 
 *            (<0 to disable)
 */
void addProfSel(dispData_t * dispData, charData_t charData, int row, int col,
                int sel);

/**
 * Sets a proficiency from the standard order
 * 
 * @param charData The character data to modify
 * @param idx The index of the proficiency to toggle
 * @param val The value to set
 */
void setProfIdx(charData_t * charData, int idx, bool val);

/**
 * Returns the status of a proficiency from a standard order index
 * 
 * @param charData The character to query
 * @param idx The index of the proficiency to query
 * 
 * @param The status of the selected proficiency (true iff proficient)
 */
bool getProfIdx(charData_t charData, int idx);

#endif