#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "charData.h"
#include "genUtils.h"

// OS specific FS stuff
#ifdef __unix__
#include "fs_unix.h"
#endif

int main() {
    char inBuf[256];
    int ret;

    character_t character = mkBlankCharacter();
    

    // Grab all data required for the 
    printf("Enter a character name: ");
    fgets(inBuf, 256, stdin);
    strTrim(inBuf);
    character.name = calloc(strlen(inBuf) + 1, sizeof(char));
    strcpy(character.name, inBuf);

    // Cleanup
    freeCharacterData(character);

    return EXIT_SUCCESS;
}

