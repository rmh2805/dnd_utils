#ifndef _TILE_H_
#define _TILE_H_

#include "sprite.h"
#include "dispBase.h"

typedef struct tile_s {
    sprite_t sprite;    // The sprite on top of this tile

    int x, y;           // The grid position of this tile
    short bgPalette;    // Palette overrides for this tile

    // For each: 0 is no wall, 1 is wall, >1 is door
    unsigned char lWall : 2;
    unsigned char rWall : 2;
    unsigned char uWall : 2;
    unsigned char dWall : 2;

} tile_t;

typedef struct tileData_s {
    dispData_t dispData;    // The underlying dispBase data store
    
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
 * Draws the provided tile in the proper place on screen
 * 
 * @param data The data structure defining the sprites to draw
 * @param tile The tile to draw to screen
 * @param scrX The x value of the tiles at the left of the screen
 * @param scrY The y value of the tiles at the top of the screen
 */
void drawTile(tileData_t data, tile_t tile, int scrX, int scrY);

#endif