#include "tile.h"

/** 
 * Makes a blank tile with no walls or sprite and default palettes
 * 
 * @param x The grid position x value
 * @param y The grid position y value
 * @return A blank tile with the provided positions
 */
tile_t mkTile() {
    return (tile_t) {kNoSprite, 0, 0, 0, 0, 0, 0};
}

/** 
 * Makes an empty tile with default palettes
 * 
 * @param x The grid position x value
 * @param y The grid position y value
 * @return A blank tile with the provided positions
 */
tile_t mkEmptyTile() {
    return (tile_t) {kNoSprite, 0, 0, 0, 0, 0, -1};
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
 * Frees all of the allocated data from the tileData struct (besides the sprite
 *  list)
 * 
 * @param tileData The tileData struct to free from
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
 * Reads a tile written to the current line of the file
 * 
 * @param tile A return pointer for the tile read in
 * @param fp The file pointer to read from
 * 
 * @return 0 on success, < 0 on failure
 */
int readTile(tile_t * tile, FILE* fp) {
    unsigned char walls;

    // Read in the raw data from the next line
    int ret = fscanf(fp, "%hd %hhu %hhd %d", &tile->bgPalette, &walls, 
                        &tile->isEmpty, &tile->sprite);
    
    // If any field was missed, return failure
    if(ret == 3) {
        tile->sprite = -1;
    } else if (ret < 3) {
        return -1;
    }

    // Since all were decoded properly, extract walls
    tile->lWall = (walls >> 6) & 0x03;
    tile->rWall = (walls >> 4) & 0x03;
    tile->uWall = (walls >> 2) & 0x03;
    tile->dWall = walls & 0x03;

    return 0;
}

/**
 * Writes a tile to a line in the provided file
 * 
 * @param tile The tile to write out
 * @param fp The file pointer to write to
 * 
 * @return 0 on success, < 0 on failure
 */
int writeTile(tile_t tile, FILE* fp) {
    if(fp == NULL) return -1;

    unsigned char walls = (tile.lWall << 6) | (tile.rWall << 4) | 
                            (tile.uWall << 2) | (tile.dWall);

    fprintf(fp, "%hd %hhu %hhd %d\n", tile.bgPalette, walls, tile.isEmpty, tile.sprite);
    return 0;
}

/**
 * Draws the provided tile in the proper place on screen
 * 
 * @param data The data structure defining the sprites to draw
 * @param tile The tile to draw to screen
 * @param scrX The x value of the tiles at the left of the screen
 * @param scrY The y value of the tiles at the top of the screen
 * @param x The x value of the tile in the map
 * @param y The y value of the tile in the map
 */
void drawTile(tileData_t data, tile_t tile, int scrX, int scrY, int x, int y) {
    // First calculate the screen position of the tile
    unsigned char tileWidth = data.tileBase.width;
    unsigned char tileHeight = data.tileBase.height;

    // Adjust grid coordinates to the view of the screen
    int dX = x - scrX, dY = y - scrY;
    if(dX < 0 || dY < 0) return;    // Nothing to draw above or left of screen

    // Calculate the character position of the tile (and ensure it is onscreen)
    int col = dX * tileWidth, row = dY * tileHeight;
    if(col >= data.dispData.screenCols || row >= data.dispData.screenRows) return;
    

    // If the tile is empty, draw the empty tile and be done with it
    if(tile.isEmpty) {
        drawSprite(data.dispData, data.emptyBase, row, col);
        return;
    }

    // First draw the Base tile
    short tmp = data.tileBase.palette;
    if(tile.bgPalette != 0) data.tileBase.palette = tile.bgPalette;
    drawSprite(data.dispData, data.tileBase, row, col);
    data.tileBase.palette = tmp;
}

void drawWalls(tileData_t data, tile_t tile, int scrX, int scrY, int x, int y) {
    // First calculate the screen position of the tile
    unsigned char tileWidth = data.tileBase.width;
    unsigned char tileHeight = data.tileBase.height;

    // Adjust grid coordinates to the view of the screen
    int dX = x - scrX, dY = y - scrY;
    if(dX < 0 || dY < 0) return;    // Nothing to draw above or left of screen

    // Calculate the character position of the tile (and ensure it is onscreen)
    int col = dX * tileWidth, row = dY * tileHeight;
    if(col >= data.dispData.screenCols || row >= data.dispData.screenRows) return;

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
    drawSprite(data.dispData, sprite, row, col);
    
    switch(tile.rWall) {
        case 0:
            break;
        case 1:
            sprite = data.rWall;
            break;
        default:
            sprite = data.rDoor;
    }
    drawSprite(data.dispData, sprite, row, col);
    
    switch(tile.uWall) {
        case 0:
            break;
        case 1:
            sprite = data.uWall;
            break;
        default:
            sprite = data.uDoor;
    }
    drawSprite(data.dispData, sprite, row, col);
    
    switch(tile.dWall) {
        case 0:
            break;
        case 1:
            sprite = data.dWall;
            break;
        default:
            sprite = data.dDoor;
    }
    drawSprite(data.dispData, sprite, row, col);

}

void getScreenTileDim(tileData_t data, int * width, int * height) {
    if(width != NULL) *width = data.dispData.screenCols / data.emptyBase.width;
    if(height != NULL) *height = data.dispData.screenRows / data.emptyBase.height;
}

/**
 * Generates a sprite based on a recovered value
 * 
 * @param val The value to recover from
 * @return The sprite recovered from the val (an empty sprite if none recovered)
 */
sprite_t mkCharSprite(int val) {
    if(val >= 0) {
        return kEmptySprite;
    }

    val = val * -1;
    char ch = val & 0xFF;
    short palette = (val >> 8) & 0xFFFF;

    if(ch < 0x20 || ch > 0x7e) { // If char is non-visible...
        return kEmptySprite;
    }

    sprite_t sprite = mkBlankTile(palette, 3, 3);
    sprite.data[1][1] = ch;
    return sprite;
}