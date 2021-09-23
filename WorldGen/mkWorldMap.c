#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../map/map.h"

//=================================<Typedefs>=================================//
typedef enum mapType_e {
    coast, island, land, peninsula
} mapType_t;


//================================<Parameters>================================//
#define kSizeArg "-D"
#define kNumCities "-C"

#define kCoastArg "-c"
#define kIslandArg "-i"
#define kLandArg "-l"
#define kPeninsulaArg "-p"

//==============================<Default Params>==============================//
#define kDefSize 64
#define kDefCities 1

#define kDefMapType land

//===============================<Module State>===============================//
static int mapSize = kDefSize;
static int nCities = kDefCities;

static mapType_t mapType = kDefMapType;

//===============================<Helper Fxns>================================//


//================================<Main Code>=================================//
int main(int argc, char** argv) {
    for(int i = 1; i < argc; i++) {
        if(strcmp(kCoastArg, argv[i]) == 0) {
            mapType = coast;
        } else if(strcmp(kIslandArg, argv[i]) == 0) {
            mapType = island;
        } else if(strcmp(kLandArg, argv[i]) == 0) {
            mapType = land;
        } else if(strcmp(kPeninsulaArg, argv[i]) == 0) {
            mapType = peninsula;
        }
    }

    return EXIT_SUCCESS;
}

