#include <stdlib.h>
#include <stdio.h>

#include "sprite.h"

#define kOutFile "out.o"

void printSprite(sprite_t sprite) {
    for(int row = 0; row < sprite.height; row++) {
        for(int col = 0; col < sprite.width; col++) {
            char ch = sprite.data[row][col];
            if(ch == 0) {
                printf(" ");
            } else {
                printf("%c", ch);
            }
        }
        printf("\n");
    }
}

int main() {
    //Make a simple sprite and print it out
    printf("Allocating a sprite\n");

    sprite_t sprite = mkSprite(0, 3, 3, 0, 0);
    if(sprite.data == NULL) {
        fprintf(stderr, "*ERROR* in main: failed to alloc initial sprite\n");
        return EXIT_FAILURE;
    }

    for(int row = 0; row < sprite.height; row++) {
        for(int col = 0; col < sprite.width; col++) {
            sprite.data[row][col] = '#';
        }
    }
    sprite.data[1][1] = 0;
    sprite.data[0][0] = '\\';

    // Test writing sprites to file
    printf("Writing sprites to file\n");

    FILE* fp = fopen(kOutFile, "w");
    if(fp == NULL) {
        rmSprite(sprite);
        fprintf(stderr, "*ERROR* in main: failed to open file for write\n");
        return EXIT_FAILURE;
    }

    writeSprite(fp, sprite);

    printf("First sprite written: \n\n");
    printSprite(sprite);
    printf("\n");

    sprite.data[0][2] = '/';
    writeSprite(fp, sprite);

    printf("Second sprite written: \n\n");
    printSprite(sprite);
    printf("\n");

    fclose(fp);
    rmSprite(sprite);

    // Test reading sprites from file
    fp = fopen(kOutFile, "r");

    int i = 0;
    for(sprite = readSprite(fp); sprite.data != NULL; sprite = readSprite(fp)) {
        printf("Read in sprite number %d:\n\n", ++i);
        printSprite(sprite);
        printf("\n");

        rmSprite(sprite);
    }
    printf("Read %d sprites total\n", i);

    //Cleanup and exit
    fclose(fp);
    return EXIT_SUCCESS;
}