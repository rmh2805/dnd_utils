#include "charData.h"


const int kNStats = 6;
const int kNProfs = 24;

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




//===========================<Getters and Setters>============================//
/**
 * Gets the value of a stat by index
 * 
 * @param data The character to query
 * @param idx The index of the stat
 * 
 * @return The value of the requested stat
 */
int getStat(charData_t * data, int idx) {
    switch(idx) {
        case 0:
            return data->Str;
            break;
        case 1:
            return data->Dex;
            break;
        case 2:
            return data->Con;
            break;
        case 3:
            return data->Int;
            break;
        case 4:
            return data->Wis;
            break;
        case 5:
            return data->Cha;
            break;
        default:
            return -1;
            break;
    }
}

/**
 * Sets a stat by index
 * 
 * @param data The character to modify
 * @param idx The index of the stat
 * @param val The new value for the stat
 */
void setStat(charData_t * data, int idx, int val) {
    switch(idx) {
        case 0:
            data->Str = val;
            break;
        case 1:
            data->Dex = val;
            break;
        case 2:
            data->Con = val;
            break;
        case 3:
            data->Int = val;
            break;
        case 4:
            data->Wis = val;
            break;
        case 5:
            data->Cha = val;
            break;
        default:
            break;
    }

}

/**
 * Modifies a stat by index
 * 
 * @param data The character to modify
 * @param idx The index of the stat
 * @param delta The amount to modify the stat (added to current value)
 */
void modStat(charData_t * data, int idx, int delta) {
    switch(idx) {
        case 0:
            data->Str += delta;
            break;
        case 1:
            data->Dex += delta;
            break;
        case 2:
            data->Con += delta;
            break;
        case 3:
            data->Int += delta;
            break;
        case 4:
            data->Wis += delta;
            break;
        case 5:
            data->Cha += delta;
            break;
        default:
            break;
    }
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
    if(val) val = -1;
    
    switch(idx) {
        case 0:
            charData->proStr = val;
            break;
        case 1:
            charData->proDex = val;
            break;
        case 2:
            charData->proCon = val;
            break;
        case 3:
            charData->proInt = val;
            break;
        case 4:
            charData->proWis = val;
            break;
        case 5:
            charData->proCha = val;
            break;

        case 6:
            charData->proAcro = val;
            break;
        case 7:
            charData->proAnim = val;
            break;
        case 8:
            charData->proArca = val;
            break;
        case 9:
            charData->proAthl = val;
            break;
        case 10:
            charData->proDece = val;
            break;
        case 11:
            charData->proHist = val;
            break;
        case 12:
            charData->proInsi = val;
            break;
        case 13:
            charData->proInti = val;
            break;
        case 14:
            charData->proInve = val;
            break;
        case 15:
            charData->proMedi = val;
            break;
        case 16:
            charData->proNatu = val;
            break;
        case 17:
            charData->proPerc = val;
            break;
        case 18:
            charData->proPerf = val;
            break;
        case 19:
            charData->proPers = val;
            break;
        case 20:
            charData->proReli = val;
            break;
        case 21:
            charData->proSlig = val;
            break;
        case 22:
            charData->proStea = val;
            break;
        case 23:
            charData->proSurv = val;
            break;
    }
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
