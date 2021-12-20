#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <ncurses.h>
#include "mapDisp.h"

#include "sprite.h"
#include "tile.h"
#include "map.h"

int main(int argc, char** argv) {
    //============================<Core State>============================//
    int status = EXIT_FAILURE;
    
    FILE* fp = NULL;
    bool fileOpen = false;

    tileData_t data;
    bool tilesLoaded = false;

    map_t map;
    bool mapLoaded = false;

    //============================<Main Code>=============================//
    if(argc < 2) {
        printf("Usage: %s <map file>\n\n", argv[0]);
        goto main_cleanup;
    }
    
    bool doPages = argc > 2;

    
    // Load in the tile data
    if(loadTileData(&data)) {
        fprintf(stderr, "*FATAL ERROR* Failed to read tile data from file\n");
        goto main_cleanup;
    }
    tilesLoaded = true;

    // Load in the map
    fp = fopen(argv[1], "r");
    if(fp == NULL) {
        fprintf(stderr, "*FATAL ERROR* Failed to open map file\n");
        goto main_cleanup;
    }
    fileOpen = true;

    if(loadMap(&map, fp) != 0) {
        fprintf(stderr, "*FATAL ERROR* Failed to load map from file\n");
        goto main_cleanup;
    }
    fclose(fp);
    fileOpen = false;
    mapLoaded = true;
    

    // Print the map to stdout
    if(doPages) {
        mapToSections(data, map, stdout, 80, 64, true);
    } else {
        mapToFile(data, map, stdout);
    }

    status = EXIT_SUCCESS;
main_cleanup:
    // Cleanup
    if(fileOpen) fclose(fp);
    if(tilesLoaded) rmTileData(data);
    if(mapLoaded) rmMap(map);
    return status;
}