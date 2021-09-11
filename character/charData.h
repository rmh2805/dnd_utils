#ifndef _CHAR_DATA_H_
#define _CHAR_DATA_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

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

#endif