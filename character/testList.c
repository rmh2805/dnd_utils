#include <stdlib.h>
#include <stdio.h>

#include "../common/list.h"

const char * str1 = "m >= n >= m+2";
const char * str2 = "a contradiction";

void freeHelp(void * ptr) {
    if(ptr != NULL) free(ptr);
}

int main() {
    list_t list = mkList();
    FILE * fp;
    int ret;

    listAppend(list, NULL);

    fp = fopen("test.out", "w");
    ret = saveList(list, fp, writeStrEntry);
    fclose(fp);

    rmList(list, freeHelp);
    list = NULL;

    if(ret < 0) {
        return EXIT_FAILURE;
    }

    fp = fopen("test.out", "r");
    ret = loadList(&list, fp, readStrEntry);
    fclose(fp);

    if(ret < 0) {
        if(list != NULL) rmList(list, freeHelp);
        return EXIT_FAILURE;
    }

    for(unsigned i = 0; i < listLen(list); i++) {
        printf("%s\n", (char *)listGet(list, i));
    }

    rmList(list, freeHelp);
    return EXIT_SUCCESS;
}