#ifndef _FS_MODULE_H_
#define _FS_MODULE_H_

#include <stdbool.h>

// Include unix file system manipulation libraries
#include <sys/stat.h>
#include <sys/types.h>

/**
 * Returns true iff the provided path ends with a directory
 * 
 * @param path The path to check
 * @return true iff the provided path ends in a directory
 */
bool checkDir(const char * path);

/**
 * Attempts to create a new directory with the provided path if one doesn't 
 * alreadt exist
 * 
 * @param path The path of the directory to create
 * @return An exit status (0 on success, <0 on failure, 1 if directory exists)
 */
int createDir(const char * path);

#endif