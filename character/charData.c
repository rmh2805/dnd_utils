#include "charData.h"

/**
 * Generates a null-initialized character
 * 
 * @return A null-initialized character struct
 */
character_t mkBlankCharacter() {
    character_t character;

    for(unsigned int i = 0; i < sizeof(character_t); i++) {
        ((char *) &character)[i] = 0;
    }
    return character;
}


/**
 * Free all allocated data associated with the character
 * 
 * @param character The character to free from
 * @return An exit status (0 on success, <0 on failure)
 */
int freeCharacterData(character_t character) {
    if(character.savePath != NULL) free(character.savePath);
    if(character.name != NULL) free(character.name);
    if(character.playerName != NULL) free(character.playerName);
    if(character.race != NULL) free(character.race);
    if(character.background != NULL) free(character.background);

    return 0;
}

/**
 * Loads a character from file
 * 
 * @param savePath The path of the character's save directory
 * @param character A return pointer for the character struct
 * @return An exit status (0 on success, <0 on failure)
 */
int loadCharacter(const char * savePath, character_t * character) {
    return -1;
}


/**
 * Writes the provided character's information out to file
 * 
 * @param character The character to write to file
 * @return An exit status (0 on success, <0 on failure)
 */
int writeCharacter(character_t character) {
    return -1;
}