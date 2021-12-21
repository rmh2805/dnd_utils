#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "map.h"
#include "../common/list.h"

//===========================<Default Definitions>============================//
// Define Default maze configuration
#define kDefDeadEndRooms 3
#define kDefMidRooms 5

#define kDefRows 8
#define kDefCols 20

// Define endpoint room sizes
#define kDefFirstRoomRows 1
#define kDefFirstRoomCols 1

#define kDefFinalRoomRows 1
#define kDefFinalRoomCols 1

// Define generated room sizes
#define kDefDeadEndMinDim 2
#define kDefDeadEndMaxDim 4

#define kDefMidMinDim 2
#define kDefMidMaxDim 4

// Define misc generation constants
#define kDefOverlapReries 10000 //Number of times to retry on overlapping rooms
#define kDefOOBRetries 10000   // Number of times to retry on OOB rooms

#define kMidSquarePrecent 90   // Chance that mid rooms will be forced squares
#define kEndSquarePercent 30    // Chance that dead ends will be forced squares

//==============================<Argument Flags>==============================//
#define kFinalDimFlag "-f"
#define kStartDimFlag "-s"
#define kMazeDimFlag "-d"
#define kMidRoomsFlag "-m"
#define kMidRoomDimFlag "-M"
#define kDeadEndsFlag "-e"
#define kDeadEndDimFlag "-E"
#define kOverlapRetriesFlag "-r"
#define kOOBRetriesFlag "-o"
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

//===========================<Helper Declarations>============================//
void printUsage(const char * call);
int randRange(int mini, int maxi);

#define min(a, b) ((b < a) ? b : a)
#define max(a, b) ((b > a) ? b : a)

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
 * @param mergeOverlap If true, merge rooms on overlap. If false, new rooms are 
 *  placed "below" existing rooms
 */
void placeRoom(map_t * map, room_t room, bool mergeOverlap) {
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
                if(!mergeOverlap) continue;

                // On a present cell, make sure all walls are properley set for 
                // the new room in order to merge the two
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

/**
 * Generaes a copy of the original room with dimensions reversed
 * 
 * @param room The root room
 * 
 * @return The rotated room
 */
room_t rotRoom(room_t room) {
    return (room_t) {room.x, room.y, room.height, room.width};
}

/**
 * Returns a omly generated room
 * 
 * @param isDeadEnd Set true if generating the room as a dead end
 * 
 * @return The generated room
 */
room_t mkRandRoom(bool isDeadEnd) {
    room_t room;
    room.width = randRange((isDeadEnd) ? deadEndMinDim : midMinDim,
                            (isDeadEnd) ? deadEndMaxDim : midMaxDim);
    if(randRange(0, 100) <= ((isDeadEnd) ? kEndSquarePercent : kMidSquarePrecent)) {
        room.height = room.width;
    } else {
        room.height = randRange((isDeadEnd) ? deadEndMinDim : midMinDim,
                                (isDeadEnd) ? deadEndMaxDim : midMaxDim);

        // 50 % chance to flip the room
        if(rand()%2 == 0) {
            room = rotRoom(room);
        }
    }

    room.x = randRange(0, mazeCols-1);
    room.y = randRange(0, mazeRows-1);

    return room;
}

/**
 * Calculates the shortest path distance between 2 rooms
 * 
 * @param one The first room to check
 * @param two The second room to check
 * 
 * @return The distance of a shortest path between the rooms
 */
int roomDist(room_t one, room_t two, room_t * src, room_t * dst) {
    // Find x and y coords in each which have minimum dist in that dimension
    int x1 = one.x, y1 = one.y;
    int x2 = two.x, y2 = two.y;

    int xSep = abs(x1-x2), ySep = abs(y1-y2);

    for(int dX1 = one.width - 1; dX1 >= 0; dX1--) {
        for(int dX2 = 0; dX2 < two.width; dX2++) {
            if(abs((one.x + dX1)-(two.x + dX2)) < xSep) {
                x1 = (one.x + dX1);
                x2 = (two.x + dX2);
                xSep = abs(x1-x2);
            }
        }
    }

    for(int dY1 = 0; dY1 < one.height; dY1++) {
        for(int dY2 = two.height-1; dY2 >= 0; dY2--) {
            if(abs((one.y + dY1)-(two.y + dY2)) < ySep) {
                y1 = (one.y + dY1);
                y2 = (two.y + dY2);
                ySep = abs(y1-y2);
            }
        }
    }

    // If they aren't null, set the src and dst "rooms" with the located 
    // positions
    if(src != NULL) {
        *src = (room_t) {x1, y1, 1, 1};
    }
    if(dst != NULL) {
        *dst = (room_t) {x2, y2, 1, 1};
    }

    return (xSep - 1) + (ySep - 1);
}


room_t * roomCompBase = NULL;
int compRooms(const void* a, const void* b) {
    if(roomCompBase == NULL || a == NULL || b == NULL) return 0;

    int distA = roomDist(*roomCompBase, *(room_t *) a, NULL, NULL);
    int distB = roomDist(*roomCompBase, *(room_t *) b, NULL, NULL);
    return distA - distB;
}

//=============================<Helper Functions>=============================//
/**
 * Prints a basic usage message
 * 
 * @param call The name of this executable (i.e. argv[0])
 */
void printUsage(const char * call) {
    printf("Usage: %s [%s <finalRows> <finalCols>] [%s <startRows> <startCols>]"
        " [%s <mazeRows> <mazeCols>] [%s <midRooms>] [%s <midRoomMinDim> "
        "<midRoomMaxDim>] [%s <deadEnds>] [%s <endMidDim> <endMaxDim>] "
        "[%s <overlapRetries>] [%s <oobRetries>] <Output File>\n\n", call, 
        kFinalDimFlag, kStartDimFlag, kMazeDimFlag, kMidRoomsFlag, 
        kMidRoomDimFlag, kDeadEndsFlag, kDeadEndDimFlag, kOverlapRetriesFlag, 
        kOOBRetriesFlag);
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


void mkXPath(map_t * map, room_t src, room_t dst) {
    if(src.x == dst.x) return;

    bool goingLeft = dst.x < src.x;
    int delta = (goingLeft) ? -1 : 1;

    int y = src.y;
    for(int x = src.x; x != dst.x; x += delta) {
        if(map->data[y][x].isEmpty) {   // On a new tile...
            // Enable this tile and mark it as a hallway
            map->data[y][x].isEmpty = false;
            map->data[y][x].sprite = 0;

            // Set its walls
            map->data[y][x].uWall = 1;
            if(y == map->nRows - 1) map->data[y][x].dWall = 1;
            else map->data[y+1][x].uWall = 1;
        } else {   // On an existing tile...
            // If room tile, replace walls in dir of travel with doors. 
            // Else delete them
            unsigned char rep = (map->data[y][x].sprite == kNoSprite) ? 2 : 0;
            
            if(goingLeft) {
                if(map->data[y][x].lWall == 1) {
                    map->data[y][x].lWall = rep;
                }
            } else if(x == map->nCols - 1) {
                if(map->data[y][x].rWall == 1) {
                    map->data[y][x].rWall = rep;
                }
            } else {
                if(map->data[y][x + 1].lWall == 1) {
                    map->data[y][x + 1].lWall = rep;
                }
            }
        }
    }

    // Handle the last tile in the link
    int x = dst.x;
    if(map->data[y][x].isEmpty) { // On empty last tile...
        // Enable this tile and mark it as a hallway
        map->data[y][x].isEmpty = false;
        map->data[y][x].sprite = 0;

        // Set its walls
        map->data[y][x].uWall = 1;
        if(y == map->nRows - 1) map->data[y][x].dWall = 1;
        else map->data[y+1][x].uWall = 1;

        if(goingLeft) {
            map->data[y][x].lWall = 1;
        } else if(x == map->nCols - 1) {
            map->data[y][x].rWall = 1;
        } else {
            map->data[y][x + 1].lWall = 1;
        }
    } else { // On an existing last tile...
        // If room tile, replace wall behind you with door. Else delete it
        unsigned char rep = (map->data[y][x].sprite == kNoSprite) ? 2 : 0;
        if(!goingLeft) {
            map->data[y][x].lWall = rep;
        } else if(x == map->nCols - 1) {
            map->data[y][x].rWall = rep;
        } else {
            map->data[y][x+1].lWall = rep;
        }
    }
}

void mkYPath(map_t * map, room_t src, room_t dst) {
    if(src.y == dst.y) return;

    bool goingDown = dst.y > src.y;
    int delta = (goingDown) ? 1 : -1;

    int x = src.x;
    int y = src.y;

    for(; y != dst.y; y += delta) {
        if(map->data[y][x].isEmpty) { // On an empty tile
            // Enable the tile and mark it as a hall
            map->data[y][x].isEmpty = false;
            map->data[y][x].sprite = 0;

            // Set this tile's side walls
            map->data[y][x].lWall = 1;
            if(x == map->nCols - 1) map->data[y][x].rWall = 1;
            else map->data[y][x + 1].lWall = 1;
        } else { // On an existing tile...
            // If room tile, replace walls in direction of travel with doors.
            // Else, delete them
            unsigned char rep = (map->data[y][x].sprite == kNoSprite) ? 2 : 0;

            if(!goingDown) {
                if (map->data[y][x].uWall == 1) {
                    map->data[y][x].uWall = rep;
                }
            } else if (y == map->nRows - 1) {
                if (map->data[y][x].dWall == 1) {
                    map->data[y][x].dWall = rep;
                }
            } else {
                if (map->data[y+1][x].uWall == 1) {
                    map->data[y+1][x].uWall = rep;
                }
            }
        }
    }

    // Handle the last tile as well
    if(map->data[y][x].isEmpty) { // On an empty final tile...
        // Enable the tile and mark it as a hall
        map->data[y][x].isEmpty = false;
        map->data[y][x].sprite = 0;

        // Set this tile's side walls
        map->data[y][x].lWall = 1;
        if(x == map->nCols - 1) map->data[y][x].rWall = 1;
        else map->data[y][x + 1].lWall = 1;
        
        // Set the tile's cap wall
        if(!goingDown) {
            map->data[y][x].uWall = 1;
        } else if(y == map->nRows - 1) {
            map->data[y][x].dWall = 1;
        } else {
            map->data[y+1][x].uWall = 1;
        }

    } else { // On an existing tile...
        // If room tile, replace walls behind you with doors. Else, delete them
        unsigned char rep = (map->data[y][x].sprite == kNoSprite) ? 2 : 0;

        if(goingDown) {
            map->data[y][x].uWall = rep;
        } else if (y == map->nRows - 1) {
            map->data[y][x].dWall = rep;
        } else {
            map->data[y+1][x].uWall = rep;
        }
    }
}

/**
 * Generates a hallway between 2 rooms on the map
 * 
 * @param map The map to place it in
 * @param src The room to start from
 * @param dst The room to end in
 */
void mkPath(map_t * map, room_t src, room_t dst) {
    if(map == NULL || map->data == NULL) return;

    roomDist(src, dst, &src, &dst);
    src.x = max(0, min(src.x, map->nCols-1));
    dst.x = max(0, min(dst.x, map->nCols-1));
    src.y = max(0, min(src.y, map->nRows-1));
    dst.y = max(0, min(dst.y, map->nRows-1));

    int xSep = abs(src.x - dst.x);
    int ySep = abs(src.y - dst.y);

    if(xSep > ySep) {
        mkXPath(map, src, dst);
        src.x = dst.x;
        mkYPath(map, src, dst);
    } else {
        mkYPath(map, src, dst);
        src.y = dst.y;
        mkXPath(map, src, dst);
    }
    
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
            if(sscanf(argv[++i], "%d", &mazeRows) != 1 || mazeRows <= 0) {
                fprintf(stderr, "*FATAL ERROR* in main: Invalid first room dimension \"%s\"\n", argv[i]);
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
            if(sscanf(argv[++i], "%d", &mazeCols) != 1 || mazeCols <= 0) {
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
        } else if(strcmp(kMidRoomDimFlag, argv[i]) == 0) {
            // Trying to set first room dimensions
            if(i+2 >= argc) {
                fprintf(stderr, "*FATAL ERROR* in main: Mid room dimensions unspecified\n");
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
            if(sscanf(argv[++i], "%d", &midMinDim) != 1 || midMinDim <= 0) {
                fprintf(stderr, "*FATAL ERROR* in main: Invalid mid room min dimension \"%s\"\n", argv[i]);
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
            if(sscanf(argv[++i], "%d", &midMaxDim) != 1 || midMaxDim < midMinDim) {
                fprintf(stderr, "*FATAL ERROR* in main: Invalid mid room min dimension \"%s\"\n", argv[i]);
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
        } else if(strcmp(kDeadEndDimFlag, argv[i]) == 0) {
            // Trying to set first room dimensions
            if(i+2 >= argc) {
                fprintf(stderr, "*FATAL ERROR* in main: End room dimensions unspecified\n");
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
            if(sscanf(argv[++i], "%d", &deadEndMinDim) != 1 || deadEndMinDim <= 0) {
                fprintf(stderr, "*FATAL ERROR* in main: Invalid mid room min dimension \"%s\"\n", argv[i]);
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
            if(sscanf(argv[++i], "%d", &deadEndMaxDim) != 1 || deadEndMaxDim < deadEndMinDim) {
                fprintf(stderr, "*FATAL ERROR* in main: Invalid mid room min dimension \"%s\"\n", argv[i]);
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
        } else if(strcmp(kDeadEndsFlag, argv[i]) == 0) {
            // Trying to set first room dimensions
            if(i+1 >= argc) {
                fprintf(stderr, "*FATAL ERROR* in main: Dead end count unspecified\n");
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
            if(sscanf(argv[++i], "%d", &deadEnds) != 1 || midRooms < 0) {
                fprintf(stderr, "*FATAL ERROR* in main: Invalid dead end count \"%s\"\n", argv[i]);
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
        } else if(strcmp(kOOBRetriesFlag, argv[i]) == 0) {
            // Trying to set first room dimensions
            if(i+1 >= argc) {
                fprintf(stderr, "*FATAL ERROR* in main: OOB retry count unspecified\n");
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
            if(sscanf(argv[++i], "%d", &oobRetries) != 1 || oobRetries < 0) {
                fprintf(stderr, "*FATAL ERROR* in main: Invalid OOB retry count \"%s\"\n", argv[i]);
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
        } else if(strcmp(kOverlapRetriesFlag, argv[i]) == 0) {
            // Trying to set first room dimensions
            if(i+1 >= argc) {
                fprintf(stderr, "*FATAL ERROR* in main: Overlap retry count unspecified\n");
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
            if(sscanf(argv[++i], "%d", &overlapRetries) != 1 || overlapRetries < 0) {
                fprintf(stderr, "*FATAL ERROR* in main: Invalid overlap retry count \"%s\"\n", argv[i]);
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
    srand(time(0));

    // Make a map to hold the maze
    map_t map;
    if(mkMap(mazeRows, mazeCols, &map) != 0) {
        fprintf(stderr, "*FATAL ERROR* failed to allocate a map\n");
        return EXIT_FAILURE;
    }

    // Make lists to hold the path and dead end rooms
    room_t * pathRooms = calloc(2 + midRooms, sizeof(room_t));
    if(pathRooms == NULL) {
        fprintf(stderr, "*FATAL ERROR* failed to allocate a path room list\n");
        rmMap(map);
        return EXIT_FAILURE;
    }

    room_t * endRooms = calloc(deadEnds, sizeof(room_t));
    if(endRooms == NULL) {
        fprintf(stderr, "*FATAL ERROR* failed to allocate a dead-end room list\n");
        free(pathRooms);
        rmMap(map);
        return EXIT_FAILURE;
    }

    // Generate the starting and ending rooms
    room_t firstRoom = {0, 0, firstRoomCols, firstRoomRows};
    room_t finalRoom = {map.nCols - finalRoomCols, map.nRows - finalRoomRows, 
                            finalRoomCols, finalRoomRows};

    // Generate all path rooms
    pathRooms[0] = firstRoom;
    pathRooms[1] = finalRoom;

    placeRoom(&map, finalRoom, false);
    placeRoom(&map, firstRoom, false);

    for(int i = 2; i < 2 + midRooms; i++) {
        int overlaps = 0, bounds = 0;
        while(overlaps < overlapRetries && bounds < oobRetries) {
            pathRooms[i] = mkRandRoom(false);
            if(roomOverlaps(map, pathRooms[i])) {
                overlaps += 1;
            } else if (isOutOfBounds(map, pathRooms[i])) {
                bounds += 1;
            } else {
                break;
            }
        }

        placeRoom(&map, pathRooms[i], false);
    }

    // Sort the rooms by distance from start
    roomCompBase = &firstRoom;
    qsort(pathRooms, 2 + midRooms, sizeof(room_t), compRooms);

    // Build paths between each of the mid rooms
    for(int i = 1; i < 2 + midRooms; i++) {
        mkPath(&map, pathRooms[i-1], pathRooms[i]);
    }

    // Generate all of the dead end rooms
    for(int i = 0; i < deadEnds; i++) {
        int overlaps = 0, bounds = 0;
        while(overlaps < overlapRetries && bounds < oobRetries) {
            endRooms[i] = mkRandRoom(true);
            if(roomOverlaps(map, endRooms[i])) {
                overlaps += 1;
            } else if (isOutOfBounds(map, endRooms[i])) {
                bounds += 1;
            } else {
                break;
            }
        }

        placeRoom(&map, endRooms[i], false);
    }

    // Make paths from all dead end rooms to their nearest path neighbors
    for(int i = 0; i < deadEnds; i++) {
        room_t src = endRooms[i];
        room_t dst = pathRooms[0];
        int dist = roomDist(src, dst, NULL, NULL);
        
        for(int j = 1; j < 2 + midRooms; j++) {
            int tmp = roomDist(src, pathRooms[j], NULL, NULL);
            if(tmp < dist) {
                dst = pathRooms[j];
                dist = tmp;
            }
        }

        mkPath(&map, src, dst);
    }
    
    // Ensure all sprite slots are reset
    for(int row = 0; row < map.nRows; row++) {
        for(int col = 0; col < map.nCols; col++) {
            map.data[row][col].sprite = kNoSprite;
        }
    }

    // Place a char sprite showing room order
    for(int i = 0; i < 2 + midRooms; i++) {
        setCharSprite(&map.data[pathRooms[i].y][pathRooms[i].x], '0' + (i % 10),
                        kDefPalette);
    }

    //=============================<Cleanup>==============================//
    // Set start and end sprites
    setCharSprite(&map.data[0][0], 'S', kGreenPalette);
    map.data[0][0].spritePalette = kGreenPalette;
    setCharSprite(&map.data[map.nRows-1][map.nCols-1], 'E', kRedPalette);
    map.data[map.nRows-1][map.nCols-1].spritePalette = kRedPalette;

    int status = EXIT_SUCCESS;
    // Write the generated map to file
    FILE* file = fopen(outFileLoc, "w");
    if(file == NULL || writeMap(map, NULL, file) != 0) {
        fprintf(stderr, "*FATAL ERROR* Failed to open the output file\n");
        status = EXIT_FAILURE;
    } else {
        fclose(file);
    }

    rmMap(map);

    free(pathRooms);
    free(endRooms);
    return status;
}