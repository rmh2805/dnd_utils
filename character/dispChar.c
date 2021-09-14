#include "dispChar.h"

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
                bool doVert) {
    addStatSel(dispData, charData, row, col, doVert, -1);
}

static const char * statStrings[] = {
    "  Strength  ",
    " Dexterity  ",
    "Constitution",
    "Inteligence ",
    "   Wisdom   ",
    "  Charisma  "
};

void addStat(dispData_t * dispData, charData_t charData, int row, int col, 
                int idx, int sel) {
    char buf[7];

    short palette = (idx == sel) ? kWhitePalette : kBlackPalette;
    int statVal;
    switch(idx) {
        case 0:
            statVal = charData.Str;
            break;
        case 1:
            statVal = charData.Dex;
            break;
        case 2:
            statVal = charData.Con;
            break;
        case 3:
            statVal = charData.Int;
            break;
        case 4:
            statVal = charData.Wis;
            break;
        case 5:
            statVal = charData.Cha;
            break;
        default:
            return;
    }
    int modVal = ((int) statVal - 10) / 2;

    addText(dispData, palette, statStrings[idx], row, col);
    addText(dispData, palette, "+----+", row + 1, col + 3);
    addText(dispData, palette, "|    |", row + 2, col + 3);
    sprintf(buf, "| %2u |", statVal);
    addText(dispData, palette, buf, row + 3, col + 3);
    addText(dispData, palette, "|    |", row + 4, col + 3);
    addText(dispData, palette, "+----+", row + 5, col + 3);
    sprintf(buf, "%+2d", modVal);
    addText(dispData, palette, buf, row + 6, col + 5);

}

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
                    bool doVert, int sel) {
    
    int y = row, x = col;
    for(int i = 0; i < 6; i++) {
        addStat(dispData, charData, y, x, i, sel);

        if(doVert) y += 8;
        else x += 14;
    }
}