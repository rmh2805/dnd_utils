#ifndef _SPRITE_H_
#define _SPRITE_H_

#include <stdlib.h>
#include <stdio.h>


typedef struct sprite_s {
    int palette;

    unsigned char width, height;    // Width and height of the sprite
    unsigned char xOff, yOff;       // X and Y offsets from top left of tile
    char** data;                    // The actual characters to display as text

} sprite_t;

/**
 * Allocates data for a sprite with the specified dimensions
 * 
 * @param width The width of the sprite (in characters)
 * @param height The height of the sprite (in characters)
 * 
 * @return The allocated and initialized sprite
 */
sprite_t mkSprite(unsigned char width, unsigned char height, unsigned char xOff, unsigned char yOff);

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


#endif