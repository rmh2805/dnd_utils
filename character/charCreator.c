#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "charData.h"

int main() {
    charData_t data = mkCharData();

    FILE* fp = fopen("testChar.out", "r");
    if(fp == NULL) {
        return EXIT_FAILURE;
    }

    if(loadCharData(fp, &data) != 0) {
        return EXIT_FAILURE;
    }
    fclose(fp);

    data.proStr = (data.proStr) ? 0 : 1;

    fp = fopen("testChar.out", "w");
    if(fp == NULL) {
        rmCharData(data);
        return EXIT_FAILURE;
    }
    saveCharData(fp, data);
    fclose(fp);

    rmCharData(data);
    return EXIT_SUCCESS;
}