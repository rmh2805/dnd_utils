#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "charData.h"
#include "genUtils.h"

// OS specific FS stuff
#ifdef __unix__
#include "fs_unix.h"
#endif

// Input Values
#define kReviewChar 'R'
#define kMainInfoChar 'I'
#define kAbilityChar 'A'
#define kProficiencyChar 'P'
#define kMiscChar 'M'
#define kEditChar 'E'
#define kQuitChar 'Q'

// Macro Instructions
#define toLower(ch) (ch >= 'A' && ch <= 'Z') ? ch - 'A' + 'a' : ch
#define toUpper(ch) (ch >= 'a' && ch <= 'z') ? ch - 'a' + 'A' : ch

#define getText(ch, buf, nBuf, field, prompt) \
    printf("%s: ", prompt);\
    fgets(buf, nBuf, stdin);\
    strTrim(buf);\
    ch.field = calloc(strlen(buf) + 1, sizeof(char));\
    strcpy(ch.field, buf);

#define textOut(ch, field, prompt)\
    printf("%s: %s\n", prompt, (ch.field == NULL) ? "<NONE>" : ch.field)

// Output Helpers
void printCharacterOverview(character_t character) {
    textOut(character, name, "Name");
    textOut(character, playerName, "Player name");
    textOut(character, race, "Race");
    textOut(character, background, "Background");
}

// Main Functions
int main() {
    char inBuf[256];
    bool loopDone = false;

    character_t character = mkBlankCharacter();

    while (!loopDone) {
        // Print the primary prompt
        printf("\nMain Menu:\n");
        printf("\t%c: Review the character\n", kReviewChar);
        printf("\t%c: Main Character Info\n", kMainInfoChar);
        printf("\t%c: Ability Info\n", kAbilityChar);
        printf("\t%c: Proficiency Info\n", kProficiencyChar);
        printf("\t%c: Misc Info\n", kMiscChar);
        printf("\t%c: Fine Detail\n", kEditChar);
        printf("\t%c: Quit Editing\n", kQuitChar);

        printf("\nEnter your selection: ");
        fgets(inBuf, 256, stdin);
        strTrim(inBuf);
        
        // Main selection loop
        switch(toUpper(inBuf[0])) {
            case kReviewChar:
                printf("\n");
                printCharacterOverview(character);
                printf("\nPress Enter to continue: ");
                fgets(inBuf, 256, stdin);
                break;

            case kMainInfoChar:
                while(1) {
                    printf("\n");
                    getText(character, inBuf, 256, playerName, "Enter the player's name")
                    getText(character, inBuf, 256, name, "Enter the character's name")
                    getText(character, inBuf, 256, race, "Enter the character's race")
                    getText(character, inBuf, 256, background, "Enter the character's background")
                    
                    printf("\n");
                    printCharacterOverview(character);
                    printf("\nDoes this look right? [Y/n]");
                    
                    fgets(inBuf, 256, stdin);
                    strTrim(inBuf);
                    if(toUpper(inBuf[0]) != 'N') break;

                    printf("\n");
                }
                break;
            
            case kQuitChar:
                loopDone = true;
                break;

            default:
                printf("\nUnrecognized command %c\n", toUpper(inBuf[0]));
                break;
        }
    }
    

    // Cleanup
    freeCharacterData(character);

    return EXIT_SUCCESS;
}

