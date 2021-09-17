#ifndef _CHAR_DATA_H_
#define _CHAR_DATA_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

//================================<Constants>=================================//
extern const int kNStats;
extern const int kNProfs;

// Skill indeces
#define kStrIdx 0
#define kDexIdx 1
#define kConIdx 2
#define kIntIdx 3
#define kWisIdx 4
#define kChaIdx 5

#define kAcroIdx 6
#define kAnimIdx 7
#define kArcaIdx 8
#define kAthlIdx 9
#define kDeceIdx 10
#define kHistIdx 11
#define kInsiIdx 12
#define kIntiIdx 13
#define kInveIdx 14
#define kMediIdx 15
#define kNatuIdx 16
#define kPercIdx 17
#define kPerfIdx 18
#define kPersIdx 19
#define kReliIdx 20
#define kSligIdx 21
#define kSteaIdx 22
#define kSurvIdx 23

//=================================<Typedefs>=================================//
typedef struct charData_s {
    // Basic Biographical Info
    char * name;
    char * playerName;

    char * baseClass;
    char * background;
    char * race;

    // Base Stats
    uint32_t Str : 5;
    uint32_t Dex : 5;
    uint32_t Con : 5;
    uint32_t Int : 5;
    uint32_t Wis : 5;
    uint32_t Cha : 5;

    // Proficiencies
    uint32_t proStr : 1;
    uint32_t proDex : 1;
    uint32_t proCon : 1;
    uint32_t proInt : 1;
    uint32_t proWis : 1;
    uint32_t proCha : 1;

    uint32_t proAcro : 1;
    uint32_t proAnim : 1;
    uint32_t proArca : 1;
    uint32_t proAthl : 1;
    uint32_t proDece : 1;
    uint32_t proHist : 1;
    uint32_t proInsi : 1;
    uint32_t proInti : 1;
    uint32_t proInve : 1;
    uint32_t proMedi : 1;
    uint32_t proNatu : 1;
    uint32_t proPerc : 1;
    uint32_t proPerf : 1;
    uint32_t proPers : 1;
    uint32_t proReli : 1;
    uint32_t proSlig : 1;
    uint32_t proStea : 1;
    uint32_t proSurv : 1;
    
} charData_t;


//===============================<Alloc & Free>===============================//
/**
 * Returns a zeroed-out character struct
 * 
 * @return A zeroed-out character struct
 */
charData_t mkCharData();

/**
 * Removes all data allocated to the data struct
 * 
 * @param charData The character to free
 */
void rmCharData(charData_t charData);

/**
 * Writes the provided charData file out to file
 * 
 * @param fp The file to write out to
 * @param charData The character to write to file
 * 
 * @return 0 on success, < 0 on failure
 */
int saveCharData(FILE * fp, charData_t charData);

/**
 * Reads a provided character data struct from file
 * 
 * @param fp The file to read from
 * @param charData A return pointer to read into
 * 
 * @return 0 on success, < 0 on failure
 */
int loadCharData(FILE * fp, charData_t * charData);

//===========================<Getters and Setters>============================//
/**
 * Gets the value of a stat by index
 * 
 * @param data The character to query
 * @param idx The index of the stat
 * 
 * @return The value of the requested stat
 */
int getStat(charData_t data, int idx);

/**
 * Sets a stat by index
 * 
 * @param data The character to modify
 * @param idx The index of the stat
 * @param val The new value for the stat
 */
void setStat(charData_t * data, int idx, int val);

/**
 * Modifies a stat by index
 * 
 * @param data The character to modify
 * @param idx The index of the stat
 * @param delta The amount to modify the stat (added to current value)
 */
void modStat(charData_t * data, int idx, int delta);

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

/**
 * Calculates the stat modifier for a given skill check
 * 
 * @param data The character making the check
 * @param skill The relevant proficiency index (<0 for raw check)
 * 
 * @return The modifier of the check
 */
int getMod(charData_t data, int skill);

#endif