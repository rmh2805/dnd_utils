#include "map.h"


//===========================<Helper Declarations>============================//
void freeTiles(tile_t** tiles, int nRows);

#ifndef min
#define min(a, b) ((a < b) ? a : b)
#endif

#ifndef max
#define max(a, b) ((a > b) ? a : b)
#endif

//============================<Memory Management>=============================//

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

int initMap(tile_t ** data, int nRows, int nCols, map_t * map) {
    *map = (map_t) {data, nRows, nCols};

    for(int row = 0; row < nRows; row++) {
        for(int col = 0; col < nCols; col++) {
            data[row][col] = mkEmptyTile(col, row);
        }
    }

    return 0;
}

void rmMap(map_t map) {
    freeTiles(map.data, map.nRows);
}

void freeTiles(tile_t** tiles, int nRows) {
    if(tiles == NULL) return;

    for(int i = 0; i < nRows; i++) {
        if(tiles[i] != NULL) free(tiles[i]);
    }

    free(tiles);
}

//==============================<Serialization>===============================//

int writeMap(map_t map, list_t sprites, FILE* fp) {
    if(fp == NULL) return -1;

    fprintf(fp, "%d %d\n", map.nRows, map.nCols);

    int len = 0;
    if(sprites != NULL) {
        len = listLen(sprites);
    }

    for(int row = 0; row < map.nRows; row++) {
        for(int col = 0; col < map.nCols; col++) {
            tile_t tile = map.data[row][col];
            if(tile.sprite >= len) {    // Cull any illegal sprites
                tile.sprite = kNoSprite;
            }

            if(writeTile(tile, fp) < 0) return -2;
        }
    }

    if(sprites != NULL) {
        if(saveSpriteList(fp, sprites) < 0) return -2;
    }
    
    return 0;
}

int loadMap(map_t* map, list_t * sprites, FILE* fp) {
    if(fp == NULL || map == NULL || sprites == NULL) return -1;

    int rows, cols;
    int ret = fscanf(fp, "%d %d", &rows, &cols);

    if(ret != 2) return -2;

    if(mkMap(rows, cols, map) < 0) {
        return -3;
    }

    if((*sprites = mkList()) == NULL) {
        rmMap(*map);
        return -3;
    }

    for(int row = 0; row < rows; row++) {
        for(int col = 0; col < cols; col++) {
            ret = readTile(&map->data[row][col], fp);
            if(ret < 0) {
                rmMap(*map);
                rmList(*sprites, freeSpriteEntry);
                *sprites = NULL;
                return -2;
            }
        }
    }

    if(loadSpriteList(fp, sprites) < 0) {
        rmMap(*map);
        rmList(*sprites, freeSpriteEntry);
        *sprites = NULL;
        return -2;
    }

    return 0;
}