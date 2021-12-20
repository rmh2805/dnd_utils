#ifndef _MAP_DISP_H_
#define _MAP_DISP_H_

#include "../common/dispBase.h"

#include "sprite.h"
#include "tile.h"
#include "map.h"


//==============================<Sprite Display>==============================//
/**
 * Adds the specified sprite to the screen buffer
 * 
 * @param data The display data struct
 * @param sprite The sprite to draw
 * @param palette The palette to draw the sprite in (0 for default)
 * @param screenRow The top row to draw in
 * @param screenCol The left column to draw in
 */
void addSprite(dispData_t * data, sprite_t sprite, short palette, int screenRow, int screenCol);

//===============================<Tile Display>===============================//
/**
 * Buffers the provided tile in the proper place on screen
 * 
 * @param data The data structure defining the sprites to draw
 * @param tile The tile to draw to screen
 * @param scrX The x value of the tiles at the left of the screen
 * @param scrY The y value of the tiles at the top of the screen
 * @param x The x value of the tile in the map
 * @param y The y value of the tile in the map
 */
void addTileBase(tileData_t * data, tile_t tile, int scrX, int scrY, int x, int y);

/**
 * Buffers the walls of the provided tile in the proper place on screen
 * 
 * @param data The data structure defining the sprites to draw
 * @param tile The tile to draw to screen
 * @param scrX The x value of the tiles at the left of the screen
 * @param scrY The y value of the tiles at the top of the screen
 * @param x The x value of the tile in the map
 * @param y The y value of the tile in the map
 */
void addTileWalls(tileData_t * data, tile_t tile, int scrX, int scrY, int x, int y);

/**
 * Buffers the sprite of the provided tile in the proper place on screen
 * 
 * @param data The data structure defining the sprites to draw
 * @param tile The tile to draw to screen
 * @param scrX The x value of the tiles at the left of the screen
 * @param scrY The y value of the tiles at the top of the screen
 * @param x The x value of the tile in the map
 * @param y The y value of the tile in the map
 */
void addTileSprite(tileData_t * data, tile_t tile, int scrX, int scrY, int x, int y);

/**
 * Calculates the width and height of the screen in tiles
 * 
 * @param data The data struct defining screen tiles
 * @param width A return pointer for the width of the screen in tiles
 * @param height A return pointer for the height of the scren in tiles
 */
void getScreenTileDim(tileData_t data, int * width, int * height);

//===============================<Map Display>================================//
/**
 * Buffers the section of the map in view, with focus on the tile at position 
 * (x,y)
 * 
 * @param data The tile data struct to use for display
 * @param map The map to display
 * @param x The x coordinate of the selected tile
 * @param y The y coordinate of the selected cell
 * 
 * @return 0 on success, <0 on failure
 */
int addMap(tileData_t * data, map_t map, int x, int y);

/**
 * Sets cursor focus on the tile at position (x,y)
 * 
 * @param data The tile data struct to use for display
 * @param map The map to display
 * @param x The x coordinate of the selected tile
 * @param y The y coordinate of the selected cell
 */
void setCursor(tileData_t data, map_t map, int x, int y);

/**
 * Renders the map out to the specified file
 * 
 * @param data The tile data to use in rendering
 * @param map The map to render out
 * @param file The file to write out to
 * 
 * @return 0 on success, <0 on failure
 */
int mapToFile(tileData_t data, map_t map, FILE* file);

/**
 * Renders the map out to file in page sections (designed for good txt printout)
 * 
 * @param data The tile data to use in rendering
 * @param map The map to render out
 * @param file The file to write out to
 * @param pgWidth The width of the sections in characters (default 80)
 * @param pgHeight The height of the sections in characters (default 64)
 * @param doSprites Renders the sprite layer iff this is set true
 * 
 * @return 0 on success, <0 on failure
 */
int mapToSections(tileData_t data, map_t map, FILE* file, int pgWidth, int pgHeight, bool doSprites);

#endif