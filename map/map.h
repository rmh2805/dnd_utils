#ifndef _MAP_H_
#define _MAP_H_

#include <stdlib.h>

#include "tile.h"

//=============================<Type Definitions>=============================//
typedef struct map_s {
    tile_t ** data;
    int nRows, nCols;
} map_t;


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
int mkMap(int nRows, int nCols, map_t * map);

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
int initMap(tile_t ** data, int nRows, int nCols, map_t * map);

/**
 * Cleans up the sprites in the map without freeing its tile buffer
 * 
 * @param map The map to clean
 */
void rmMap(map_t map);

#endif