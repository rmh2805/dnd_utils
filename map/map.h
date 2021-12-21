#ifndef _MAP_H_
#define _MAP_H_

#include <stdlib.h>

#include "tile.h"

//=============================<Type Definitions>=============================//
typedef struct map_s {
    tile_t ** data;
    int nRows, nCols;
} map_t;


//=========================<Initialization & Cleanup>=========================//
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
 * Frees the tile buffers allocated by mkMap
 * 
 * @param map The map to clean
 */
void rmMap(map_t map);

//==============================<Serialization>===============================//

/**
 * Write a map out to file
 * 
 * @param map The map to write to file
 * @param sprites The sprite list used with this map
 * @param fp The file to write out to
 * 
 * @return 0 on success, < 0 on failure
 */
int writeMap(map_t map, list_t sprites, FILE* fp);

/**
 * Load a map from a file
 * 
 * @param map A return pointer for the map read from file
 * @param sprites A return pointer for the sprite list used in this map (destructive)
 * @param fp The file to read from
 * 
 * @return 0 on success, < 0 on failure
 *          -1 on null param, 
 *          -2 on failure to read from file,
 *          -3 if unable to allocate the new map or sprite list
 */
int loadMap(map_t* map, list_t * sprites, FILE* fp);

#endif