#include "dispChar.h"

static const char * statStrings[] = {
    "  Strength  ",
    " Dexterity  ",
    "Constitution",
    "Inteligence ",
    "   Wisdom   ",
    "  Charisma  "
};

static const char * profStrings[] = {
    "Strength",
    "Dexterity",
    "Constitution",
    "Inteligence",
    "Wisdom",
    "Charisma",

    "Acrobatics",
    "Animal Handling",
    "Arcana",
    "Athletics",
    "Deception",
    "History",
    "Insight",
    "Intimidate",
    "Investigation",
    "Medicine",
    "Nature",
    "Perception",
    "Performance",
    "Persuasion",
    "Religion",
    "Sleight of Hand",
    "Stealth",
    "Survival"
};

#define min(a, b) ((b < a) ? b : a)
#define max(a, b) ((b > a) ? b : a)

//==================================<Stats>===================================//
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

void addStat(dispData_t * dispData, charData_t charData, int row, int col, 
                int idx, int sel) {
    char buf[7];

    short palette = (idx == sel) ? kWhitePalette : kBlackPalette;
    int statVal = getStat(charData, idx);
    int modVal = getMod(charData, idx);

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
    for(int i = 0; i < kNStats; i++) {
        addStat(dispData, charData, y, x, i, sel);

        if(doVert) y += 8;
        else x += 14;
    }
}

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
                int sel) {
    if(dispData == NULL) return;

    // Calculate the min and max visible parameters
    int mini = max(sel - (dispData->screenRows - row) / 2, row);

    for(int i = mini; row + i < dispData->screenRows && mini + i < kNProfs; ++i) {
        short palette = (getProfIdx(charData, mini + i)) ? kWhitePalette : kBlackPalette;
        if(mini + i == sel) {
            palette = (palette == kWhitePalette) ? kGreenPalette : kRedPalette;
        }

        addText(dispData, palette, profStrings[mini + i], row + i, col);
    }

}