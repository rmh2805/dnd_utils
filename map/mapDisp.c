#include "mapDisp.h"

#ifndef min
#define min(a, b) ((a < b) ? a : b)
#endif

#ifndef max
#define max(a, b) ((a > b) ? a : b)
#endif

//==============================<Sprite Display>==============================//
void addSprite(dispData_t * data, sprite_t sprite, short palette, int screenRow, int screenCol) {
    if(data == NULL || data->data == NULL) return;
    if(palette == 0) palette = sprite.defPalette; 

    screenRow = screenRow + sprite.yOff;
    screenCol = screenCol + sprite.xOff;
    if (sprite.data == NULL || 
            screenRow + sprite.height <= 0 || screenCol + sprite.width <= 0 ||
            screenRow >= data->screenRows || screenCol >= data->screenCols) {
        return;
    }

    for(int dRow = 0; dRow < sprite.height; dRow++) {
        int row = screenRow + dRow;
        if(row < 0 || row >= data->screenRows) continue;

        for(int dCol = 0; dCol < sprite.width; dCol++) {
            int col = screenCol + dCol;
            if(col < 0 || col >= data->screenCols) continue;

            char ch = sprite.data[dRow][dCol];
            if(ch == '\0') {
                continue;
            } else {
                data->data[row][col] = (drawPair_t) {palette, sprite.data[dRow][dCol]};
            }
        }
    }
}

//===============================<Tile Display>===============================//

void addTile(tileData_t * data, tile_t tile, int scrX, int scrY, int x, int y) {
    if(data == NULL) return;

    // First calculate the screen position of the tile
    unsigned char tileWidth = data->tileBase.width;
    unsigned char tileHeight = data->tileBase.height;

    // Adjust grid coordinates to the view of the screen
    int dX = x - scrX, dY = y - scrY;
    if(dX < 0 || dY < 0) return;    // Nothing to draw above or left of screen

    // Calculate the character position of the tile (and ensure it is onscreen)
    int col = dX * tileWidth, row = dY * tileHeight;
    if(col >= data->dispData.screenCols || row >= data->dispData.screenRows) return;
    

    // If the tile is empty, draw the empty tile and be done with it
    if(tile.isEmpty) {
        addSprite(&data->dispData, data->emptyBase, 0, row, col);
        return;
    }

    // First draw the Base tile
    short tmp = data->tileBase.defPalette;
    short palette = (tile.bgOverride != 0) ? tile.bgOverride : tile.bgPalette;
    addSprite(&data->dispData, data->tileBase, palette, row, col);
    data->tileBase.defPalette = tmp;
}

void addWalls(tileData_t * data, tile_t tile, int scrX, int scrY, int x, int y) {
    if(data == NULL) return;

    // First calculate the screen position of the tile
    unsigned char tileWidth = data->tileBase.width;
    unsigned char tileHeight = data->tileBase.height;

    // Adjust grid coordinates to the view of the screen
    int dX = x - scrX, dY = y - scrY;
    if(dX < 0 || dY < 0) return;    // Nothing to draw above or left of screen

    // Calculate the character position of the tile (and ensure it is onscreen)
    int col = dX * tileWidth, row = dY * tileHeight;
    if(col >= data->dispData.screenCols || row >= data->dispData.screenRows) return;

    sprite_t sprite = kEmptySprite;
    switch(tile.lWall) {
        case 0:
            break;
        case 1:
            sprite = data->lWall;
            break;
        default:
            sprite = data->lDoor;
    }
    addSprite(&data->dispData, sprite, 0, row, col);
    
    switch(tile.rWall) {
        case 0:
            break;
        case 1:
            sprite = data->rWall;
            break;
        default:
            sprite = data->rDoor;
    }
    addSprite(&data->dispData, sprite, 0, row, col);
    
    switch(tile.uWall) {
        case 0:
            break;
        case 1:
            sprite = data->uWall;
            break;
        default:
            sprite = data->uDoor;
    }
    addSprite(&data->dispData, sprite, 0, row, col);
    
    switch(tile.dWall) {
        case 0:
            break;
        case 1:
            sprite = data->dWall;
            break;
        default:
            sprite = data->dDoor;
    }
    addSprite(&data->dispData, sprite, 0, row, col);

}

void addTileSprite(tileData_t * data, tile_t tile, int scrX, int scrY, int x, int y) {
    if(data == NULL) return;
    
    if(tile.sprite == kNoSprite || (tile.sprite >= 0 && (data->spriteList == NULL
            || (unsigned int) tile.sprite >= listLen(data->spriteList)))) {
        return;
    }
    
    // First calculate the screen position of the tile
    unsigned char tileWidth = data->tileBase.width;
    unsigned char tileHeight = data->tileBase.height;

    // Adjust grid coordinates to the view of the screen
    int dX = x - scrX, dY = y - scrY;
    if(dX < 0 || dY < 0) return;    // Nothing to draw above or left of screen

    // Calculate the character position of the tile (and ensure it is onscreen)
    int col = dX * tileWidth, row = dY * tileHeight;
    if(col >= data->dispData.screenCols || row >= data->dispData.screenRows) return;
    

    // Get the sprite to draw
    sprite_t sprite;
    if(tile.sprite < 0) {
        if(data->charSprite.data == NULL) return;

        sprite = data->charSprite;
        
        char ch = (-1 * tile.sprite) & 0xFF;
        if(ch < 0x20 || ch > 0x7E) ch = ' ';
        
        short palette = (-1 * tile.sprite) >> 8;
        if (palette<kMinPalette || palette>kMaxPalette) palette = kDefPalette;

        sprite.data[1][1] = ch;
        sprite.defPalette = palette;

        sprite = data->charSprite;
    } else {
        sprite = *(sprite_t *) listGet(data->spriteList, tile.sprite);
    }

    // First draw the Base tile
    short tmp = sprite.defPalette;
    short palette = (tile.spriteOverride != 0) ? tile.spriteOverride : tile.spritePalette;
    addSprite(&data->dispData, sprite, palette, row, col);
    sprite.defPalette = tmp;
}

void getScreenTileDim(tileData_t data, int * width, int * height) {
    if(width != NULL) *width = data.dispData.screenCols / data.emptyBase.width;
    if(height != NULL) *height = data.dispData.screenRows / data.emptyBase.height;
}

//===============================<Map Display>================================//

int addMap(tileData_t * data, map_t map, int x, int y) {
    // Determine the position of the screen
    int width, height;  // Width and height of the screen in tiles
    getScreenTileDim(*data, &width, &height);

    // X & Y coords of the top-left tile
    int scrX = max(min(x - width/2, map.nCols-width), 0);
    int scrY = max(min(y - height/2, map.nRows-height), 0);

    // X and Y offsets of the selected tile from the top-left of the screen
    int dX = x - scrX, dY = y-scrY;
    
    // Draw all tiles on the screen
    for(int dRow = 0; dRow < height && dRow + scrY < map.nRows; dRow++) {
        int row = dRow + scrY;
        for(int dCol = 0; dCol < width && dCol + scrX < map.nCols; dCol++) {
            int col = dCol + scrX;
            addTile(data, map.data[row][col], scrX, scrY, col, row);
        }
    }

    // And then draw all of the walls
    for(int dRow = 0; dRow < height && dRow + scrY < map.nRows; dRow++) {
        int row = dRow + scrY;
        for(int dCol = 0; dCol < width && dCol + scrX < map.nCols; dCol++) {
            int col = dCol + scrX;
            addWalls(data, map.data[row][col], scrX, scrY, col, row);
        }
    }

    // Finally, draw all of the sprites
    for(int dRow = 0; dRow < height && dRow + scrY < map.nRows; dRow++) {
        int row = dRow + scrY;
        for(int dCol = 0; dCol < width && dCol + scrX < map.nCols; dCol++) {
            int col = dCol + scrX;
            addTileSprite(data, map.data[row][col], scrX, scrY, col, row);
        }
    }

    // Retarget the selected tile
    move(dY * data->emptyBase.height + data->emptyBase.height/2, 
            dX * data->emptyBase.width + data->emptyBase.width/2);

    return 0;
}

void setCursor(tileData_t data, map_t map, int x, int y) {
    // Determine the position of the screen
    int width, height;  // Width and height of the screen in tiles
    getScreenTileDim(data, &width, &height);

    // X & Y coords of the top-left tile
    int scrX = max(min(x - width/2, map.nCols-width), 0);
    int scrY = max(min(y - height/2, map.nRows-height), 0);

    // X and Y offsets of the selected tile from the top-left of the screen
    int dX = x - scrX, dY = y-scrY;
    
    // Retarget the selected tile
    move(dY * data.emptyBase.height + data.emptyBase.height/2, 
            dX * data.emptyBase.width + data.emptyBase.width/2);
}

int mapSectionToFile(tileData_t data, map_t map, FILE* file, 
        int startRow, int startCol, int endRow, int endCol, bool doSprites);

int mapToFile(tileData_t data, map_t map, FILE* file) {
    return mapSectionToFile(data, map, file, 0, 0, 0, 0, true);
}

int mapToSections(tileData_t data, map_t map, FILE* file, int pgWidth, int pgHeight, bool doSprites) {
    if(file == NULL) return -1;

    // Determine the number of rows and columns per page (and extra lines needed)
    int pgRows = pgHeight/data.emptyBase.height;
    if(pgRows == 0) return -2;
    int pgCols = pgWidth/data.emptyBase.width;
    if(pgCols == 0) return -2;
    int pgExcess = pgHeight - pgRows*data.emptyBase.height;

    // Iterate through all of the page groups
    for(int pgRank = 0; pgRank * pgRows < map.nRows; pgRank++) {
        for(int pgFile = 0; pgFile * pgCols < map.nCols; pgFile++) {
            int ret = mapSectionToFile(data, map, file, 
                    pgRank * pgRows, pgFile * pgCols,
                    (pgRank+1) * pgRows, (pgFile+1) * pgCols, doSprites); 
            if( ret < 0) {
                return ret - 2;
            }
            for(int i = 0; i < pgExcess; i++) {
                fprintf(file, "\n");
            }
        }
    }

    return 0;
}

#define mapFileNextChar(file, ch) fprintf(file, "%c", ch)
int mapSectionToFile(tileData_t data, map_t map, FILE* file, 
        int startRow, int startCol, int endRow, int endCol, bool doSprites) {
    if(file == NULL) return -1;
    if(startRow > endRow || startCol > endCol) return -2;
    if(startRow > map.nRows || startCol > map.nCols) return 1;

    if(endRow == 0 || endRow > map.nRows) endRow = map.nRows;
    if(endCol == 0 || endCol > map.nCols) endCol = map.nCols;

    for(int row = startRow; row < endRow; row++) {
        for(int line = 0; line < data.emptyBase.height; line++) {
            for(int col = startCol; col < endCol; col++) {
                for(int i = 0; i < data.emptyBase.width; i++) {
                    // First attempt to render the sprite layer
                    if(doSprites && map.data[row][col].sprite != kNoSprite && (map.data[row][col].sprite < 0 || 
                            (data.spriteList != NULL && (unsigned) map.data[row][col].sprite < listLen(data.spriteList)))) {
                        // There is some sprite...

                        int spriteNr = map.data[row][col].sprite;
                        if(spriteNr < 0 && data.charSprite.data != NULL && 
                                line >= data.charSprite.yOff && line < data.charSprite.yOff + data.charSprite.height &&
                                i >= data.charSprite.xOff && i < data.charSprite.xOff + data.charSprite.width) {
                            // Check for char sprite data
                            int y = line-data.charSprite.yOff, x = i-data.charSprite.xOff;
                            
                            mapFileNextChar(file,(x==1 && y==1) ? (char)(-1 * spriteNr) & 0xFF : data.charSprite.data[y][x]);
                            continue;
                        }
                        //todo Check for real sprites, as well

                    }

                    // Next attempt to render the wall layer
                    if(map.data[row][col].uWall == 1 && line < data.uWall.height && data.uWall.data[line][i]) {
                        // Check for characters in the upper wall
                        mapFileNextChar(file, data.uWall.data[line][i]);
                        continue;
                    } else if(map.data[row][col].uWall == 2 && line < data.uDoor.height && data.uDoor.data[line][i]) {
                        // Check for characters in the upper door
                        mapFileNextChar(file, data.uDoor.data[line][i]);
                        continue;
                    } else if(map.data[row][col].lWall == 1 && i < data.lWall.width && data.lWall.data[line][i]) {
                        // Check for characters in the left wall
                        mapFileNextChar(file, data.lWall.data[line][i]);
                        continue;
                    } else if(map.data[row][col].lWall == 2 && i < data.lDoor.width && data.lDoor.data[line][i]) {
                        // Check for characters in the left door
                        mapFileNextChar(file, data.lDoor.data[line][i]);
                        continue;
                    } else if(map.data[row][col].dWall == 1 && line >= data.emptyBase.height - data.dWall.height 
                                && data.dWall.data[line - (data.emptyBase.height - data.dWall.height)][i]) {
                        // Check for characters in the lower wall
                        mapFileNextChar(file, data.dWall.data[line - (data.emptyBase.height - data.dWall.height)][i]);
                        continue;
                    } else if(map.data[row][col].dWall == 2 && line >= data.emptyBase.height - data.dDoor.height 
                                && data.dDoor.data[line - (data.emptyBase.height - data.dDoor.height)][i]) {
                        // Check for characters in the lower door
                        mapFileNextChar(file, data.dDoor.data[line - (data.emptyBase.height - data.dDoor.height)][i]);
                        continue;
                    } else if(map.data[row][col].rWall == 1 && i >= data.emptyBase.width - data.rWall.width 
                                && data.rWall.data[line][i - (data.emptyBase.width - data.rWall.width)]) {
                        // Check for characters in the right wall
                        mapFileNextChar(file, data.rWall.data[line][i - (data.emptyBase.width - data.rWall.width)]);
                        continue;
                    } else if(map.data[row][col].rWall == 2 && i >= data.emptyBase.width - data.rDoor.width 
                                && data.rDoor.data[line][i - (data.emptyBase.width - data.rDoor.width)]) {
                        // Check for characters in the right wall
                        mapFileNextChar(file, data.rDoor.data[line][i - (data.emptyBase.width - data.rDoor.width)]);
                        continue;
                    } 
                    
                    // Finally, render the base layer
                    if(!map.data[row][col].isEmpty && data.tileBase.data[line][i]) {
                        // Check for characters in the base tile
                        mapFileNextChar(file, data.tileBase.data[line][i]);
                        continue;
                    }
                    
                    // Output the proper character to file
                    mapFileNextChar(file, ' ');

                }
            }
            fprintf(file, "\n");
        }
    }

    return 0;
}