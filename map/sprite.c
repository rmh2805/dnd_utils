#include "sprite.h"


//============================<Memory Management>=============================//

/**
 * Allocates data for a sprite with the specified dimensions
 * 
 * @param width The width of the sprite (in characters)
 * @param height The height of the sprite (in characters)
 * 
 * @return The allocated and initialized sprite
 */
sprite_t mkSprite(short palette, unsigned char width, unsigned char height, unsigned char xOff, unsigned char yOff) {
    sprite_t sprite;
    sprite.defPalette = palette;
    sprite.width = width;
    sprite.height = height;
    sprite.xOff = xOff;
    sprite.yOff = yOff;

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

sprite_t * mkSpriteEntry(sprite_t sprite) {
    sprite_t * ptr = calloc(1, sizeof(sprite_t));
    if(ptr == NULL) {
        return NULL;
    }
    *ptr = sprite;
    return ptr;
}

void freeSpriteEntry(void * entry) {
    if(entry == NULL) {
        return;
    }
    rmSprite(*(sprite_t *) entry);
    free(entry);
}

//===============================<File Access>================================//

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

int writeSprite(FILE* file, sprite_t sprite) {
    if(file == NULL || sprite.data == NULL) return -1;

    fprintf(file, "%hd %hhu %hhu %hhu %hhu |", sprite.defPalette, sprite.width,
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

int loadSpriteList(FILE* file, list_t * list) {
    // Ensure that the file and sprite list both exist
    if(list == NULL || file == NULL) {
        return -1;
    }

    // Prepare internal files
    int nRead = 0, ret = 0;
    sprite_t sprite, * entry;

    // While valid sprites are being returned from the file...
    while((sprite = readSprite(file)).data != NULL) {
        ret = -1;
        // Turn the sprite into an entry
        entry = mkSpriteEntry(sprite);
        if(entry == NULL) {
            break;
        }

        ret = listAppend(*list, entry);
        if(ret < 0) {
            freeSpriteEntry(entry);
            break;
        }

        ++nRead;
    }

    if(nRead == 0 && ret < 0) { // If we failed to create or append the first entry...
        return -1;
    }

    return nRead;
}

int saveSpriteList(FILE* file, list_t list) {
    if(file == NULL || list == NULL) {
        return -1;
    }

    int len = listLen(list), ret = 0, nWritten = 0;

    for(int i = 0; i < len; ++i) {
        sprite_t * entry = listGet(list, i);
        if(entry == NULL) {
            continue;
        }
        ret = writeSprite(file, *entry);
        if(ret < 0) {
            continue;
        }
        nWritten += 1;
    }

    return nWritten;
}

int readSpriteEntry(void** entry, FILE* fp) {
    if(entry == NULL) return -1;

    // Free any existing sprite entries
    if(*entry != NULL) {
        freeSpriteEntry(*(sprite_t**)entry);
    }

    // Read a sprite onto the stack
    sprite_t sprite = readSprite(fp);
    if(sprite.data == NULL) {
        return -1;
    }

    // Move the sprite from stack to heap
    if((*entry = mkSpriteEntry(sprite)) == NULL) {
        rmSprite(sprite);
        return -1;
    }

    // Return success
    return 0;
}

int writeSpriteEntry(void* entry, FILE* fp) {
    return writeSprite(fp, *(sprite_t *)entry);
}
