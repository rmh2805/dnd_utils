#ifndef _TILE_H_
#define _TILE_H_

#include "wallSprites.h"
#include "sprite.h"
#include "../common/dispBase.h"
#include "../common/list.h"

#define kNoSprite -1
#define writeCharSprite(palette, ch) (-1 * (palette << 8 | ch))

typedef struct tile_s {
    int sprite;             // The index of the sprite used on this tile
    short bgPalette;        // Background palette for this tile
    short bgOverride;       // Background palette override for this tile
    short spritePalette;    // Sprite palette for this tile
    short spriteOverride;   // sprite palette override for this tile

    // For each: 0 is no wall, 1 is wall, >1 is door
    unsigned char lWall : 2;
    unsigned char rWall : 2;
    unsigned char uWall : 2;
    unsigned char dWall : 2;

    // Controls sprite emptyness
    signed char isEmpty;
} tile_t;

typedef struct tileData_s {
    dispData_t dispData;    // The underlying dispBase data store
    
    // Base Layer Definitions
    sprite_t emptyBase;     // The empty tile background sprite
    sprite_t tileBase;      // The basic tile background sprite

    // Wall layer definitions
    sprite_t lWall;         // The left wall sprite
    sprite_t rWall;         // The right wall sprite
    sprite_t uWall;         // The top wall sprite
    sprite_t dWall;         // The bottom wall sprite

    sprite_t lDoor;         // The left door sprite
    sprite_t rDoor;         // The right door sprite
    sprite_t uDoor;         // The top door sprite
    sprite_t dDoor;         // The bottom door sprite

    // Sprite Layer definitions
    list_t spriteList;      // The list of sprites to use
    sprite_t charSprite;    // The basic character sprite
} tileData_t;

//=============================<Data Allocation>==============================//
/** 
 * Makes a blank tile with no walls or sprite and default palettes
 * 
 * @return A blank tile
 */
tile_t mkTile();
/** 
 * Makes an empty tile with default palettes
 * 
 * @return An empty tile 
 */
tile_t mkEmptyTile();

/**
 * Loads a tile data struct's sprites from file
 * 
 * @param fileName The sprite file to load tiles from
 * @param data A return pointer for the tile data struct
 * @return 0 on success, <0 on failure
 */
int loadTileData(tileData_t * data);

/**
 * Frees all of the allocated data from the tileData struct
 * 
 * @param tileData The tileData struct to free from
 */
void rmTileData(tileData_t tileData);

/**
 * Reads a tile written to the current line of the file
 * 
 * @param tile A return pointer for the tile read in
 * @param fp The file pointer to read from
 * 
 * @return 0 on success, < 0 on failure
 */
int readTile(tile_t * tile, FILE* fp);

/**
 * Writes a tile to a line in the provided file
 * 
 * @param tile The tile to write out
 * @param fp The file pointer to write to
 * 
 * @return 0 on success, < 0 on failure
 */
int writeTile(tile_t tile, FILE* fp);

//===========================<Sprite Manipulation>============================//

/**
 * Returns the sprite index of the tile
 * 
 * @param tile The tile to get the sprite index from
 * 
 * @return A sprite index (>= 0) if one exists, -1 if no sprite is set, -2 if 
 *         a character sprite is set
 */
int getSpriteIdx(tile_t tile);

/**
 * Sets a sprite index in the tile
 * 
 * @param data The data structure defining tile sprites
 * @param tile The tile to modify
 * @param idx The sprite index to set
 * 
 * @return 0 on success, < 0 on failure
 */
int setSpriteIdx(tileData_t data, tile_t* tile, int idx);

/**
 * Removes any sprite from the provided tile
 * 
 * @param tile The tile to modify
 */
void clearTileSprite(tile_t* tile);

/**
 * Sets a character sprite on the provided tile
 * 
 * @param tile The tile to modify
 * @param ch The character to set
 * @param palette The palette to set
 */
void setCharSprite(tile_t* tile, char ch, short palette);

/**
 * Extracts a charSprite's character from its encoding
 * 
 * @param spriteCode The encoding for the char sprite
 * @return The char sprite's character
 */
char getCharSpriteChar(int spriteCode);

/**
 * Extract's a charSprite's default palette from its encoding
 * 
 * @param spriteCode The encoding for the char sprite
 * @return The char sprite's character
 */
short getCharSpritePalette(int spriteCode);

#endif