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
int mkActor(actor_t * actor, const char * name, const char * status, int maxHP, int deltaHP, int spriteIdx) {
    // Ensure that the actor pointer is not null
    if(actor == NULL) {
        return -1;
    }

    // Zero-fill the actor struct
    for(size_t i = 0; i < sizeof(*actor); ++i) {
        ((char *) actor)[i] = 0;
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

    // Set the actor's max HP and deltaHP
    actor->maxHP = maxHP;
    actor->deltaHP = deltaHP;

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

actorData_t mkActorData() {
    actorData_t data;
    for(size_t i = 0; i < sizeof(data); ++i) {
        ((char*)(&data))[i] = '\0';
    }

    return data;
}

void rmActorData(actorData_t data) {
    if(data.actorSprites != NULL) {
        rmList(data.actorSprites, freeSpriteEntry);
    }
    if(data.pcActors != NULL) {
        rmList(data.pcActors, freeActorEntry);
    }
    if(data.npcActors != NULL) {
        rmList(data.npcActors, freeActorEntry);
    }
    if(data.enemyActors != NULL) {
        rmList(data.enemyActors, freeActorEntry);
    }
}

//===============================<File Access>================================//

#define writeActorHelper_str(str) fprintf(fp, "%lu |%s\n", (str == NULL) ? 0 : strlen(str), (str == NULL) ? "" : str)

int writeActor(actor_t actor, FILE* fp) {
    if(fp == NULL) {
        return -1;
    }

    writeActorHelper_str(actor.name);
    writeActorHelper_str(actor.status);

    fprintf(fp, "%d %d %d %d %d %hd %hd\n", actor.maxHP, actor.deltaHP, 
                actor.spriteIdx, actor.x, actor.y, actor.palette, 
                actor.paletteOverride);

    return 0;
}

int readActorHelper_str(FILE* fp, char** str) {
    if(fp == NULL || str == NULL) {
        return -1;
    }

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

    fread(*str, sizeof(char), bufSize, fp);
    return 0;
}

int readActor(actor_t* actor, FILE* fp) {
    if(actor == NULL || fp == NULL) {
        return -1;
    }

    if(readActorHelper_str(fp, &actor->name) < 0) {
        rmActor(*actor);
        return -1;
    }
    if(readActorHelper_str(fp, &actor->status) < 0) {
        rmActor(*actor);
        return -1;
    }

    int ret = fscanf(fp, "%d %d %d %d %d %hd %hd\n", &actor->maxHP, &actor->deltaHP, 
                        &actor->spriteIdx, &actor->x, &actor->y, &actor->palette, 
                        &actor->paletteOverride);

    if(ret != 7) {
        rmActor(*actor);
        return -1;
    }

    return 0;
}

int writeActorEntry(actor_t* entry, FILE* fp) {
    return writeActor(*entry, fp);
}

int readActorEntry(actor_t** entry, FILE* fp) {
    if(entry == NULL) return -1;

    // Free any pre-existing actor
    if(*entry != NULL) {
        freeActorEntry(*entry);
    }

    // Read the actor in to the stack
    actor_t actor;
    if(readActor(&actor, fp) < 0) {
        return -1;
    }

    // Move the actor from stack to heap
    if((*entry = mkActorEntry(actor)) == NULL) {
        rmActor(actor);
        return -1;
    }

    // Return success
    return 0;
}
