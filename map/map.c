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
            if(ret < 0) return -2;
        }
    } 

    return 0;
}