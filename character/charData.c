#include "charData.h"

//===============================<Alloc & Free>===============================//
/**
 * Returns a zeroed-out character struct
 * 
 * @return A zeroed-out character struct
 */
charData_t mkCharData() {
    charData_t data;
    for(size_t i = 0; i < sizeof(data); i++) {
        ((char *) &data)[i] = 0;
    }
    return data;
}

/**
 * Removes all data allocated to the data struct
 * 
 * @param charData The character to free
 */
void rmCharData(charData_t charData) {
    if(charData.name != NULL) free(charData.name);
    if(charData.playerName != NULL) free(charData.playerName);

    if(charData.baseClass != NULL) free(charData.baseClass);
    if(charData.background != NULL) free(charData.background);
    if(charData.race != NULL) free(charData.race);
}


#define saveCharDataWriteString(fp, data, field) \
    fprintf(fp, "%lu |%s\n", (size_t) (data.field == NULL) ? 0 : \
        strlen(data.field), (data.field == NULL) ? "" : data.field)

#define saveCharDataMergeBlock(block, data, field, width) \
    block = (block << width) | data.field

/**
 * Writes the provided charData file out to file
 * 
 * @param fp The file to write out to
 * @param charData The character to write to file
 * 
 * @return 0 on success, < 0 on failure
 */
int saveCharData(FILE * fp, charData_t charData) {
    if(fp == NULL) return -1;

    // Save the single strings first
    saveCharDataWriteString(fp, charData, name);
    saveCharDataWriteString(fp, charData, playerName);
    saveCharDataWriteString(fp, charData, baseClass);
    saveCharDataWriteString(fp, charData, background);
    saveCharDataWriteString(fp, charData, race);

    // Generate the base stat block;
    uint32_t statBlock = 0;
    saveCharDataMergeBlock(statBlock, charData, Str, 5);
    saveCharDataMergeBlock(statBlock, charData, Dex, 5);
    saveCharDataMergeBlock(statBlock, charData, Con, 5);
    saveCharDataMergeBlock(statBlock, charData, Int, 5);
    saveCharDataMergeBlock(statBlock, charData, Wis, 5);
    saveCharDataMergeBlock(statBlock, charData, Cha, 5);

    // Generate the proficiencies block
    uint32_t profBlock = 0;
    saveCharDataMergeBlock(profBlock, charData, proStr, 1);
    saveCharDataMergeBlock(profBlock, charData, proDex, 1);
    saveCharDataMergeBlock(profBlock, charData, proCon, 1);
    saveCharDataMergeBlock(profBlock, charData, proInt, 1);
    saveCharDataMergeBlock(profBlock, charData, proWis, 1);
    saveCharDataMergeBlock(profBlock, charData, proCha, 1);

    saveCharDataMergeBlock(profBlock, charData, proAcro, 1);
    saveCharDataMergeBlock(profBlock, charData, proAnim, 1);
    saveCharDataMergeBlock(profBlock, charData, proArca, 1);
    saveCharDataMergeBlock(profBlock, charData, proAthl, 1);
    saveCharDataMergeBlock(profBlock, charData, proDece, 1);
    saveCharDataMergeBlock(profBlock, charData, proHist, 1);
    saveCharDataMergeBlock(profBlock, charData, proInsi, 1);
    saveCharDataMergeBlock(profBlock, charData, proInti, 1);
    saveCharDataMergeBlock(profBlock, charData, proInve, 1);
    saveCharDataMergeBlock(profBlock, charData, proMedi, 1);
    saveCharDataMergeBlock(profBlock, charData, proNatu, 1);
    saveCharDataMergeBlock(profBlock, charData, proPerc, 1);
    saveCharDataMergeBlock(profBlock, charData, proPerf, 1);
    saveCharDataMergeBlock(profBlock, charData, proPers, 1);
    saveCharDataMergeBlock(profBlock, charData, proReli, 1);
    saveCharDataMergeBlock(profBlock, charData, proSlig, 1);
    saveCharDataMergeBlock(profBlock, charData, proStea, 1);
    saveCharDataMergeBlock(profBlock, charData, proSurv, 1);

    fprintf(fp, "%u %u\n", statBlock, profBlock);
    return 0;
}

#define loadCharDataBreakBlock(block, data, field, width, mask) \
    data->field = (block & mask); \
    block = block >> width

int loadCharDataString(FILE * fp, char** str) {
    int ret = 0;
    size_t bufSize = 0;

    ret = fscanf(fp, "%lu |", &bufSize);
    if(ret != 1) {
        return -1;
    }

    *str = calloc(bufSize + 1, sizeof(char));
    if(*str == NULL) {
        return -1;
    }

    fread(*str, 1, bufSize, fp);

    return 0;
}

/**
 * Reads a provided character data struct from file
 * 
 * @param fp The file to read from
 * @param charData A return pointer to read into
 * 
 * @return 0 on success, < 0 on failure
 */
int loadCharData(FILE * fp, charData_t * charData) {
    if(fp == NULL || charData == NULL) return -1;

    // Null-out strings
    charData->name = NULL;
    charData->playerName = NULL;
    charData->baseClass = NULL;
    charData->background = NULL;
    charData->race = NULL;

    // Load each of the individual strings
    if(loadCharDataString(fp, &charData->name) != 0) {
        rmCharData(*charData);
        return -1;
    }
    if(loadCharDataString(fp, &charData->playerName) != 0) {
        rmCharData(*charData);
        return -1;
    }
    if(loadCharDataString(fp, &charData->baseClass) != 0) {
        rmCharData(*charData);
        return -1;
    }
    if(loadCharDataString(fp, &charData->background) != 0) {
        rmCharData(*charData);
        return -1;
    }
    if(loadCharDataString(fp, &charData->race) != 0) {
        rmCharData(*charData);
        return -1;
    }

    // Load the field blocks
    uint32_t statBlock = 0, profBlock = 0;
    if(fscanf(fp, "%u %u", &statBlock, &profBlock) != 2) {
        rmCharData(*charData);
        return EXIT_FAILURE;
    }

    loadCharDataBreakBlock(statBlock, charData, Cha, 5, 0x1F);
    loadCharDataBreakBlock(statBlock, charData, Wis, 5, 0x1F);
    loadCharDataBreakBlock(statBlock, charData, Int, 5, 0x1F);
    loadCharDataBreakBlock(statBlock, charData, Con, 5, 0x1F);
    loadCharDataBreakBlock(statBlock, charData, Dex, 5, 0x1F);
    loadCharDataBreakBlock(statBlock, charData, Str, 5, 0x1F);

    loadCharDataBreakBlock(profBlock, charData, proSurv, 1, 0x01);
    loadCharDataBreakBlock(profBlock, charData, proStea, 1, 0x01);
    loadCharDataBreakBlock(profBlock, charData, proSlig, 1, 0x01);
    loadCharDataBreakBlock(profBlock, charData, proReli, 1, 0x01);
    loadCharDataBreakBlock(profBlock, charData, proPers, 1, 0x01);
    loadCharDataBreakBlock(profBlock, charData, proPerf, 1, 0x01);
    loadCharDataBreakBlock(profBlock, charData, proPerc, 1, 0x01);
    loadCharDataBreakBlock(profBlock, charData, proNatu, 1, 0x01);
    loadCharDataBreakBlock(profBlock, charData, proMedi, 1, 0x01);
    loadCharDataBreakBlock(profBlock, charData, proInve, 1, 0x01);
    loadCharDataBreakBlock(profBlock, charData, proInti, 1, 0x01);
    loadCharDataBreakBlock(profBlock, charData, proInsi, 1, 0x01);
    loadCharDataBreakBlock(profBlock, charData, proHist, 1, 0x01);
    loadCharDataBreakBlock(profBlock, charData, proDece, 1, 0x01);
    loadCharDataBreakBlock(profBlock, charData, proAthl, 1, 0x01);
    loadCharDataBreakBlock(profBlock, charData, proArca, 1, 0x01);
    loadCharDataBreakBlock(profBlock, charData, proAnim, 1, 0x01);
    loadCharDataBreakBlock(profBlock, charData, proAcro, 1, 0x01);
    
    loadCharDataBreakBlock(profBlock, charData, proCha, 1, 0x01);
    loadCharDataBreakBlock(profBlock, charData, proWis, 1, 0x01);
    loadCharDataBreakBlock(profBlock, charData, proInt, 1, 0x01);
    loadCharDataBreakBlock(profBlock, charData, proCon, 1, 0x01);
    loadCharDataBreakBlock(profBlock, charData, proDex, 1, 0x01);
    loadCharDataBreakBlock(profBlock, charData, proStr, 1, 0x01);

    return 0;
}