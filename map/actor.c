#include "actor.h"

//=============================<Helper Functions>=============================//

/**
 * Allocates and copies from name to dst
 * 
 * @param dst A pointer to the destination string pointer (e.g. actor->name)
 * @param name The source string pointer (e.g. mkActor's name param)
 * 
 * @return 0 on success, <0 on failure
 */
int cpyName(char** dst, const char * name) {
    // Ensure that a destination is specified
    if(dst == NULL) {
        return -1;
    }

    // If the name was NULL, just copy that across
    if(name == NULL) {
        *dst = NULL;
        return 0;
    }

    // Otherwise, allocate a new destination string
    *dst = calloc(strlen(name) + 1, sizeof(char));
    if(*dst == NULL) {
        return -1;
    }

    // Perform the memory copy
    memcpy(*dst, name, strlen(name));
    return 0;
}

//============================<Memory Management>=============================//
int mkActor(actor_t * actor, const char * name, const char * status, int maxHP, int damage, int spriteIdx) {
    // Ensure that the actor pointer is not null
    if(actor == NULL) {
        return -1;
    }

    // Zero-fill the actor struct
    for(size_t i = 0; i < sizeof(*actor); ++i) {
        ((char *) actor)[i] = '\0';
    }

    // Copy the name and status into the actor
    if(cpyName(&actor->name, name) < 0) {
        rmActor(*actor);
        return -1;
    }
    if(cpyName(&actor->status, status) < 0) {
        rmActor(*actor);
        return -1;
    }

    // Set the actor's sprite
    actor->spriteIdx = spriteIdx;

    // Set the actor's max HP and damage
    actor->maxHP = maxHP;
    actor->damage = damage;

    return 0;
}

void rmActor(actor_t actor) {
    if(actor.name != NULL) {
        free(actor.name);
    }
    if(actor.status != NULL) {
        free(actor.status);
    }
}

actor_t* mkActorEntry(actor_t actor) {
    // Allocate the new entry
    actor_t* entry = calloc(1, sizeof(actor_t));
    if(entry == NULL) {
        return NULL;
    }

    // Copy the actor into the entry and return
    *entry = actor;
    return entry;
}

void freeActorEntry(void * data) {
    if(data == NULL) {
        return;
    }

    rmActor(*(actor_t *)data);
    free(data);
}