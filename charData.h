#ifndef _CHAR_DATA_H_
#define _CHAR_DATA_H_

#include <stdlib.h>
#include <stdio.h>

typedef enum alignment_e {
    LawfulGood,    NeutralGood, ChaoticGood,
    LawfulNeutral, Neutral,     ChaoticNeutral,
    LawfulEvil,    NeutralEvil, ChaoticEvil
} alignment_t;

typedef struct character_s {
    // Basic Biographical info
    char * name;
    char * playerName;

    // Core progression info
    char * class;
    char * race;
    char * background;

    // Stat tracking
    unsigned int inspiration;
    unsigned int exp;
    unsigned char abilityScores[6];
    
    // HP tracking
    unsigned int maxHp;
    unsigned int curHp;
    unsigned int tempHp;
    unsigned int hitDice;
    unsigned int hitDieType;

    // Saving Throw Proficiencies
    unsigned char PStr: 1;
    unsigned char PDex: 1;
    unsigned char PCon: 1;
    unsigned char PInt: 1;
    unsigned char PWis: 1;
    unsigned char PCha: 1;

    // Skill Proficiencies
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

    // RP info
    alignment_t alignment;
    char * inventory;
    char * spells;
    char * info;

} character_t;

#endif