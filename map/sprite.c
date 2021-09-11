#include "sprite.h"


/**
 * Allocates data for a sprite with the specified dimensions
 * 
 * @param width The width of the sprite (in characters)
 * @param height The height of the sprite (in characters)
 * 
 * @return The allocated and initialized sprite
 */
sprite_t mkSprite(short palette, unsigned char width, unsigned char height, unsigned char xOff, unsigned char yOff) {
    sprite_t sprite = {palette, width, height, xOff, yOff, NULL};

    sprite.data = calloc(height, sizeof(char *));
    
    if(sprite.data == NULL) {
        return kEmptySprite;
    }
    
    for(int row = 0; row < height; row++) {
        sprite.data[row] = calloc(width, sizeof(char));
        
        if(sprite.data[row] == NULL) {
            rmSprite(sprite);
            return kEmptySprite;
        }
    }

    return sprite;
}


/**
 * Allocates data for a tile with the specified dimensions and initializes it as
 * opaque and blank
 * 
 * @param palette The color palette of the tile
 * @param width The width of the tile (in characters)
 * @param height The height of the tile (in characters)
 * 
 * @return The allocated and initialized sprite
 */
sprite_t mkBlankTile(short palette, unsigned char width, unsigned char height) {
    sprite_t sprite = mkSprite(palette, width, height, 0, 0);
    if(sprite.data == NULL) {
        return kEmptySprite;
    }

    for(int row = 0; row < height; row++) {
        for(int col = 0; col < width; col++) {
            sprite.data[row][col] = ' ';
        }
    }

    return sprite;
}


/**
 * Frees all data allocated for the sprite
 * 
 * @param sprite The sprite to free
 */
void rmSprite(sprite_t sprite) {
    // Free all character data
    if(sprite.data == NULL) return;

    for(int row = 0; row < sprite.height; row++) {
        if(sprite.data[row] != NULL) {
            free(sprite.data[row]);
        }
    }

    free(sprite.data);
}

/**
 * Reads a sprite from file
 * 
 * @param file The file to read from
 * 
 * @return The sprite read from file
 */
sprite_t readSprite(FILE* file) {
    short palette;
    unsigned char width, height;
    unsigned char xOff, yOff;

    int ret;

    // Get the components from the line
    ret = fscanf(file, "%hd %hhu %hhu %hhu %hhu |", &palette, &width, &height, &xOff, &yOff);
    if(ret != 5) {
        return kEmptySprite;
    }

    // Create the basic struct
    sprite_t sprite = mkSprite(palette, width, height, xOff, yOff);

    //Read in the data from file
    for(int row = 0; row < sprite.height; row++) {
        for(int col = 0; col < sprite.width; col++) {
            char ch = fgetc(file);

            switch (ch) {
                case EOF:
                    rmSprite(sprite);
                    return kEmptySprite;

                case '\\':
                    ch = fgetc(file);
                    switch(ch) {
                        case '0':
                            sprite.data[row][col] = 0;
                            break;
                        case '\\':
                            sprite.data[row][col] = '\\';
                            break;
                        default:
                            rmSprite(sprite);
                            return kEmptySprite;
                    }
                    break;

                default:
                    sprite.data[row][col] = ch;
            }
        }
    }

    // Return the sprite read from file
    return sprite;
}

/**
 * Writes a sprite out to the file
 * 
 * @param file The file to write to
 * @param sprite The sprite to write out
 * 
 * @return 0 iff successful
 */
int writeSprite(FILE* file, sprite_t sprite) {
    if(file == NULL || sprite.data == NULL) return -1;

    fprintf(file, "%hd %hhu %hhu %hhu %hhu |", sprite.palette, sprite.width,
                sprite.height, sprite.xOff, sprite.yOff);
    
    for(int row = 0; row < sprite.height; row++) {
        for(int col = 0; col < sprite.width; col++) {
            switch(sprite.data[row][col]) {
                case 0:
                    fprintf(file, "\\0");   // prints as "\0"
                    break;
                case '\\':
                    fprintf(file, "\\\\"); // prints as "\\"
                    break;
                default:
                    fprintf(file, "%c", sprite.data[row][col]);
            }
        }
    }
    fprintf(file, "\n");
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
void addSprite(dispData_t * data, sprite_t sprite, int screenRow, int screenCol) {
    if(data == NULL || data->data == NULL) return;

    screenRow = screenRow + sprite.yOff;
    screenCol = screenCol + sprite.xOff;
    if (sprite.data == NULL || 
            screenRow + sprite.height <= 0 || screenCol + sprite.width <= 0 ||
            screenRow >= data->screenRows || screenCol >= data->screenCols) {
        return;
    }

    for(int dRow = 0; dRow < sprite.height; dRow++) {
        int row = screenRow + dRow;
        if(row < 0 || row >= data->screenRows) continue;

        for(int dCol = 0; dCol < sprite.width; dCol++) {
            int col = screenCol + dCol;
            if(col < 0 || col >= data->screenCols) continue;

            char ch = sprite.data[dRow][dCol];
            if(ch == '\0') {
                continue;
            } else {
                data->data[row][col] = (drawPair_t) {sprite.palette, sprite.data[dRow][dCol]};
            }
        }
    }
    refresh();
    wattroff(stdscr, COLOR_PAIR(sprite.palette));
}