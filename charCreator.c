#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "charData.h"
#include "genUtils.h"

// OS specific FS stuff
#ifdef __unix__
#include "fs_unix.h"
#endif

#define getText(ch, buf, nBuf, field, prompt) \
    printf("%s: ", prompt);\
    fgets(buf, nBuf, stdin);\
    strTrim(buf);\
    ch.field = calloc(strlen(buf) + 1, sizeof(char));\
    strcpy(ch.field, buf);

#define textOut(ch, field, prompt)\
    printf("%s: %s\n", prompt, (ch.field == NULL) ? "" : ch.field)

void printCharacterOverview(character_t character) {
    textOut(character, name, "Name");
    textOut(character, playerName, "Player name");
    textOut(character, race, "Race");
    textOut(character, background, "Background");
}

int main() {
    char inBuf[256];
    int ret;

    character_t character = mkBlankCharacter();
    
    while (1) {
        getText(character, inBuf, 256, playerName, "Enter the player's name")
        getText(character, inBuf, 256, name, "Enter the character's name")
        getText(character, inBuf, 256, race, "Enter the character's race")
        getText(character, inBuf, 256, background, "Enter the character's background")
        
        printf("\n");
        printCharacterOverview(character);
        printf("\nDoes this look right? [Y/n]");
        
        fgets(inBuf, 256, stdin);
        strTrim(inBuf);
        if(inBuf[0] != 'n' && inBuf[0] != 'N') break;

        printf("\n");
    }
    

    // Cleanup
    freeCharacterData(character);

    return EXIT_SUCCESS;
}

