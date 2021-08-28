#include "tile.h"

/** 
 * Makes a blank tile with no walls or sprite and default palettes
 * 
 * @param x The grid position x value
 * @param y The grid position y value
 * @return A blank tile with the provided positions
 */
tile_t mkTile(int x, int y) {
    return (tile_t) {NULL, x, y, kDefPalette, 0, 0, 0, 0, 0};
}

/** 
 * Makes an empty tile with default palettes
 * 
 * @param x The grid position x value
 * @param y The grid position y value
 * @return A blank tile with the provided positions
 */
tile_t mkEmptyTile(int x, int y) {
    return (tile_t) {NULL, x, y, kDefPalette, 0, 0, 0, 0, -1};
}


#define loadTileDataHelp(field) \
    data->field = readSprite(fp);\
    if(data->field.data == NULL) goto loadTileDataFail

/**
 * Loads a tile data struct's sprites from file
 * 
 * @param fileName The sprite file to load tiles from
 * @param data A return pointer for the tile data struct
 * @return 0 on success, <0 on failure
 */
int loadTileData(FILE* fp, tileData_t * data) {
    loadTileDataHelp(emptyBase);
    loadTileDataHelp(tileBase);
    loadTileDataHelp(lWall);
    loadTileDataHelp(rWall);
    loadTileDataHelp(uWall);
    loadTileDataHelp(dWall);
    loadTileDataHelp(lDoor);
    loadTileDataHelp(rDoor);
    loadTileDataHelp(uDoor);
    loadTileDataHelp(dDoor);
    return 0;

loadTileDataFail:
    rmTileData(*data);
    return -1;
}

/**
 * Frees all of the allocated data from the tileData struct
 */
void rmTileData(tileData_t data) {
    rmSprite(data.emptyBase);
    rmSprite(data.tileBase);
    rmSprite(data.lWall);
    rmSprite(data.rWall);
    rmSprite(data.uWall);
    rmSprite(data.dWall);
    rmSprite(data.lDoor);
    rmSprite(data.rDoor);
    rmSprite(data.uDoor);
    rmSprite(data.dDoor);
}

/**
 * Draws the provided tile in the proper place on screen
 * 
 * @param data The data structure defining the sprites to draw
 * @param tile The tile to draw to screen
 * @param scrX The x value of the tiles at the left of the screen
 * @param scrY The y value of the tiles at the top of the screen
 */
void drawTile(tileData_t data, tile_t tile, int scrX, int scrY) {
    // First calculate the screen position of the tile
    unsigned char tileWidth = data.tileBase.width;
    unsigned char tileHeight = data.tileBase.height;

    // Adjust grid coordinates to the view of the screen
    int dX = tile.x - scrX, dY = tile.y - scrY;
    if(dX < 0 || dY < 0) return;    // Nothing to draw above or left of screen

    // Calculate the character position of the tile (and ensure it is onscreen)
    int x = dX * tileWidth, y = dY * tileHeight;
    if(x >= data.dispData.screenCols || y >= data.dispData.screenRows) return;
    
    // If the tile is empty, draw the empty tile and be done with it
    if(tile.isEmpty) {
        drawSprite(data.dispData, data.emptyBase, y, x);
        return;
    }

    // First draw the Base tile
    short tmp = data.tileBase.palette;
    if(tile.bgPalette != kDefPalette) data.tileBase.palette = tile.bgPalette;
    drawSprite(data.dispData, data.tileBase, y, x);
    data.tileBase.palette = tmp;

    // Next draw each of the walls
    sprite_t sprite = kEmptySprite;
    switch(tile.lWall) {
        case 0:
            break;
        case 1:
            sprite = data.lWall;
            break;
        default:
            sprite = data.lDoor;
    }
    drawSprite(data.dispData, sprite, y, x);
    
    switch(tile.rWall) {
        case 0:
            break;
        case 1:
            sprite = data.rWall;
            break;
        default:
            sprite = data.rDoor;
    }
    drawSprite(data.dispData, sprite, y, x);
    
    switch(tile.uWall) {
        case 0:
            break;
        case 1:
            sprite = data.uWall;
            break;
        default:
            sprite = data.uDoor;
    }
    drawSprite(data.dispData, sprite, y, x);
    
    switch(tile.dWall) {
        case 0:
            break;
        case 1:
            sprite = data.dWall;
            break;
        default:
            sprite = data.dDoor;
    }
    drawSprite(data.dispData, sprite, y, x);

    // Finally, draw the sprite itself
    drawSprite(data.dispData, *tile.sprite, y, x);

}