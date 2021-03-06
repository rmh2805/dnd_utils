#ifndef _LIST_H_
#define _LIST_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct list_s * list_t;
typedef void (*freeFxn_t)(void * data);

//==============================<Alloc and Free>==============================//
/**
 * Makes a new linked list
 * 
 * @return The linked list created
 */
list_t mkList();

/**
 * Frees a list and all of its elements
 * 
 * @param list The list to free
 * @param freeFxn A function used to free all list elements
 */
void rmList(list_t list, freeFxn_t freeFxn);

/**
 * Saves the list out to file
 * 
 * @param list The list to save
 * @param fp The file to save to
 * @param writeEntry A function to write an entry to file
 * 
 * @return 0 on success, < 0 on failure
 */
int saveList(list_t list, FILE* fp, int (* writeEntry)(void*, FILE*));

/**
 * Reads a list from file
 * 
 * @param list a return pointer for the list
 * @param fp the file to save to
 * @param readEntry A function to read an entry from the current file line
 * 
 * @return 0 on success, <0 on failure (does not self-clean on failure)
 */
int loadList(list_t * list, FILE* fp, int (*readEntry)(void**, FILE*));

/**
 * Example writeEntry for strings
 * 
 * @param str The string to write
 * @param fp The file to write to
 * 
 * @return 0 on success, <0 on failure
 */
int writeStrEntry(void * str, FILE* fp);

/**
 * Example readEntry for strings
 * 
 * @param str Return pointer for retrieved string
 * @param fp The file to read from
 * 
 * @return 0 on success, <0 on failure
 */
int readStrEntry(void ** str, FILE* fp);

//=================================<Setters>==================================//
/**
 * Appends an element to the end of the list
 * 
 * @param list The list to append to
 * @param data The data to append to the list
 * 
 * @return The index of the appended item (<0 on failure)
 */
int listAppend(list_t list, void * data);

/**
 * Inserts an element into the list, pushing all successive elements down by one
 * 
 * @param list The list to insert into
 * @param idx The index to insert into
 * @param data The data to insert into the list
 * 
 * @return The index of the inserted item (<0 on failure)
 */
int listInsert(list_t list, unsigned int idx, void * data);

/**
 * Places the provided data into the list at existing index idx
 * 
 * @param list The list to modify
 * @param idx The index to modify
 * @param data The data to put at the specified index
 * 
 * @return The previous entry at idx (NULL on invalid index)
 */
void * listPut(list_t list, unsigned int idx, void * data);

//=================================<Getters>==================================//
/**
 * Gets the element at the provided index
 * 
 * @param list The list to read from
 * @param idx The index to read from
 * 
 * @return The item at the specified index (NULL on invalid index)
 */
void * listGet(list_t list, unsigned int idx);

/**
 * Removes the element at the specified index, moving all further elements up
 * 
 * @param list The list to remove from
 * @param idx The index to remove
 * 
 * @return The item removed from the list
 */
void * listRm(list_t list, unsigned int idx);

/**
 * Returns the length of the provided list
 * 
 * @param list The list to query
 * 
 * @return The length of the provided list
 */
unsigned int listLen(list_t list);

/**
 * Returns the index of the first occurence of `obj` in the list
 * 
 * @param list The list to search in
 * @param obj The item to search for
 * 
 * @return The index of obj's first occurence in the list (<0 on failure)
 */
int listFind(list_t list, void* obj);


#endif