#ifndef _SPRITE_H_
#define _SPRITE_H_

#include <stdlib.h>
#include <stdio.h>

#include "../common/dispBase.h"
#include "../common/list.h"

typedef struct sprite_s {
    short defPalette;               // The default palette for this sprite

    unsigned char width, height;    // Width and height of the sprite
    char xOff, yOff;                // X and Y offsets from top left of tile
    char** data;                    // The actual characters to display as text

} sprite_t;

#define kEmptySprite (sprite_t) {0, 0, 0, 0, 0, NULL}


//============================<Memory Management>=============================//

/**
 * Allocates data for a sprite with the specified dimensions
 * 
 * @param palette The color palette to draw with
 * @param width The width of the sprite (in characters)
 * @param height The height of the sprite (in characters)
 * @param xOff The x offset of the sprite (in characters)
 * @param yOff The y offset of the sprite (in characters)
 * 
 * @return The allocated and initialized sprite
 */
sprite_t mkSprite(short palette, unsigned char width, unsigned char height, unsigned char xOff, unsigned char yOff);

/**
 * Allocates data for a tile with the specified dimensions and initializes it as
 * opaque and blank
 * 
 * @param palette The color palette of the tile
 * @param width The width of the tile (in characters)
 * @param height The height of the tile (in characters)
 * 
 * @return The allocated and initialized sprite
 */
sprite_t mkBlankTile(short palette, unsigned char width, unsigned char height);

/**
 * Frees all data allocated for the sprite
 * 
 * @param sprite The sprite to free
 */
void rmSprite(sprite_t sprite);

/**
 * Copies the contents of a sprite onto the heap for entry into a list
 * 
 * @param sprite The sprite to copy onto the heap
 * @return The pointer to the new heap sprite
 */
sprite_t * mkSpriteEntry(sprite_t sprite);

/**
 * Free function for sprite list entries
 * 
 * @param entry The sprite list entry to free
 */
void freeSpriteEntry(void * entry);

//===============================<File Access>================================//
/**
 * Reads a sprite from file
 * 
 * @param file The file to read from
 * 
 * @return The sprite read from file
 */
sprite_t readSprite(FILE* file);

/**
 * Writes a sprite out to the file
 * 
 * @param file The file to write to
 * @param sprite The sprite to write out
 * 
 * @return 0 iff successful
 */
int writeSprite(FILE* file, sprite_t sprite);

/**
 * Loads all sprites from the provided file and appends them to the provided list
 * 
 * @param file The file to load sprites from
 * @param list The sprite list to load into
 * 
 * @return The number of sprites read (<0 on failure)
 */
int loadSpriteList(FILE* file, list_t * list);

/**
 * Writes all sprites in the given list out to the provided file
 * 
 * @param file The file to save the sprites to
 * @param list The sprite list to save from
 * 
 * @return The number of sprites saved (<0 on failure)
 */
int saveSpriteList(FILE* file, list_t list);

/**
 * Reads a sprite from file into the referenced sprite
 * 
 * @param entry A double pointer to read in to
 * @param fp The file to read the actor from
 * 
 * @return 0 on success, <0 on failure
 */
int readSpriteEntry(void** entry, FILE* fp);

/**
 * Writes the referand of an entry pointer out to file
 * 
 * @param entry A pointer to the sprite to write out
 * @param fp The file to write the sprite to
 * 
 * @return 0 on success, <0 on failure
 */
int writeSpriteEntry(void* entry, FILE* fp);

#endif