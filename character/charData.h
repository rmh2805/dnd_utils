#ifndef _CHAR_DATA_H_
#define _CHAR_DATA_H_

#include <stdlib.h>
#include <stdio.h>

#include "../common/fs.h"

typedef enum alignment_e {
    LawfulGood,    NeutralGood, ChaoticGood,
    LawfulNeutral, Neutral,     ChaoticNeutral,
    LawfulEvil,    NeutralEvil, ChaoticEvil
} alignment_t;

typedef struct character_s {
    char * savePath;                // The path of this character's save dir
    
    // Basic info
    char * name;                    // This character's name
    char * playerName;              // This character's player's name

    // Core progression info
    char * race;                    // Character race
    char * background;              // Character background

    // Stat tracking
    unsigned int inspiration;       // Tracks inspiration
    unsigned int exp;               // Tracks current experience
    unsigned int armorClass;        // Tracks current armor class
    unsigned int speed;             // Tracks current speed
    
    // HP tracking
    unsigned int maxHp;             // Max HP
    unsigned int curHp;             // Current HP
    unsigned int tempHp;            // Temporary HP

    // Misc tracking
    unsigned int moneyTrack[5];     // Order: CP, SP, EP, GP, PP
    unsigned int passiveWis;        // Tracks passive wisdom
    int          initiativeMod;     // Tracks initiative modifier

    // Tracking for stat definition
    unsigned char abilityScores[6]; // Tracks ability scores
                                    // Order: Str, Dex, Con, Int, Wis, Cha
    unsigned int profBonus;         // Tracks proficiency bonus

    // Hit dice tracking (order: d4, d6, d8, d10, d12, d20)
    unsigned int maxHitDice[6];     // Max number of hit dice of each type
    unsigned int usedHitDice[6];    // Nr hit dice used of each type

    // Saving Throw Proficiencies (boolean fields)
    unsigned char PStr: 1;
    unsigned char PDex: 1;
    unsigned char PCon: 1;
    unsigned char PInt: 1;
    unsigned char PWis: 1;
    unsigned char PCha: 1;

    // Skill Proficiencies (boolean fields)
    unsigned char PAcrobatics:1;
    unsigned char PAnimalHandling:1;
    unsigned char PArcana:1;
    unsigned char PAthletics:1;
    unsigned char PDeception:1;
    unsigned char PHistory:1;
    unsigned char PInsight:1;
    unsigned char PIntimidation:1;
    unsigned char PInvestigation:1;
    unsigned char PMedicine:1;
    unsigned char PNature:1;
    unsigned char PPerception:1;
    unsigned char PReligion:1;
    unsigned char PSleightOfHand:1;
    unsigned char PStealth:1;
    unsigned char PSurvival:1;

} character_t;

/**
 * Generates a null-initialized character
 * 
 * @return A null-initialized character struct
 */
character_t mkBlankCharacter();


/**
 * Free all allocated data associated with the character
 * 
 * @param character The character to free from
 * @return An exit status (0 on success, <0 on failure)
 */
int freeCharacterData(character_t character);

/**
 * Loads a character from file
 * 
 * @param savePath The path of the character's save directory
 * @param character A return pointer for the character struct
 * @return An exit status (0 on success, <0 on failure)
 */
int loadCharacter(const char * savePath, character_t * character);


/**
 * Writes the provided character's information out to file
 * 
 * @param character The character to write to file
 * @return An exit status (0 on success, <0 on failure)
 */
int writeCharacter(character_t character);


#endif