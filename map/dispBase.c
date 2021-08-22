#include "dispBase.h"

/**
 * Core display initialization
 * 
 * @param data A return pointer for a display data struct
 * @return 0 iff display was initialized correctly
 */
int initDisp(dispData_t* data) {
    // Initialize curses mode
    initscr();

    keypad(stdscr, true);
    noecho();

    // Check for color and set up palette pairs
    if(!has_colors()) {
        closeDisp();
        fprintf(stderr, "*ERROR* in initDisp: Terminal does not have color\n");
        return -1;
    }

    start_color();
    init_pair(kBlackPalette,   COLOR_WHITE, COLOR_BLACK);
    init_pair(kWhitePalette,   COLOR_BLACK, COLOR_WHITE);
    init_pair(kRedPalette,     COLOR_BLACK, COLOR_RED);
    init_pair(kGreenPalette,   COLOR_BLACK, COLOR_GREEN);
    init_pair(kBluePalette,    COLOR_BLACK, COLOR_BLUE);
    init_pair(kYellowPalette,  COLOR_BLACK, COLOR_YELLOW);
    init_pair(kMagentaPalette, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(kCyanPalette,    COLOR_BLACK, COLOR_CYAN);



    // Get screen size
    getmaxyx(stdscr, data->screenRows, data->screenCols);

    return 0;
}

/**
 * Core display close
 * 
 * @return 0 iff display was closed correctly
 */
int closeDisp() {
    endwin();

    return 0;
}


/**
 * Draws the specified sprite on screen
 * 
 * @param data A common display data struct
 * @param sprite The sprite to draw
 * @param screenRow The top row to draw in
 * @param screenCol The left column to draw in
 */
void drawSprite(dispData_t data, sprite_t sprite, int screenRow, int screenCol) {
    if (sprite.data == NULL || 
            screenRow + sprite.height <= 0 || screenCol + sprite.width <= 0 ||
            screenRow >= data.screenRows || screenCol >= data.screenCols) {
        return;
    }

    wattron(stdscr, COLOR_PAIR(sprite.palette));
    for(int dRow = 0; dRow < sprite.height; dRow++) {
        int row = screenRow + dRow;
        if(row < 0 || row >= data.screenRows) continue;

        for(int dCol = 0; dCol < sprite.width; dCol++) {
            int col = screenCol + dCol;
            if(col < 0 || col >= data.screenCols) continue;

            char ch = sprite.data[dRow][dCol];
            if(ch == '\0') {
                continue;
            } else {
                wmove(stdscr, row, col);
                waddch(stdscr, ch);
            }
        }
    }
    refresh();
    wattroff(stdscr, COLOR_PAIR(sprite.palette));
}