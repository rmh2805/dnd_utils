#include "mapDisp.h"

#ifndef min
#define min(a, b) ((a < b) ? a : b)
#endif

#ifndef max
#define max(a, b) ((a > b) ? a : b)
#endif

//==================================<Helpers>=================================//
/**
 * Returns the screen position of a tile
 * 
 * @param data The tile data struct to use for the display
 * @param scrX The map x coordinate of the left-most tiles
 * @param scrY The map y coordinate of the upper-most tiles
 * @param x The map x coordinate of the tile in question
 * @param y The map y coordinate of the tile in question
 * 
 * @param col A return pointer for the screen column
 * @param row A return pointer for the screen row
 * @return 0 if tile is visible, <0 otherwise
 */
int getScreenRowCol(tileData_t * data, int scrX, int scrY, int x, int y, int * col, int * row) {
    if(data == NULL) return -1;

    // Adjust grid coordinates to the view of the screen
    int dX = x - scrX, dY = y - scrY;
    if(dX < 0 || dY < 0) {
        return -1;    // Nothing to draw above or left of screen
    }

    // First calculate the screen position of the tile
    unsigned char tileWidth = data->tileBase.width;
    unsigned char tileHeight = data->tileBase.height;

    // Calculate the character position of the tile (and ensure it is onscreen)
    *col = dX * tileWidth;
    *row = dY * tileHeight;
    if(*col >= data->dispData.screenCols || *row >= data->dispData.screenRows) {
        return -1;
    }

    return 0;
}

/**
 * Returns the coordinates of the upper-left visible tile of the given map and 
 * tile data
 * 
 * @param data The tile data struct used in display
 * @param map The map used in display
 * @param x The x coordinate of the focussed tile
 * @param y The y coordinate of the focussed tile
 * 
 * @param scrX A return pointer for the x of the upper-left tile of the screen
 * @param scrY A return pointer for the y of the upper-left tile of the screen
 */
void getScreenXY(tileData_t data, map_t map, int x, int y, int* scrX, int*scrY) {
    int width, height;
    getScreenTileDim(data, &width, &height);

    *scrX = max(min(x - width/2, map.nCols-width), 0);
    *scrY = max(min(y - height/2, map.nRows-height), 0);
}

//==============================<Sprite Display>==============================//
void addSprite(dispData_t * data, sprite_t sprite, short palette, int screenRow, int screenCol) {
    if(data == NULL || data->data == NULL || sprite.data == NULL) return;
    if(palette == 0) palette = sprite.defPalette; 

    // Ensure the sprite is not entirely off screen
    screenRow = screenRow + sprite.yOff;
    screenCol = screenCol + sprite.xOff;
    if (screenRow + sprite.height <= 0 || screenCol + sprite.width <= 0 ||
            screenRow >= data->screenRows || screenCol >= data->screenCols) {
        return;
    }

    // Buffer each character from the sprite
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

void addTileBase(tileData_t * data, tile_t tile, int scrX, int scrY, int x, int y) {
    if(data == NULL) return;

    // Get the row and column of the referenced tile
    int row, col;
    if(getScreenRowCol(data, scrX, scrY, x, y, &col, &row) != 0) {
        return;
    }

    // If the tile is empty, draw the empty tile and be done with it
    if(tile.isEmpty) {
        addSprite(&data->dispData, data->emptyBase, 0, row, col);
        return;
    }

    // Determine correct palette (override or tile), and buffer sprite
    short palette = (tile.bgOverride != 0) ? tile.bgOverride : tile.bgPalette;
    addSprite(&data->dispData, data->tileBase, palette, row, col);
}

#define getWallSprite(dir)\
    switch(tile.dir##Wall) {\
        case 0:\
            break;\
        case 1: \
            sprite = data->dir##Wall;\
            break;\
        default:\
            sprite = data->dir##Door;\
    }

void addTileWalls(tileData_t * data, tile_t tile, int scrX, int scrY, int x, int y) {
    if(data == NULL) return;
    
    // Get the row and column of the referenced tile
    int row, col;
    if(getScreenRowCol(data, scrX, scrY, x, y, &col, &row) != 0) {
        return;
    }

    sprite_t sprite = kEmptySprite;
    getWallSprite(l);
    addSprite(&data->dispData, sprite, 0, row, col);
    getWallSprite(r);
    addSprite(&data->dispData, sprite, 0, row, col);
    getWallSprite(u);
    addSprite(&data->dispData, sprite, 0, row, col);
    getWallSprite(d);
    addSprite(&data->dispData, sprite, 0, row, col);

}

void addActorSprite(tileData_t * data, list_t actorSprites, actor_t actor, int scrX, int scrY) {
    if(data == NULL) {
        return;
    }

    // Get the proper sprite
    sprite_t sprite;
    if(actor.spriteIdx < 0) {           // Get the default sprite
        // TODO: Handle the default sprite
        return; 
    } else if(actorSprites == NULL ||   // Get the sprite from list
            (unsigned int) actor.spriteIdx >= listLen(actorSprites)) {
        sprite = *(sprite_t*) listGet(actorSprites, actor.spriteIdx);
    } else {                            // Illegal sprite idx, just return
        return;
    }

    // Get the row and column of the actor's tile
    int row, col;
    if(getScreenRowCol(data, scrX, scrY, actor.x, actor.y, &col, &row) != 0) {
        return;
    }

    // Add in the sprite itself with the correct palette
    short palette = (actor.paletteOverride != 0) ? actor.paletteOverride : actor.palette;
    addSprite(&data->dispData, sprite, palette, row, col);

}

void addTileSprite(tileData_t * data, tile_t tile, int scrX, int scrY, int x, int y) {
    if(data == NULL) return;
    
    // Ensure that a sprite is specified (and that the list contains it)
    if(tile.sprite == kNoSprite || (tile.sprite >= 0 && (data->spriteList == NULL || 
            (unsigned int) tile.sprite >= listLen(data->spriteList)))) {
        return;
    }
    
    // Get the row and column of the referenced tile
    int row, col;
    if(getScreenRowCol(data, scrX, scrY, x, y, &col, &row) != 0) {
        return;
    }

    // Get the sprite to draw
    sprite_t sprite;
    if(tile.sprite < 0) {   // Decode the character sprite
        if(data->charSprite.data == NULL) return;

        sprite = data->charSprite;
        
        char ch = getCharSpriteChar(tile.sprite);
        if(ch == '\0') {
            ch = ' ';
        }
        
        short palette = getCharSpritePalette(tile.sprite);
        if (palette<kMinPalette || palette>kMaxPalette) palette = kDefPalette;

        sprite.data[1][1] = ch;
        sprite.defPalette = palette;

        sprite = data->charSprite;
    } else {    // Get the proper list sprite
        sprite = *(sprite_t *) listGet(data->spriteList, tile.sprite);
    }

    // Determine correct palette (override or tile), and buffer sprite
    short palette = (tile.spriteOverride != 0) ? tile.spriteOverride : tile.spritePalette;
    addSprite(&data->dispData, sprite, palette, row, col);
}

void getScreenTileDim(tileData_t data, int * width, int * height) {
    if(width != NULL) *width = data.dispData.screenCols / data.emptyBase.width;
    if(height != NULL) *height = data.dispData.screenRows / data.emptyBase.height;
}

//===============================<Map Display>================================//

int addMap(tileData_t * data, map_t map, int x, int y) {
    // Determine the position of the screen
    int width, height;  // Width and height of the screen in tiles
    int scrX, scrY;     // The X & Y coords of the top-left tile 
    getScreenTileDim(*data, &width, &height);
    getScreenXY(*data, map, x, y, &scrX, &scrY);

    // Draw all tiles on the screen
    for(int dRow = 0; dRow < height && dRow + scrY < map.nRows; dRow++) {
        int row = dRow + scrY;
        for(int dCol = 0; dCol < width && dCol + scrX < map.nCols; dCol++) {
            int col = dCol + scrX;
            addTileBase(data, map.data[row][col], scrX, scrY, col, row);
        }
    }

    // And then draw all of the walls
    for(int dRow = 0; dRow < height && dRow + scrY < map.nRows; dRow++) {
        int row = dRow + scrY;
        for(int dCol = 0; dCol < width && dCol + scrX < map.nCols; dCol++) {
            int col = dCol + scrX;
            addTileWalls(data, map.data[row][col], scrX, scrY, col, row);
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

//todo Consider replacing this with a general buffer to file function

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
                        sprite_t * sprite = NULL;
                        if(spriteNr < 0 && data.charSprite.data != NULL) {
                            // Set the active sprite to the character sprite
                            sprite = &data.charSprite;
                            sprite->data[1][1] = getCharSpriteChar(spriteNr);
                        } else {
                            sprite = listGet(data.spriteList, spriteNr);
                        }

                        // If you got a sprite, try to print its next character
                        if(sprite != NULL && line >= sprite->yOff && line < sprite->yOff + sprite->height &&
                                i >= sprite->xOff && i < sprite->xOff + sprite->width) {
                                int y = line-sprite->yOff, x = i-sprite->xOff;

                                char ch = sprite->data[y][x];
                                mapFileNextChar(file, (ch == 0) ? ' ' : ch);

                                continue;
                        }

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

//==============================<Actor Display>===============================//

void addActorsHelper(tileData_t* data, list_t actorList, list_t actorSprites, 
                    int scrX, int scrY, int maxX, int maxY) {

    unsigned int len = listLen(actorList);
    for(unsigned int i = 0; i < len; ++i) {
        actor_t actor = *(actor_t*) listGet(actorList, i);
        
        if(actor.x < scrX || actor.x >= maxX || actor.y < scrY || actor.y >= maxY) {
            continue;
        }

        addActorSprite(data, actorSprites, actor, scrX, scrY);
    }
}

int addActors(tileData_t* data, map_t map, actorData_t actors, int x, int y) {
    if(data == NULL || map.data == NULL) {
        return -1;
    }
    // Determine the position of the screen
    int width, height;  // Width and height of the screen in tiles
    int scrX, scrY;     // The X & Y coords of the top-left tile
    int maxX, maxY;     // The max X and Y coords on screen
    getScreenTileDim(*data, &width, &height);
    getScreenXY(*data, map, x, y, &scrX, &scrY);
    maxX = min(map.nCols, scrX + width);
    maxY = min(map.nRows, scrY + height);

    // Try to put each actor on screen
    addActorsHelper(data, actors.npcActors, actors.actorSprites, scrX, scrY, maxX, maxY);
    addActorsHelper(data, actors.enemyActors, actors.actorSprites, scrX, scrY, maxX, maxY);
    addActorsHelper(data, actors.pcActors, actors.actorSprites, scrX, scrY, maxX, maxY);

    return 0;
}
