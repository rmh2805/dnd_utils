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

int loadActorData(actorData_t* data) {
    if(data == NULL) {
        return -1;
    }

    memset(data, 0, sizeof(actorData_t));

    // Allocate the lists
    if((data->pcActors = mkList()) == NULL) goto loadActorData_fail;
    if((data->npcActors = mkList()) == NULL) goto loadActorData_fail;
    if((data->enemyActors = mkList()) == NULL) goto loadActorData_fail;
    if((data->actorSprites = mkList()) == NULL) goto loadActorData_fail;

    // Allocate the default sprite
    data->defActorSprite = mkSprite(kDefPalette, kTileWidth-1, kTileHeight-1, 1, 1);
    if(data->defActorSprite.data == NULL) goto loadActorData_fail;

    // Initialize the default sprite with whitespace
    for(int row = 0; row < data->defActorSprite.height; ++row) {
        memset(data->defActorSprite.data[row], ' ', data->defActorSprite.width);
    }

    return 0;

loadActorData_fail:
    rmActorData(*data);
    return -1;
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
    if(data.defActorSprite.data != NULL) {
        rmSprite(data.defActorSprite);
    }
}

int mkActor(actor_t * actor, const char * name, const char * status, int maxHP, int deltaHP, int spriteIdx) {
    // Ensure that the actor pointer is not null
    if(actor == NULL) {
        return -1;
    }

    // Zero-fill the actor struct
    memset(actor, 0, sizeof(actor_t));

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

int writeActorEntry(void* entry, FILE* fp) {
    return writeActor(*(actor_t*)entry, fp);
}

int readActorEntry(void** entry, FILE* fp) {
    if(entry == NULL) return -1;

    // Free any pre-existing actor
    if(*entry != NULL) {
        freeActorEntry(*(actor_t**)entry);
    }

    // Read the actor in to the stack
    actor_t actor;
    if(readActor(&actor, fp) < 0) {
        return -1;
    }

    // Move the actor from stack to heap
    if((*(actor_t**)entry = mkActorEntry(actor)) == NULL) {
        rmActor(actor);
        return -1;
    }

    // Return success
    return 0;
}

int writeActorData(actorData_t data, FILE* fp) {
    if(fp == NULL) return -1;

    if(saveList(data.pcActors, fp, writeActorEntry) < 0) return -1;
    if(saveList(data.enemyActors, fp, writeActorEntry) < 0) return -1;
    if(saveList(data.npcActors, fp, writeActorEntry) < 0) return -1;
    if(saveList(data.actorSprites, fp, writeSpriteEntry) < 0) return -1;

    return 0;
}

int readActorData(actorData_t* data, FILE* fp) {
    if(data == NULL || fp == NULL) return -1;

    // Initialize the actor data struct
    if(loadActorData(data) < 0) return -1;

    // Load in the lists from file
    if(loadList(&data->pcActors, fp, readActorEntry) < 0) goto readActorData_fail;
    if(loadList(&data->enemyActors, fp, readActorEntry) < 0) goto readActorData_fail;
    if(loadList(&data->npcActors, fp, readActorEntry) < 0) goto readActorData_fail;
    if(loadList(&data->actorSprites, fp, readSpriteEntry) < 0) goto readActorData_fail;

    return 0;

readActorData_fail:
    rmActorData(*data);
    return -1;
}

int writePlaySession(actorData_t data, map_t map, list_t mapSprites, FILE* fp) {
    if(fp == NULL) return -1;

    // Write the actor data to file
    if(writeActorData(data, fp) < 0) return -1;
    if(writeMapOverrides(map, mapSprites, fp) < 0) return -1;

    // Return succes
    return 0;
}


int readPlaySession(actorData_t* data, map_t* map, list_t* mapSprites, FILE* fp) {
    if(data == NULL || map == NULL || mapSprites == NULL || fp == NULL) {
        return -1;
    }

    // Read in the actor data from file (read fxn does allocation)
    if(readActorData(data, fp) < 0) {
        return -1;
    }

    // Read in the map and sprites from file
    if(loadMapOverrides(map, mapSprites, fp) < 0) {
        rmActorData(*data);
        return -1;
    }

    // Return success
    return 0;
}
