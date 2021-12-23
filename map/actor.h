#ifndef _ACTOR_H_
#define _ACTOR_H_

#include <stdlib.h>
#include <string.h>

#include "sprite.h"

typedef struct actor_s {
    // Strings
    char * name;    // The name of the character
    char * status;  // A status message for the character

    // HP Tracking
    int maxHP;  // if <0, unknown
    int damage; // The ammount of damage taken so far

    // Display Information
    int spriteIdx;      // This actor's sprite index (<0 for default)
    int x;              // This actor's X position on map
    int y;              // This actor's Y position on map
} actor_t;

//============================<Memory Management>=============================//
/**
 * Allocates data for a new actor
 * 
 * @param actor A return pointer for the allocated actor
 * @param name To be copied into this actor
 * @param status To be copied into this actor
 * @param maxHP The max HP to set (<0 if unknown)
 * @param damage The differency between this actor's max and current HP (negative for damage, positive for temp HP)
 * @param sprite The sprite to use for this actor (<0 for a default sprite)
 * 
 * @return A pointer to an actor on the heap (NULL on failure)
 */
int mkActor(actor_t * actor, const char * name, const char * status, int maxHP, int damage, int sprite);

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

#endif