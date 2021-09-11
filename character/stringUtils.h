#ifndef _STRING_UTILS_H_
#define _STRING_UTILS_H_

#include <stdlib.h>
#include <string.h>

#define isWS(ch) (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t')

/**
 * Removes leading and trailing whitespace from the string (without resizing)
 * 
 * @param buf The string to modify
 */
void strip(char * buf);

/**
 * Removes leading whitespace from the string (without resizing)
 * 
 * @param buf The string to modify
 */
void lStrip(char * buf);

/**
 * Removes trailing whitespace from the string (without resizing)
 * 
 * @param buf The string to modify
 */
void rStrip(char * buf);

#endif