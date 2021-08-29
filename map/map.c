#include "map.h"


//===========================<Helper Declarations>============================//
void freeTiles(tile_t** tiles, int nRows);

//============================<Memory Management>=============================//
/**
 * Allocates and initializes a map with the provided dimensions
 * 
 * @param nRows The number of rows to include in the map
 * @param nCols The number of columns to include in the map
 * @param map A return pointer for the map
 * 
 * @return 0 on success, < 0 on failure
 */
int mkMap(int nRows, int nCols, map_t * map) {
    tile_t ** tiles;

    tiles = calloc(nRows, sizeof(tile_t *));
    if(tiles == NULL) return -1;

    for(int i = 0; i < nRows; i++) {
        tiles[i] = calloc(nCols, sizeof(tile_t));
        if(tiles[i] == NULL) {
            freeTiles(tiles, nRows);
            return -1;
        }
    }

    return initMap(tiles, nRows, nCols, map);
}

/**
 * Initializes a map in the provided tile buffer with the provided dimensions
 * 
 * @param data The pre-allocated tile buffer
 * @param nRows The number of rows in the tile buffer
 * @param nCols The number of columns in the tile buffer
 * @param map A return pointer for the initialized map
 * 
 * @return 0 on success, < 0 on failure
 */
int initMap(tile_t ** data, int nRows, int nCols, map_t * map) {
    *map = (map_t) {data, nRows, nCols};

    for(int row = 0; row < nRows; row++) {
        for(int col = 0; col < nCols; col++) {
            data[row][col] = mkEmptyTile(col, row);
        }
    }

    return 0;
}

/**
 * Cleans up the entries within the map and then frees its tile buffer
 * 
 * @param map The map to clean and free
 */
void rmMap(map_t map) {
    freeTiles(map.data, map.nRows);
}

/**
 * Frees the tile buffer of the provided map
 * 
 * @param map The map to free from
 */
void freeTiles(tile_t** tiles, int nRows) {
    if(tiles == NULL) return;

    for(int i = 0; i < nRows; i++) {
        if(tiles[i] != NULL) free(tiles[i]);
    }

    free(tiles);
}

//==============================<Serialization>===============================//

/**
 * Write a map out to file
 * 
 * @param map The map to write to file
 * @param fp The file to write out to
 * 
 * @return 0 on success, < 0 on failure
 */
int writeMap(map_t map, FILE* fp) {
    if(fp == NULL) return -1;

    fprintf(fp, "%d %d\n", map.nRows, map.nCols);

    for(int row = 0; row < map.nRows; row++) {
        for(int col = 0; col < map.nCols; col++) {
            int ret = writeTile(map.data[row][col], fp);
            if(ret < 0) return -2;
        }
    }
    
    return 0;
}

/**
 * Load a map from a file
 * 
 * @param map A return pointer for the map read from file
 * @param fp The file to read from
 * 
 * @return 0 on success, < 0 on failure
 *          -1 on null param, 
 *          -2 on failure to read from file,
 *          -3 if unable to allocate the new map
 */
int loadMap(map_t* map, FILE* fp) {
    if(fp == NULL || map == NULL) return -1;

    int rows, cols;
    int ret = fscanf(fp, "%d %d", &rows, &cols);

    if(ret != 2) return -2;

    if(mkMap(rows, cols, map) < 0) return -3;

    for(int row = 0; row < rows; row++) {
        for(int col = 0; col < cols; col++) {
            ret = readTile(&map->data[row][col], fp);
            if(ret < 0) {
                rmMap(*map);
                return -2;
            }
        }
    } 

    return 0;
}

#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a > b) ? a : b)

/**
 * Draw the section of the map in view, with focus on the tile at position (x,y)
 * 
 * @param data The tile data struct to use for display
 * @param map The map to display
 * @param x The x coordinate of the selected tile
 * @param y The y coordinate of the selected cell
 * 
 * @return 0 on success, <0 on failure
 */
int drawMap(tileData_t data, map_t map, int x, int y) {
    // Determine the position of the screen
    int width, height;  // Width and height of the screen in tiles
    getScreenTileDim(data, &width, &height);

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
            drawTile(data, map.data[row][col], scrX, scrY, col, row);
        }
    }

    // And then draw all of the walls
    for(int dRow = 0; dRow < height && dRow + scrY < map.nRows; dRow++) {
        int row = dRow + scrY;
        for(int dCol = 0; dCol < width && dCol + scrX < map.nCols; dCol++) {
            int col = dCol + scrX;
            drawWalls(data, map.data[row][col], scrX, scrY, col, row);
        }
    }


    // Retarget the selected tile
    move(dY * data.emptyBase.height + data.emptyBase.height/2, 
            dX * data.emptyBase.width + data.emptyBase.width/2);

    return 0;
}


#define printError(msg) clear();printText(kRedPalette, msg, 0, 0); getch()

int mapSectionToFile(tileData_t data, map_t map, FILE* file, 
        int startRow, int startCol, int endRow, int endCol);

int mapToFile(tileData_t data, map_t map, FILE* file) {
    return mapSectionToFile(data, map, file, 0, 0, 0, 0);
}

int mapToSections(tileData_t data, map_t map, FILE* file, int pgWidth, int pgHeight) {
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
                    (pgRank+1) * pgRows, (pgFile+1) * pgCols); 
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

int mapSectionToFile(tileData_t data, map_t map, FILE* file, 
        int startRow, int startCol, int endRow, int endCol) {
    if(file == NULL) return -1;
    if(startRow > endRow || startCol > endCol) return -2;
    if(startRow > map.nRows || startCol > map.nCols) return 1;

    if(endRow == 0 || endRow > map.nRows) endRow = map.nRows;
    if(endCol == 0 || endCol > map.nCols) endCol = map.nCols;

    for(int row = startRow; row < endRow; row++) {
        for(int line = 0; line < data.emptyBase.height; line++) {
            for(int col = startCol; col < endCol; col++) {
                for(int i = 0; i < data.emptyBase.width; i++) {
                    int ch = ' ';
                    if(map.data[row][col].uWall == 1 && line < data.uWall.height && data.uWall.data[line][i]) {
                        // Check for characters in the upper wall
                        ch = data.uWall.data[line][i];
                    } else if(map.data[row][col].uWall == 2 && line < data.uDoor.height && data.uDoor.data[line][i]) {
                        // Check for characters in the upper door
                        ch = data.uDoor.data[line][i];
                    } else if(map.data[row][col].lWall == 1 && i < data.lWall.width && data.lWall.data[line][i]) {
                        // Check for characters in the left wall
                        ch = data.lWall.data[line][i];
                    } else if(map.data[row][col].lWall == 2 && i < data.lDoor.width && data.lDoor.data[line][i]) {
                        // Check for characters in the left door
                        ch = data.lDoor.data[line][i];
                    } else if(map.data[row][col].dWall == 1 && line >= data.emptyBase.height - data.dWall.height 
                                && data.dWall.data[line - (data.emptyBase.height - data.dWall.height)][i]) {
                        // Check for characters in the lower wall
                        ch = data.dWall.data[line - (data.emptyBase.height - data.dWall.height)][i];
                    } else if(map.data[row][col].dWall == 2 && line >= data.emptyBase.height - data.dDoor.height 
                                && data.dDoor.data[line - (data.emptyBase.height - data.dDoor.height)][i]) {
                        // Check for characters in the lower door
                        ch = data.dDoor.data[line - (data.emptyBase.height - data.dDoor.height)][i];
                    } else if(map.data[row][col].rWall == 1 && i >= data.emptyBase.width - data.rWall.width 
                                && data.rWall.data[line][i - (data.emptyBase.width - data.rWall.width)]) {
                        // Check for characters in the right wall
                        ch = data.rWall.data[line][i - (data.emptyBase.width - data.rWall.width)];
                    } else if(map.data[row][col].rWall == 2 && i >= data.emptyBase.width - data.rDoor.width 
                                && data.rDoor.data[line][i - (data.emptyBase.width - data.rDoor.width)]) {
                        // Check for characters in the right wall
                        ch = data.rDoor.data[line][i - (data.emptyBase.width - data.rDoor.width)];
                    } else if(!map.data[row][col].isEmpty && data.tileBase.data[line][i]) {
                        // Check for characters in the base tile
                        ch = data.tileBase.data[line][i];
                    }
                    
                    fprintf(file, "%c", ch);

                }
            }
            fprintf(file, "\n");
        }
    }

    return 0;
}