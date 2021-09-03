#include "tile.h"
//=============================<Data Allocation>==============================//
/** 
 * Makes a blank tile with no walls or sprite and default palettes
 * 
 * @param x The grid position x value
 * @param y The grid position y value
 * @return A blank tile with the provided positions
 */
tile_t mkTile() {
    return (tile_t) {kNoSprite, 0, 0, 0, 0, 0, 0, 0};
}

/** 
 * Makes an empty tile with default palettes
 * 
 * @param x The grid position x value
 * @param y The grid position y value
 * @return A blank tile with the provided positions
 */
tile_t mkEmptyTile() {
    return (tile_t) {kNoSprite, 0, 0, 0, 0, 0, 0, -1};
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

    // Allocate the character sprite
    data->charSprite = mkBlankTile(kWhitePalette, 3, 3);
    if(data->charSprite.data == NULL) goto loadTileDataFail;

    // Set the offsets on the char sprite to the middle
    data->charSprite.xOff = (data->emptyBase.width/2 - 1);
    data->charSprite.yOff = (data->emptyBase.height/2 - 1);

    // Set the border of the sprite
    data->charSprite.data[0][0] = '+';
    data->charSprite.data[0][2] = '+';
    data->charSprite.data[2][0] = '+';
    data->charSprite.data[2][2] = '+';

    data->charSprite.data[0][1] = '-';
    data->charSprite.data[2][1] = '-';
    data->charSprite.data[1][0] = '|';
    data->charSprite.data[1][2] = '|';

    return 0;

loadTileDataFail:
    rmTileData(*data);
    return -1;
}

/**
 * (Internal Helper) Free function for sprite list entries
 * 
 * @param data The sprite entry to free
 */
void freeSpriteEntry(void * data) {
    if(data == NULL) return;
    rmSprite(*(sprite_t *) data);
    free(data);
}


/**
 * Loads a sprite list from file
 * 
 * @param fp The file to read from
 * @param data The tile data to load into
 * 
 * @return 0 on success, < 0 on failure
 */
int loadSpriteList(FILE* fp, tileData_t data) {
    if(fp == NULL || data.spriteList != NULL) return -1;

    data.spriteList = mkList();
    if(data.spriteList == NULL) return -1;

    sprite_t sprite, * entry;

    for(sprite = readSprite(fp); sprite.data != NULL; sprite = readSprite(fp)) {
        entry = malloc(sizeof(sprite_t));
        
        if(entry == NULL) { // On failure to place sprite in heap...
            // Clear out the existing menu items
            rmList(data.spriteList, freeSpriteEntry);
            data.spriteList = NULL;

            // Inform the user and drop back to menu
            return -1;
        } else {    // On success in placing sprite in heap...
            *entry = sprite;
            listAppend(data.spriteList, entry);  // Add sprite to list
        }
    }

    return 0;
}

/**
 * Frees all of the allocated data from the tileData struct
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

    rmList(data.spriteList, freeSpriteEntry);

    rmSprite(data.charSprite);
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
    int ret = fscanf(fp, "%hd %hhu %hhd %d %hd", &tile->bgPalette, &walls, 
                        &tile->isEmpty, &tile->sprite, &tile->spritePalette);
    
    // If any field was missed, return failure
    if(ret < 3) {
        return -1;
    }

    if(ret < 4) {
        tile->sprite = kNoSprite;
    } 
    if (ret < 5) {
        tile->spritePalette = 0;
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

    fprintf(fp, "%hd %hhu %hhd %d %hd\n", tile.bgPalette, walls, tile.isEmpty, 
                tile.sprite, tile.spritePalette);
    return 0;
}

//===============================<Draw Helpers>===============================//
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

/**
 * Draws the walls of the provided tile in the proper place on screen
 * 
 * @param data The data structure defining the sprites to draw
 * @param tile The tile to draw to screen
 * @param scrX The x value of the tiles at the left of the screen
 * @param scrY The y value of the tiles at the top of the screen
 * @param x The x value of the tile in the map
 * @param y The y value of the tile in the map
 */
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

/**
 * Draws the sprite of the provided tile in the proper place on screen
 * 
 * @param data The data structure defining the sprites to draw
 * @param tile The tile to draw to screen
 * @param scrX The x value of the tiles at the left of the screen
 * @param scrY The y value of the tiles at the top of the screen
 * @param x The x value of the tile in the map
 * @param y The y value of the tile in the map
 */
void drawTileSprite(tileData_t data, tile_t tile, int scrX, int scrY, int x, int y) {
    if(tile.sprite == kNoSprite || (tile.sprite >= 0 && (data.spriteList == NULL 
            || (unsigned) tile.sprite >= listLen(data.spriteList)))) {
        return;
    }
    
    // First calculate the screen position of the tile
    unsigned char tileWidth = data.tileBase.width;
    unsigned char tileHeight = data.tileBase.height;

    // Adjust grid coordinates to the view of the screen
    int dX = x - scrX, dY = y - scrY;
    if(dX < 0 || dY < 0) return;    // Nothing to draw above or left of screen

    // Calculate the character position of the tile (and ensure it is onscreen)
    int col = dX * tileWidth, row = dY * tileHeight;
    if(col >= data.dispData.screenCols || row >= data.dispData.screenRows) return;
    

    // Get the sprite to draw
    sprite_t sprite;
    if(tile.sprite < 0) {
        if(data.charSprite.data == NULL) return;
        
        char ch = (-1 * tile.sprite) & 0xFF;
        if(ch < 0x20 || ch > 0x7E) ch = ' ';
        
        short palette = (-1 * tile.sprite) >> 8;
        if (palette<kMinPalette || palette>kMaxPalette) palette = kDefPalette;

        data.charSprite.data[1][1] = ch;
        data.charSprite.palette = palette;
    } else {
        sprite = *(sprite_t *) listGet(data.spriteList, tile.sprite);
    }

    // First draw the Base tile
    short tmp = sprite.palette;
    if(tile.spritePalette != 0) data.tileBase.palette = tile.spritePalette;
    drawSprite(data.dispData, sprite, row, col);
    sprite.palette = tmp;
}

//===========================<Sprite Manipulation>============================//
/**
 * Returns the sprite index of the tile
 * 
 * @param tile The tile to get the sprite index from
 * 
 * @return A sprite index (>= 0) if one exists, -1 if no sprite is set, -2 if 
 *         a character sprite is set
 */
int getSpriteIdx(tile_t tile) {
    return (tile.sprite < -1) ? -2 : tile.sprite;
}

/**
 * Sets a sprite index in the tile
 * 
 * @param data The data structure defining tile sprites
 * @param tile The tile to modify
 * @param idx The sprite index to set
 * 
 * @return 0 on success, < 0 on failure
 */
int setSpriteIdx(tileData_t data, tile_t* tile, int idx) {
    if(data.spriteList == NULL || idx < 0 || 
            (unsigned) idx >= listLen(data.spriteList)) {
        return -1;
    }

    tile->sprite = idx;
    return 0;
}

/**
 * Removes any sprite from the provided tile
 * 
 * @param tile The tile to modify
 */
void clearTileSprite(tile_t* tile) {
    tile->sprite = kNoSprite;
}

/**
 * Sets a character sprite on the provided tile
 * 
 * @param tile The tile to modify
 * @param ch The character to set
 * @param palette The palette to set
 */
void setCharSprite(tile_t* tile, char ch, short palette) {
    if(ch >= 0x20 && ch <= 0x7e) {
        tile->sprite = -1 * (palette << 8 | ch);
    } else {
        tile->sprite = kNoSprite;
    }
}

//===============================<Misc Helpers>===============================//
/**
 * Calculates the width and height of the screen in tiles
 * 
 * @param data The data struct defining screen tiles
 * @param width A return pointer for the width of the screen in tiles
 * @param height A return pointer for the height of the scren in tiles
 */
void getScreenTileDim(tileData_t data, int * width, int * height) {
    if(width != NULL) *width = data.dispData.screenCols / data.emptyBase.width;
    if(height != NULL) *height = data.dispData.screenRows / data.emptyBase.height;
}


