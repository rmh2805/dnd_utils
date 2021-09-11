#ifndef _SPRITE_H_
#define _SPRITE_H_

#include <stdlib.h>
#include <stdio.h>

#include "dispBase.h"

typedef struct sprite_s {
    short palette;

    unsigned char width, height;    // Width and height of the sprite
    unsigned char xOff, yOff;       // X and Y offsets from top left of tile
    char** data;                    // The actual characters to display as text

} sprite_t;

#define kEmptySprite (sprite_t) {0, 0, 0, 0, 0, NULL}

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
 * Adds the specified sprite to the screen buffer
 * 
 * @param data The display data struct
 * @param sprite The sprite to draw
 * @param screenRow The top row to draw in
 * @param screenCol The left column to draw in
 */
void addSprite(dispData_t * data, sprite_t sprite, int screenRow, int screenCol);


#endif