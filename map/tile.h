#ifndef _TILE_H_
#define _TILE_H_

#include "sprite.h"
#include "dispBase.h"

typedef struct tile_s {
    int x, y;           // The grid position of this tile
    short bgPalette;    // Palette overrides for this tile

    // For each: 0 is no wall, 1 is wall, >1 is door
    unsigned char lWall : 2;
    unsigned char rWall : 2;
    unsigned char uWall : 2;
    unsigned char dWall : 2;

    signed char isEmpty;

} tile_t;

typedef struct tileData_s {
    dispData_t dispData;    // The underlying dispBase data store
    
    sprite_t emptyBase;     // The empty tile background sprite
    sprite_t tileBase;      // The basic tile background sprite

    sprite_t lWall;         // The left wall sprite
    sprite_t rWall;         // The right wall sprite
    sprite_t uWall;         // The top wall sprite
    sprite_t dWall;         // The bottom wall sprite

    sprite_t lDoor;         // The left door sprite
    sprite_t rDoor;         // The right door sprite
    sprite_t uDoor;         // The top door sprite
    sprite_t dDoor;         // The bottom door sprite
} tileData_t;

/** 
 * Makes a blank tile with no walls or sprite and default palettes
 * 
 * @param x The grid position x value
 * @param y The grid position y value
 * @return A blank tile with the provided positions
 */
tile_t mkTile(int x, int y);

/** 
 * Makes an empty tile with default palettes
 * 
 * @param x The grid position x value
 * @param y The grid position y value
 * @return A blank tile with the provided positions
 */
tile_t mkEmptyTile(int x, int y);

/**
 * Loads a tile data struct's sprites from file
 * 
 * @param fileName The sprite file to load tiles from
 * @param data A return pointer for the tile data struct
 * @return 0 on success, <0 on failure
 */
int loadTileData(FILE* fp, tileData_t * data);

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

/**
 * Draws the provided tile in the proper place on screen
 * 
 * @param data The data structure defining the sprites to draw
 * @param tile The tile to draw to screen
 * @param scrX The x value of the tiles at the left of the screen
 * @param scrY The y value of the tiles at the top of the screen
 */
void drawTile(tileData_t data, tile_t tile, int scrX, int scrY);

void drawWalls(tileData_t data, tile_t tile, int scrX, int scrY);

/**
 * Calculates the width and height of the screen in tiles
 * 
 * @param data The data struct defining screen tiles
 * @param width A return pointer for the width of the screen in tiles
 * @param height A return pointer for the height of the scren in tiles
 */
void getScreenTileDim(tileData_t data, int * width, int * height);

#endif