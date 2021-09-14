#include "dispChar.h"

static const char * statStrings[] = {
    "  Strength  ",
    " Dexterity  ",
    "Constitution",
    "Inteligence ",
    "   Wisdom   ",
    "  Charisma  "
};

#define kStatStringsSize sizeof(statStrings)/sizeof(statStrings[0])

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
    "Survival"
};

#define kProfStringsSize sizeof(profStrings)/sizeof(profStrings[0])

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
}

/**
 * Sets a proficiency from the standard order
 * 
 * @param charData The character data to modify
 * @param idx The index of the proficiency to toggle
 * @param val The value to set
 */
void setProfIdx(charData_t * charData, int idx, bool val) {
    if(charData == NULL) return;
    val = (val == 0) ? 0 : -1; // Full bit field value

}

/**
 * Returns the status of a proficiency from a standard order index
 * 
 * @param charData The character to query
 * @param idx The index of the proficiency to query
 * 
 * @param The status of the selected proficiency (true iff proficient)
 */
bool getProfIdx(charData_t charData, int idx) {
    switch(idx) {
        case 0:
            return charData.proStr;
        case 1:
            return charData.proDex;
        case 2:
            return charData.proCon;
        case 3:
            return charData.proInt;
        case 4:
            return charData.proWis;
        case 5:
            return charData.proCha;

        case 6:
            return charData.proAcro;
        case 7:
            return charData.proAnim;
        case 8:
            return charData.proArca;
        case 9:
            return charData.proAthl;
        case 10:
            return charData.proDece;
        case 11:
            return charData.proHist;
        case 12:
            return charData.proInsi;
        case 13:
            return charData.proInti;
        case 14:
            return charData.proInve;
        case 15:
            return charData.proMedi;
        case 16:
            return charData.proNatu;
        case 17:
            return charData.proPerc;
        case 18:
            return charData.proPerf;
        case 19:
            return charData.proPers;
        case 20:
            return charData.proReli;
        case 21:
            return charData.proSlig;
        case 22:
            return charData.proStea;
        case 23:
            return charData.proSurv;
    }

    return false;
}
