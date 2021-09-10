#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <ncurses.h>

#include "dispBase.h"
#include "sprite.h"
#include "tile.h"
#include "map.h"


#define kTileFile "walls.out"

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: %s <map file>\n\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    bool doPages = argc > 2;

    tileData_t data;
    
    // Load in the tile file
    FILE* fp = fopen(kTileFile, "r");
    if(fp == NULL) {
        fprintf(stderr, "*FATAL ERROR* Failed to load tile data file\n");
        return EXIT_FAILURE;
    }

    if(loadTileData(fp, &data)) {
        fprintf(stderr, "*FATAL ERROR* Failed to read tile data from file\n");
        return EXIT_FAILURE;
    }
    fclose(fp);

    // Load in the map
    fp = fopen(argv[1], "r");
    if(fp == NULL) {
        fprintf(stderr, "*FATAL ERROR* Failed to open map file\n");
        rmTileData(data);
        return EXIT_FAILURE;
    }

    map_t map;
    if(loadMap(&map, fp) != 0) {
        fprintf(stderr, "*FATAL ERROR* Failed to load map from file\n");
        rmTileData(data);
        return EXIT_FAILURE;
    }
    fclose(fp);

    // Print the map to stdout
    if(doPages) {
        mapToSections(data, map, stdout, 80, 64, true);
    } else {
        mapToFile(data, map, stdout);
    }

    // Cleanup
    rmTileData(data);
    rmMap(map);
    return EXIT_SUCCESS;
}