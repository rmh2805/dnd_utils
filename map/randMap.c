#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "map.h"

//===========================<Default Definitions>============================//
// Define Default maze configuration
#define kDefDeadEndRooms 0
#define kDefMidRooms 0
#define kDefRows 64
#define kDefCols 64

// Define endpoint room sizes
#define kDefFirstRoomRows 1
#define kDefFirstRoomCols 1
#define kDefFinalRoomRows 6
#define kDefFinalRoomCols 6

// Define generated room sizes
#define kDefDeadEndMinDim 2
#define kDefDeadEndMaxDim 4
#define kDefMidMinDim 4
#define kDefMidMaxDim 6

// Define misc generation constants
#define kDefOverlapReries 3 // Number of times to retry on overlapping rooms
#define kDefOOBRetries 10 // Number of times to retry on OOB rooms

//==============================<Argument Flags>==============================//
#define kFinalDimFlag "-f"
#define kStartDimFlag "-s"
#define kMazeDimFlag "-d"
#define kMidRoomsFlag "-m"
#define kDeadEndsFlag "-e"
#define kUsageFlag "-?"

//===============================<Global State>===============================//
char * outFileLoc;

int deadEnds, midRooms;

int mazeRows, mazeCols;
int firstRoomRows, firstRoomCols;
int finalRoomRows, finalRoomCols;

int deadEndMinDim, deadEndMaxDim;
int midMinDim, midMaxDim;

int overlapRetries, oobRetries;

//=============================<Room Definitions>=============================//
typedef struct room_s {
    int x;
    int y;
    int width;
    int height;
} room_t;

/**
 * Checks if a room overlaps any already non-empty regions on the map
 * 
 * @param map The map to check against
 * @param room The room to check
 * 
 * @return true iff room overlaps some non-empty tile in map
 */
bool roomOverlaps(map_t map, room_t room){
    if(map.data == NULL) return true;

    for(int dRow = 0; dRow < room.height; dRow++) {
        int row = room.y + dRow;
        if(row < 0) continue;
        if(row >= map.nRows) break;

        for(int dCol = 0; dCol < room.width; dCol++) {
            int col = room.x + dCol;
            if(col < 0) continue;
            if(row >= map.nCols) break;

            if(!map.data[row][col].isEmpty) {
                return true;
            }
        }
    }

    return false;
}

// A macro defining a corner comparison in the roomsOverlap function
#define checkCornerRoomsOverlap(tb, lr, other) (tb##1 >= top##other && tb##1 <= bottom##other && lr##1 >= left##other && lr##1 <= right##other)

/**
 * Checks if 2 rooms would overlap each other
 * 
 * @param one The first room to compare
 * @param two The second room to compare
 * 
 * @return true iff the two rooms overlap
 */
bool roomsOverlap(room_t one, room_t two) {
    // Define corner coordinates
    int top1 = one.y, bottom1 = one.y + one.height - 1;
    int left1 = one.x, right1 = one.x + one.width - 1;
    int top2 = two.y, bottom2 = two.y + two.height - 1;
    int left2 = two.x, right2 = two.x + two.width - 1;


    // Check if 1 has a corner in 2
    if(checkCornerRoomsOverlap(top, left, 2)) {
        return true;
    }
    if(checkCornerRoomsOverlap(bottom, left, 2)) {
        return true;
    }if(checkCornerRoomsOverlap(top, right, 2)) {
        return true;
    }
    if(checkCornerRoomsOverlap(bottom, right, 2)) {
        return true;
    }

    // Check if 2 has a corner in 1
    if(checkCornerRoomsOverlap(top, left, 1)) {
        return true;
    }
    if(checkCornerRoomsOverlap(bottom, left, 1)) {
        return true;
    }if(checkCornerRoomsOverlap(top, right, 1)) {
        return true;
    }
    if(checkCornerRoomsOverlap(bottom, right, 1)) {
        return true;
    }

    // No corners overlap, so the rooms don't either
    return false;
}

/**
 * Returns true iff the provided room would go out of bounds on the provided map
 * 
 * @param map The map to check in
 * @param room The room to check
 * 
 * @return true iff room goes OOB
 */
bool isOutOfBounds(map_t map, room_t room) {
    return room.x < 0 || room.y < 0 || room.x + room.width > map.nCols || 
        room.y + room.height > map.nRows;
}

/**
 * Places a room on the provided map
 * 
 * @param map The map to place in
 * @param room The room to place
 */
void placeRoom(map_t * map, room_t room) {
    if(map == NULL || map->data == NULL) {
        return;
    }

    for(int dRow = 0; dRow < room.height; dRow++) {
        int row = room.y + dRow;
        if(row < 0) continue;
        if(row >= map->nRows) break;

        for(int dCol = 0; dCol < room.width; dCol++) {
            int col = room.x + dCol;
            if(col < 0) continue;
            if(col >= map->nCols) break;

            if(!map->data[row][col].isEmpty) {
                // On a present cell, make sure all walls are properley set for 
                // the new room
                if(dRow != 0) {
                    map->data[row][col].uWall = 0;
                }
                if(dRow != room.height - 1) {
                    map->data[row][col].dWall = 0;
                }
                if(dCol != 0) {
                    map->data[row][col].lWall = 0;
                }
                if(dCol != room.width - 1) {
                    map->data[row][col].rWall = 0;
                }
                continue;
            } else {
                // On an empty cell, mark cell as filled
                map->data[row][col].isEmpty = false;

                // Set any walls if this is on a room edge
                if(dRow == 0) {
                    map->data[row][col].uWall = 1;
                }
                if(dRow == room.height - 1) {
                    if(row < map->nRows - 1) {
                        map->data[row+1][col].uWall = 1;
                    } else {
                        map->data[row][col].dWall = 1;
                    }
                }
                if(dCol == 0) {
                    map->data[row][col].lWall = 1;
                }
                if(dCol == room.width - 1) {
                    if(col < map->nCols - 1) {
                        map->data[row][col+1].lWall = 1;
                    } else {
                        map->data[row][col].rWall = 1;
                    }
                }
            }
        }
    }
}

//=============================<Helper Functions>=============================//
/**
 * Prints a basic usage message
 * 
 * @param call The name of this executable (i.e. argv[0])
 */
void printUsage(const char * call) {
    printf("Usage: %s [%s <finalRows> <finalCols>] [%s <startRows> <startCols>]"
        " [%s <mazeRows> <mazeCols>] [%s <midRooms>] [%s <deadEnds>] "
        "<Output File>\n\n", call, kFinalDimFlag, kStartDimFlag, kMazeDimFlag, 
        kMidRoomsFlag, kDeadEndsFlag);
}

/**
 * Generates a random int in a range
 * 
 * @param mini The minimum nr to generate (inclusive)
 * @param maxi The maximum nr to generate (inclusive)
 * 
 * @return The generated number
 */
int randRange(int mini, int maxi) {
    int r = rand();
    int range = maxi-mini;
    return mini + (r % (range + 1));
}

//================================<Main Code>=================================//
int main(int argc, char** argv) {
    //============================<Parse Args>============================//
    // Check arg count
    if(argc < 2) {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    // Get the required output file location
    outFileLoc = argv[argc-1];
    if(strcmp(kUsageFlag, outFileLoc) == 0) {
        // Check for simple usage check
        printUsage(argv[0]);
        return EXIT_SUCCESS;
    }

    // Set other default values
    deadEnds = kDefDeadEndRooms;
    midRooms = kDefMidRooms;

    mazeRows = kDefRows;
    mazeCols = kDefCols;
    firstRoomRows = kDefFirstRoomRows;
    firstRoomCols = kDefFirstRoomCols;
    finalRoomRows = kDefFinalRoomRows;
    finalRoomCols = kDefFinalRoomCols;

    deadEndMinDim = kDefDeadEndMinDim;
    deadEndMaxDim = kDefDeadEndMaxDim;
    midMinDim = kDefMidMinDim;
    midMaxDim = kDefMidMaxDim;

    overlapRetries = kDefOverlapReries;
    oobRetries = kDefOOBRetries;

    // Check for other flags
    for(int i = 1; i < argc - 1; i++) {
        if(strcmp(kUsageFlag, argv[i]) == 0) {
            // Usage requested
            printUsage(argv[0]);
            return EXIT_SUCCESS;
        } else if(strcmp(kFinalDimFlag, argv[i]) == 0) {
            // Trying to set final room dimensions
            if(i+2 >= argc) {
                fprintf(stderr, "*FATAL ERROR* in main: Final room dimensions not specified\n");
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
            if(sscanf(argv[++i], "%d", &finalRoomRows) != 1 || finalRoomRows <= 0) {
                fprintf(stderr, "*FATAL ERROR* in main: Invalid final room dimension \"%s\"\n", argv[i]);
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
            if(sscanf(argv[++i], "%d", &finalRoomCols) != 1 || finalRoomCols <= 0) {
                fprintf(stderr, "*FATAL ERROR* in main: Invalid final room dimension \"%s\"\n", argv[i]);
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
        } else if(strcmp(kStartDimFlag, argv[i]) == 0) {
            // Trying to set first room dimensions
            if(i+2 >= argc) {
                fprintf(stderr, "*FATAL ERROR* in main: First room dimensions not specified\n");
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
            if(sscanf(argv[++i], "%d", &firstRoomRows) != 1 || firstRoomRows <= 0) {
                fprintf(stderr, "*FATAL ERROR* in main: Invalid first room dimension \"%s\"\n", argv[i]);
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
            if(sscanf(argv[++i], "%d", &firstRoomCols) != 1 || firstRoomCols <= 0) {
                fprintf(stderr, "*FATAL ERROR* in main: Invalid first room dimension \"%s\"\n", argv[i]);
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
        } else if(strcmp(kMazeDimFlag, argv[i]) == 0) {
            // Trying to set first room dimensions
            if(i+2 >= argc) {
                fprintf(stderr, "*FATAL ERROR* in main: Maze dimensions not specified\n");
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
            if(sscanf(argv[++i], "%d", &mazeRows) != 1 || firstRoomRows <= 0) {
                fprintf(stderr, "*FATAL ERROR* in main: Invalid first room dimension \"%s\"\n", argv[i]);
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
            if(sscanf(argv[++i], "%d", &firstRoomCols) != 1 || firstRoomCols <= 0) {
                fprintf(stderr, "*FATAL ERROR* in main: Invalid first room dimension \"%s\"\n", argv[i]);
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
        } else if(strcmp(kMidRoomsFlag, argv[i]) == 0) {
            // Trying to set first room dimensions
            if(i+1 >= argc) {
                fprintf(stderr, "*FATAL ERROR* in main: Mid room count unspecified\n");
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
            if(sscanf(argv[++i], "%d", &midRooms) != 1 || midRooms < 0) {
                fprintf(stderr, "*FATAL ERROR* in main: Invalid mid room cound \"%s\"\n", argv[i]);
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
        } else if(strcmp(kDeadEndsFlag, argv[i]) == 0) {
            // Trying to set first room dimensions
            if(i+1 >= argc) {
                fprintf(stderr, "*FATAL ERROR* in main: Dead End count unspecified\n");
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
            if(sscanf(argv[++i], "%d", &deadEnds) != 1 || midRooms < 0) {
                fprintf(stderr, "*FATAL ERROR* in main: Invalid dead end count \"%s\"\n", argv[i]);
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
        } else {
            fprintf(stderr, "*FATAL ERROR* in main: Unknown flag \"%s\"\n", argv[i]);
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    //============================<Main Code>=============================//
    printf("Out File: %s\n", outFileLoc);
    printf("Dead Ends: %d\n", deadEnds);
    printf("Mid Rooms: %d\n", midRooms);
    printf("First Room Dim: %d x %d\n", firstRoomRows, firstRoomCols);
    printf("Final Room Dim: %d x %d\n", finalRoomRows, finalRoomCols);
    printf("Dead end size range: [%d, %d]\n", deadEndMinDim, deadEndMaxDim);
    printf("Mid room size range: [%d, %d]\n", midMinDim, midMaxDim);

    //=============================<Cleanup>==============================//
    return EXIT_SUCCESS;
}