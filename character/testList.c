#include <stdlib.h>
#include <stdio.h>

#include "../common/list.h"

const char * str1 = "m >= n >= m+2";
const char * str2 = "a contradiction";

int main() {
    list_t list = mkList();
    FILE * fp;
    int ret;

    char * buf = calloc(strlen(str1) + 1, sizeof(char));
    strcpy(buf, str1);
    listAppend(list, buf);

    buf = calloc(strlen(str2) + 1, sizeof(char));
    strcpy(buf, str2);
    listAppend(list, buf);

    fp = fopen("test.out", "w");
    ret = saveList(list, fp, writeStrEntry);
    fclose(fp);

    rmList(list, free);
    list = NULL;

    if(ret < 0) {
        return EXIT_FAILURE;
    }

    fp = fopen("test.out", "r");
    ret = loadList(&list, fp, readStrEntry);
    fclose(fp);

    if(ret < 0) {
        if(list != NULL) rmList(list, free);
        return EXIT_FAILURE;
    }

    for(unsigned i = 0; i < listLen(list); i++) {
        printf("%s\n", (char *)listGet(list, i));
    }

    rmList(list, free);
    return EXIT_SUCCESS;
}