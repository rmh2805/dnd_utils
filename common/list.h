#ifndef _LIST_H_
#define _LIST_H_

#include <stdlib.h>

typedef struct list_s * list_t;
typedef void (*freeFxn_t)(void * data);

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


#endif