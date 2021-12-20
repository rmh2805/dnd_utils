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
    tile_t tile;
    for(size_t i = 0; i < sizeof(tile); i++) {
        ((char *) &tile)[i] = 0;
    }

    tile.sprite = kNoSprite;
    return tile;
}

tile_t mkEmptyTile() {
    tile_t tile;
    for(size_t i = 0; i < sizeof(tile); i++) {
        ((char *) &tile)[i] = 0;
    }

    tile.sprite = kNoSprite;
    tile.isEmpty = true;
    return tile;
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
    data->spriteList = NULL;

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
        if(tile->sprite > 0) tile->sprite = kNoSprite;  // Safety check
    } else {
        tile->sprite = kNoSprite;
    }
}

char getCharSpriteChar(int spriteCode) {
    if(spriteCode >= 0 || spriteCode == kNoSprite) return '\0';
    
    char ch = (spriteCode * -1) & 0xFF;
    if(ch < 0x20 || ch > 0x7e) {    // Ensure it's in the valid range for encoding
        return '\0';
    }

    return ch;
}

short getCharSpritePalette(int spriteCode) {
    if(spriteCode >= 0 || spriteCode == kNoSprite) {
        return 0;
    }

    return (spriteCode * -1) >> 8;
}

