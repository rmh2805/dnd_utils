#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <ncurses.h>
#include "mapDisp.h"

#include "actor.h"

//==============================<Misc Constants>==============================//

//=============================<Menu Definitions>=============================//
typedef enum mode_e {menu, quit} mode_t;

const char * menuItems[] = {
    "1. Quit"
};

mode_t menuModes[] = {
    quit
};

const int menuSize = sizeof(menuItems) / sizeof(menuItems[0]);

//===========================<Helper Declarations>============================//

//================================<Main Code>=================================//
int main(int argc, char** argv) {
    //=======================<Variable Definitions>=======================//
    int status = EXIT_FAILURE;

    dispData_t dispData;
    bool dispOpen = false;

    actorData_t actors;
    bool actorsLoaded = false;

    //==============================<Setup>===============================//

    // Initialize the display
    if(initDisp(&dispData) < 0) goto main_cleanup;

    //============================<Main Loop>=============================//

    //=============================<Cleanup>==============================//
    status = EXIT_SUCCESS;
main_cleanup:
    if(dispOpen) {
        closeDisp(dispData);
    }
    if(actorsLoaded) {
        rmActorData(actors);
    }
    return status;
}

//============================<Helper Definitions>============================//