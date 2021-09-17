#include "charData.h"


const int kNStats = 6;
const int kNProfs = 24;
const int kNDice = 6;

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

    // Save the other stats next
    fprintf(fp, "%d %d %d|\n", charData.level, charData.profBonus, charData.skillBonus);
    fprintf(fp, "%d %d %d|\n", charData.maxHP, charData.curHP, charData.tmpHP);

    // Save out the hit dice info
    for(int i = 0; i < kNDice; i++) {
        fprintf(fp, "%d ", charData.maxHitDice[i]);
    }
    fprintf(fp, "\n");
    for(int i = 0; i < kNDice; i++) {
        fprintf(fp, "%d ", charData.curHitDice[i]);
    }
    fprintf(fp, "\n");

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

    
    // recover the other stats next
    if(fscanf(fp, "%d %d %d|\n", &charData->level, &charData->profBonus, 
            &charData->skillBonus) != 3) {
        rmCharData(*charData);
        return EXIT_FAILURE;
    }
    if(fscanf(fp, "%d %d %d|\n", &charData->maxHP, &charData->curHP, &charData->tmpHP) != 3) {
        rmCharData(*charData);
        return EXIT_FAILURE;
    }

    // Recover hit dice next
    for(int i = 0; i < kNDice; i++) {
        fscanf(fp, "%d ", &charData->maxHitDice[i]);
    }
    for(int i = 0; i < kNDice; i++) {
        fscanf(fp, "%d ", &charData->curHitDice[i]);
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
int getStat(charData_t data, int idx) {
    switch(idx) {
        case kStrIdx:
            return data.Str;
            break;
        case kDexIdx:
            return data.Dex;
            break;
        case kConIdx:
            return data.Con;
            break;
        case kIntIdx:
            return data.Int;
            break;
        case kWisIdx:
            return data.Wis;
            break;
        case kChaIdx:
            return data.Cha;
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
        case kStrIdx:
            data->Str = val;
            break;
        case kDexIdx:
            data->Dex = val;
            break;
        case kConIdx:
            data->Con = val;
            break;
        case kIntIdx:
            data->Int = val;
            break;
        case kWisIdx:
            data->Wis = val;
            break;
        case kChaIdx:
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
        case kStrIdx:
            data->Str += delta;
            break;
        case kDexIdx:
            data->Dex += delta;
            break;
        case kConIdx:
            data->Con += delta;
            break;
        case kIntIdx:
            data->Int += delta;
            break;
        case kWisIdx:
            data->Wis += delta;
            break;
        case kChaIdx:
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
        case kStrIdx:
            charData->proStr = val;
            break;
        case kDexIdx:
            charData->proDex = val;
            break;
        case kConIdx:
            charData->proCon = val;
            break;
        case kIntIdx:
            charData->proInt = val;
            break;
        case kWisIdx:
            charData->proWis = val;
            break;
        case kChaIdx:
            charData->proCha = val;
            break;

        case kAcroIdx:
            charData->proAcro = val;
            break;
        case kAnimIdx:
            charData->proAnim = val;
            break;
        case kArcaIdx:
            charData->proArca = val;
            break;
        case kAthlIdx:
            charData->proAthl = val;
            break;
        case kDeceIdx:
            charData->proDece = val;
            break;
        case kHistIdx:
            charData->proHist = val;
            break;
        case kInsiIdx:
            charData->proInsi = val;
            break;
        case kIntiIdx:
            charData->proInti = val;
            break;
        case kInveIdx:
            charData->proInve = val;
            break;
        case kMediIdx:
            charData->proMedi = val;
            break;
        case kNatuIdx:
            charData->proNatu = val;
            break;
        case kPercIdx:
            charData->proPerc = val;
            break;
        case kPerfIdx:
            charData->proPerf = val;
            break;
        case kPersIdx:
            charData->proPers = val;
            break;
        case kReliIdx:
            charData->proReli = val;
            break;
        case kSligIdx:
            charData->proSlig = val;
            break;
        case kSteaIdx:
            charData->proStea = val;
            break;
        case kSurvIdx:
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
        case kStrIdx:
            return charData.proStr;
        case kDexIdx:
            return charData.proDex;
        case kConIdx:
            return charData.proCon;
        case kIntIdx:
            return charData.proInt;
        case kWisIdx:
            return charData.proWis;
        case kChaIdx:
            return charData.proCha;

        case kAcroIdx:
            return charData.proAcro;
        case kAnimIdx:
            return charData.proAnim;
        case kArcaIdx:
            return charData.proArca;
        case kAthlIdx:
            return charData.proAthl;
        case kDeceIdx:
            return charData.proDece;
        case kHistIdx:
            return charData.proHist;
        case kInsiIdx:
            return charData.proInsi;
        case kIntiIdx:
            return charData.proInti;
        case kInveIdx:
            return charData.proInve;
        case kMediIdx:
            return charData.proMedi;
        case kNatuIdx:
            return charData.proNatu;
        case kPercIdx:
            return charData.proPerc;
        case kPerfIdx:
            return charData.proPerf;
        case kPersIdx:
            return charData.proPers;
        case kReliIdx:
            return charData.proReli;
        case kSligIdx:
            return charData.proSlig;
        case kSteaIdx:
            return charData.proStea;
        case kSurvIdx:
            return charData.proSurv;
    }

    return false;
}

/**
 * Calculates the stat modifier for a given skill check
 * 
 * @param data The character making the check
 * @param skill The relevant proficiency index (<0 for raw check)
 * 
 * @return The modifier of the check
 */
int getMod(charData_t data, int skill) {
    int mod = 0; 

    // First get the base stat mod
    switch(skill) {
        // Strength skills
        case kStrIdx:
        case kAthlIdx:
            mod = data.Str;
            break;
        
        // Dexterity skills
        case kDexIdx:
        case kAcroIdx:
        case kSligIdx:
        case kSteaIdx:
            mod = data.Dex;
            break;
        
        // Constitution Skills
        case kConIdx:
            mod = data.Con;
            break;
        
        // Inteligence Skills
        case kIntIdx:
        case kArcaIdx:
        case kHistIdx:
        case kInveIdx:
        case kNatuIdx:
        case kReliIdx:
            mod = data.Int;
            break;
        
        // Wisdom Skills
        case kWisIdx:
        case kAnimIdx:
        case kInsiIdx:
        case kMediIdx:
        case kPercIdx:
        case kSurvIdx:
            mod = data.Wis;
            break;

        // Charisma Skills
        case kChaIdx:
        case kDeceIdx:
        case kIntiIdx:
        case kPerfIdx:
        case kPersIdx:
            mod = data.Cha;
            break;

        // Raw check
        default:
            mod = 10; // Evens out to no mod
            break;
    }
    mod = (mod/2) - 5;

    // Next add any proficiency or other bonuses
    if(getProfIdx(data, skill)) mod += data.profBonus;
    else if (skill >= kNStats) mod += data.skillBonus;

    return mod;
}
