#include <stdlib.h>
#include <stdio.h>

#include "../common/list.h"

const char * str1 = "m >= n >= m+2";
const char * str2 = "a contradiction";

int main() {
    list_t list = mkList();
    FILE * fp;

    char * buf = calloc(strlen(str1) + 1, sizeof(char));
    strcpy(buf, str1);
    listAppend(list, buf);

    buf = calloc(strlen(str2) + 1, sizeof(char));
    strcpy(buf, str2);
    listAppend(list, buf);

    fp = fopen("test.out", "w");
    saveList(list, fp, writeStrEntry);
    fclose(fp);


    rmList(list, free);
    return EXIT_SUCCESS;
}