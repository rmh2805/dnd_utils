#include "dispBase.h"

//=============================<Init and Cleanup>=============================//

int initDisp(dispData_t* data) {
    // Initialize curses mode
    initscr();

    keypad(stdscr, true);
    noecho();

    data->data = NULL;

    // Check for color and set up palette pairs
    if(!has_colors()) {
        closeDisp(*data);
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

    // Alloc the frame buffer
    data->data = calloc(data->screenRows, sizeof(drawPair_t *));
    if(data->data == NULL) {
        closeDisp(*data);
        fprintf(stderr, "*ERROR* in initDisp: Unable to allocate a frame buffer (rows)\n");
        return -1;
    }

    for(int i = 0; i < data->screenRows; i++) {
        data->data[i] = calloc(data->screenCols, sizeof(drawPair_t));

        if(data->data[i] == NULL) {
            closeDisp(*data);
            fprintf(stderr, "*ERROR* in initDisp: Unable to allocate a frame buffer (cols)\n");
            return -1;
        }
    }


    // Do final setup and return success
    clearBuffer(data);
    return 0;
}

int closeDisp(dispData_t data) {
    curs_set(1);
    endwin();

    if(data.data != NULL) {
        for(int i = 0; i < data.screenRows; i++) {
            if(data.data[i] != NULL) {
                free(data.data[i]);
                data.data[i] = NULL;
            }
        }

        free(data.data);
    }

    return 0;
}

//=============================<Buffer Handling>==============================//

void addText(dispData_t * data, short palette, const char * text, int row, int col) {
    if(data == NULL || data->data == NULL || 
            row < 0 || row > data->screenRows || 
            col < 0 || col > data->screenCols) {
        return;
    }
    
    for(int dCol = 0; col + dCol < data->screenCols && text[dCol]; dCol++) {
        data->data[row][col + dCol] = (drawPair_t) {palette, text[dCol]};
    }
}

void addMenu(dispData_t * data, const char * prompt, const char ** items, 
                int nItems, int selected) {
    if(data == NULL || prompt == NULL || items == NULL || nItems <= 0) {
        return;
    }

    clearBuffer(data);
    addText(data, kBlackPalette, prompt, 0, 0);

    for (int i = 0; i < nItems; i++) {
        addText(data, (i == selected) ? kWhitePalette : kBlackPalette, items[i],
                    2 + i, 0);
    }
}

void addBackground(dispData_t * data, short palette) {
    if(data == NULL || data->data == NULL) {
        return;
    }

    for(int row = 0; row < data->screenRows; ++row) {
        for(int col = 0; col < data->screenCols; ++col) {
            data->data[row][col].ch = ' ';
            data->data[row][col].palette = palette;
        }
    }
}

void printBuffer(dispData_t data) {
    if(data.data == NULL) return;

    // First clear the screen
    clear();

    // Then loop for each palette (since each requires its own refresh)
    for(short palette = kMinPalette; palette < kMaxPalette; palette++) {
        wattron(stdscr, COLOR_PAIR(palette));

        // Finally itterate through all rows and cols
        for(int row = 0; row < data.screenRows; row++) {
            for(int col = 0; col < data.screenCols; col++) {
                if(data.data[row][col].palette != palette || 
                        data.data[row][col].ch == '\0') {
                    continue;
                }

                mvaddch(row, col, data.data[row][col].ch);
            }
        }
        refresh();
        wattroff(stdscr, COLOR_PAIR(palette));
    }
}

void clearBuffer(dispData_t * data) {
    if(data == NULL || data->data == NULL) {
        return;
    }

    for(int row = 0; row < data->screenRows; row++) {
        for(int col = 0; col < data->screenCols; col++) {
            data->data[row][col] = (drawPair_t) {kDefPalette, '\0'};
        }
    }
}

//=============================<Buffer Handling>==============================//

void printText(short palette, const char * text, int row, int col) {
    wmove(stdscr, row, col);
    wattron(stdscr, COLOR_PAIR(palette));
    wprintw(stdscr, text, row, col);
    refresh();
    wattroff(stdscr, COLOR_PAIR(palette));
}

void getText(int row, int col, char* buf, unsigned int nBuf) {
    curs_set(1);
    echo();

    wmove(stdscr, row, col);
    wclrtoeol(stdscr);
    wmove(stdscr, row, col);

    unsigned i;
    for(i = 0; i < nBuf - 1; i++) {
        int ch = wgetch(stdscr);

        if(ch == KEY_ENTER || ch == '\n') {
            break; 
        } else if (ch == KEY_DC || ch == 27 || ch == '\b' || ch == KEY_BACKSPACE) {  // For some reason, delete gets sent as 27
            if(i != 0) {
                --i;            // Roll back to the previous entry
                buf[i] = 0;     // Remove the character from the out buffer

                // Erase the displayed character and retarget cursor
                wmove(stdscr, row, col + i);
                waddch(stdscr, ' ');
                wmove(stdscr, row, col + i);
                refresh();

                // Decrement i again to account for the loop increment
                --i;
            }
        } else {
            buf[i] = ch;
        }
    }
    buf[i] = 0;

    curs_set(0);
    noecho();
}