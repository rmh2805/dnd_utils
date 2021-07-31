#include <stdlib.h>
#include <stdio.h>

// OS specific FS stuff
#ifdef __unix__
#include "fs_unix.h"
#endif


#include "charData.h"

int main() {
    printf("%lu\n", sizeof(character_t));
    return EXIT_SUCCESS;
}

