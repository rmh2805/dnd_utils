#include "stringUtils.h"


/**
 * Removes leading and trailing whitespace from the string (without resizing)
 * 
 * @param buf The string to modify
 */
void strip(char * buf) {
    lStrip(buf);
    rStrip(buf);
}

/**
 * Removes leading whitespace from the string (without resizing)
 * 
 * @param buf The string to modify
 */
void lStrip(char * buf) {
    if(buf == NULL || strlen(buf) == 0) return;

    unsigned i, len = strlen(buf);
    for(i = 0; i < len && isWS(buf[i]); ++i);

    if(i == strlen(buf)) {
        buf[0] = 0;
        return;
    }

    unsigned j;
    for(j = 0; j + i < strlen(buf); j++) {
        buf[j] = buf[j + i];
    }
    buf[j] = 0;
}

/**
 * Removes trailing whitespace from the string (without resizing)
 * 
 * @param buf The string to modify
 */
void rStrip(char * buf) {
    if(buf == NULL || strlen(buf) == 0) return;

    int i;
    for(i = strlen(buf) - 1; i >= 0 && isWS(buf[i]); i--) {
        buf[i] = 0;
    }
}
