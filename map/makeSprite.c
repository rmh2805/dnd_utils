#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "sprite.h"
#include "dispBase.h"

typedef enum mode_e {menu, tile, sprite, load, save} mode_t;

int main() {
    int ret;

    // Initialize the display
    dispData_t data;
    ret = initDisp(&data);
    if(ret != 0) {
        fprintf(stderr, "*ERROR* in main: Failed to initialized display\n");
        return EXIT_FAILURE;
    }

    // Main loop
    bool running = true;
    while(running) {
        
    }

    // Cleanup and exit
    closeDisp();

    return EXIT_SUCCESS;
}