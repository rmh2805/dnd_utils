#include "genUtils.h"

void strTrim(char * s) {
    lTrim(s);
    rTrim(s);
}

void lTrim(char * s) {
    if(s == NULL) return;

    unsigned int i;
    for(i = 0; i < strlen(s) && isspace(s[i]); i++);

    unsigned int j = 0;
    while(i < strlen(s)) {
        s[j++] = s[i++];
    }

    s[j] = 0;
}

void rTrim(char * s) {
    if(s == NULL) return;

    unsigned int i;
    for(i = strlen(s) - 1; i < strlen(s) && isspace(s[i]); i--);

    s[i + 1] = 0;

}