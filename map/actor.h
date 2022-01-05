#ifndef _ACTOR_H_
#define _ACTOR_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../common/list.h"

#include "sprite.h"
#include "wallSprites.h"

typedef struct actor_s {
    // Strings
    char * name;    // The name of the character
    char * status;  // A status message for the character

    // HP Tracking
    int maxHP;      // If <0, unknown
    int deltaHP;    // The current change from max HP so far

    // Display Information
    int spriteIdx;      // This actor's sprite index (<0 for default)
    int x;              // This actor's X position on map
    int y;              // This actor's Y position on map

    short palette;          // The base palette to use for this actor
    short paletteOverride;  // An override palette to use for this actor
} actor_t;

typedef struct actorData_s {
    list_t pcActors;
    list_t npcActors;
    list_t enemyActors;

    list_t actorSprites;

    sprite_t defActorSprite;
} actorData_t;

//============================<Memory Management>=============================//

/**
 * Allocates and initializes an empty actorData struct
 * 
 * @param data A return pointer for the actor data struct
 * 
 * @return 0 on success, <0 on failure
 */
int loadActorData(actorData_t* data);


/**
 * Frees all data allocated for an actor data struct
 * 
 * @param data The actor data struct to free
 */
void rmActorData(actorData_t data);

/**
 * Allocates data for a new actor
 * 
 * @param actor A return pointer for the allocated actor
 * @param name To be copied into this actor
 * @param status To be copied into this actor
 * @param maxHP The max HP to set (<0 if unknown)
 * @param deltaHP The differency between this actor's max and current HP (negative for damage, positive for temp HP)
 * @param sprite The sprite to use for this actor (<0 for a default sprite)
 * 
 * @return 0 on success, <0 on failure
 */
int mkActor(actor_t * actor, const char * name, const char * status, int maxHP, int deltaHP, int sprite);

/**
 * Frees an actor and its allocated data (does not directly free its sprite)
 * 
 * @param actor The actor to free
 */
void rmActor(actor_t actor);

/**
 * Moves an actor onto the heap
 * 
 * @param actor The actor to move onto the heap
 * @return A pointer to the actor on the heap
 */
actor_t* mkActorEntry(actor_t actor);

/**
 * Frees an actor from the heap
 * 
 * @param data A pointer to an actor on the heap
 */
void freeActorEntry(void * data);

//===============================<File Access>================================//

/**
 * Writes an actor out to file
 * 
 * @param actor The actor to write to file
 * @param fp The file to write the actor to
 * 
 * @return 0 on success, <0 on failure
 */
int writeActor(actor_t actor, FILE* fp);

/**
 * Reads an actor from file
 * 
 * @param actor A pointer to the actor to read in to
 * @param fp The file to read the actor from
 * 
 * @return 0 on success, <0 on failure
 */
int readActor(actor_t* actor, FILE* fp);

/**
 * Writes the referand of an actor pointer out to file
 * 
 * @param entry A pointer to the actor to write out
 * @param fp The file to write the actor to
 * 
 * @return 0 on success, <0 on failure
 */
int writeActorEntry(actor_t* entry, FILE* fp);


/**
 * Reads an actor from file into the referenced actor
 * 
 * @param entry A double pointer to read in to
 * @param fp The file to read the actor from
 * 
 * @return 0 on success, <0 on failure
 */
int readActorEntry(actor_t** entry, FILE* fp);

#endif