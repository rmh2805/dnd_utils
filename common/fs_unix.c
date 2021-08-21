#include "fs_unix.h"

bool checkDir(const char * path) {
    struct stat st = {0};

    // Return false on failure to read path
    if(stat(path, &st) == -1) {
        return false;
    }

    // Otherwise return true iff this is a directory
    return S_ISDIR(st.st_mode);
}

int createDir(const char * path) {
    struct stat st = {0};

    // If no stats at the path, prepare to create a new directory
    if(stat(path, &st) == -1) {
        // Try and create the directory with permissions 0700
        if(mkdir(path, 0700) == -1) {
            // On failure to create the directory, return failure
            return -1;
        }

        // On successful directory creation, return true
        return 0;
    }

    // If this is a directory, return 1
    if(S_ISDIR(st.st_mode)) {
        return 1;
    }
    
    // Otherwise, this exists as a non-directory, return failure
    return -1;
}