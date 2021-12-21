#include "list.h"

typedef struct node_s {
    void * data;
    struct node_s * next;
} * node_t;

struct list_s {
    unsigned int length;
    node_t start;
    node_t end;
};

//================================<Helpers>================================//
node_t mkNode() {
    return calloc(1, sizeof(struct node_s));
}

void rmNode(node_t node, freeFxn_t freeFxn) {
    if(node == NULL) return;
    
    if(freeFxn != NULL && freeFxn != NULL) {
        freeFxn(node->data);
    }
    
    free(node);
}

void rmNodes(node_t first, freeFxn_t freeFxn) {
    if(first == NULL) return;

    rmNodes(first->next, freeFxn);
    rmNode(first, freeFxn);
}

node_t seekNode(list_t list, unsigned int idx) {
    if(list == NULL) return NULL;
    if(idx >= list->length) return NULL;

    if(idx == list->length - 1) {
        return list->end;
    }

    node_t node = list->start;
    for(unsigned int i = 0; i < idx; i++) {
        node = node->next;
    }
    return node;
}


//================================<Allocation>================================//
/**
 * Makes a new linked list
 * 
 * @return The linked list created
 */
list_t mkList() {
    return calloc(1, sizeof(struct list_s));
}

/**
 * Frees a list and all of its elements
 * 
 * @param list The list to free
 * @param freeFxn A function used to free all list elements
 */
void rmList(list_t list, freeFxn_t freeFxn) {
    if(list == NULL) return;
    
    rmNodes(list->start, freeFxn);
    free(list);
}

/**
 * Saves the list out to file
 * 
 * @param list The list to save
 * @param fp The file to save to
 * @param writeEntry A function to write an entry to file
 * 
 * @return 0 on success, < 0 on failure
 */
int saveList(list_t list, FILE* fp, int (* writeEntry)(void*, FILE*)) {
    if(fp == NULL || writeEntry == NULL) return -1;

    fprintf(fp, "%u\n", list->length);

    node_t node = list->start;
    while(node != NULL) {
        if(writeEntry(node->data, fp) < 0) {
            return -1;
        }

        node = node->next;
    }

    return 0;
}

/**
 * Reads a list from file
 * 
 * @param list a return pointer for the list
 * @param fp the file to save to
 * @param readEntry A function to read an entry from the current file line
 * 
 * @return 0 on success, <0 on failure
 */
int loadList(list_t * list, FILE* fp, int (*readEntry)(void**, FILE*)) {
    if(list == NULL || fp == NULL || readEntry == NULL) {
        return -1;
    }

    unsigned len;
    if(fscanf(fp, "%u", &len) != 1) {
        return -1;
    }

    *list = mkList();
    for(unsigned i = 0; i < len; i++) {
        void* ent;
        if(readEntry(&ent, fp) < 0) {
            return -1;
        }

        if(listAppend(*list, ent) < 0) {
            return -1;
        }
    }


    return 0;
}


/**
 * Example writeEntry for strings
 * 
 * @param str The string to write
 * @param fp The file to write to
 * 
 * @return 0 on success, <0 on failure
 */
int writeStrEntry(void * str, FILE* fp) {
    if(fp == NULL) return -1;

    if(str == NULL) {
        fprintf(fp, "0 |\n");
        return 0;
    }

    unsigned len = strlen(str);
    fprintf(fp, "%u |%s\n", len, (char *)str);

    return 0;
}

/**
 * Example readEntry for strings
 * 
 * @param str Return pointer for retrieved string
 * @param fp The file to read from
 * 
 * @return 0 on success, <0 on failure
 */
int readStrEntry(void ** str, FILE* fp) {
    if(fp == NULL || str == NULL) return -1;

    unsigned len;
    if(fscanf(fp, "%u |", &len) != 1) {
        return -1;
    }

    *str = calloc(len + 1, sizeof(char));
    if(*str == NULL) return -1;

    fread(*str, 1, len, fp);

    return 0;
}

//=================================<Setters>==================================//
/**
 * Appends an element to the end of the list
 * 
 * @param list The list to append to
 * @param data The data to append to the list
 * 
 * @return The index of the appended item (<0 on failure)
 */
int listAppend(list_t list, void * data) {
    if(list == NULL) return -1;

    return listInsert(list, list->length, data);
}

/**
 * Inserts an element into the list, pushing all successive elements down by one
 * 
 * @param list The list to insert into
 * @param idx The index to insert into
 * @param data The data to insert into the list
 * 
 * @return The index of the inserted item (<0 on failure)
 */
int listInsert(list_t list, unsigned int idx, void * data) {
    if(list == NULL) return -1;
    if(idx > list->length) return -1;

    node_t node = mkNode();
    if(node == NULL) {
        return -1;
    }
    node->data = data;

    
    if(idx == 0) { // Handles insertion to start (and append to empty)
        node->next = list->start;
        list->start = node;

        if(list->end == NULL) list->end = node;
    } else if(idx == list->length) { // Handles append to non-empty
        list->end->next = node;
        list->end = node;
    } else { // Handles insertion into middle
        node_t prev = seekNode(list, idx - 1);
        node->next = prev->next;
        prev->next = node;
    }

    list->length += 1;
    return idx;

}

/**
 * Places the provided data into the list at existing index idx
 * 
 * @param list The list to modify
 * @param idx The index to modify
 * @param data The data to put at the specified index
 * 
 * @return The previous entry at idx (NULL on invalid index)
 */
void * listPut(list_t list, unsigned int idx, void * data) {
    node_t node = seekNode(list, idx);
    if(node == NULL) return NULL;

    void * oldData = node->data;
    node->data = data;
    return oldData;
}

//=================================<Getters>==================================//
/**
 * Gets the element at the provided index
 * 
 * @param list The list to read from
 * @param idx The index to read from
 * 
 * @return The item at the specified index (NULL on invalid index)
 */
void * listGet(list_t list, unsigned int idx) {
    node_t node = seekNode(list, idx);
    if(node == NULL) return NULL;

    return node->data;
}

/**
 * Removes the element at the specified index, moving all further elements up
 * 
 * @param list The list to remove from
 * @param idx The index to remove
 * 
 * @return The item removed from the list
 */
void * listRm(list_t list, unsigned int idx) {
    if(list == NULL) return NULL;
    if(idx >= list->length) return NULL;

    node_t node, prev;
    if(idx == 0) {
        prev = NULL;                    // There is no previous node
        node = list->start;             // Get the first node
        list->start = node->next;       // Set the list's start to the next node
    } else {
        prev = seekNode(list, idx - 1); // Get the previous node
        node = prev->next;              // Get the targeted node
        prev->next = node->next;        // Unlink the target node
    }

    list->length -= 1;                      // Update list length
    if(list->end == node) list->end = prev; // Update list end if changed

    void * data = node->data;   //Grab the target's data
    rmNode(node, NULL);         // Free the node (but not its data)
    return data;                //Return the target's data
}

/**
 * Returns the length of the provided list
 * 
 * @param list The list to query
 * 
 * @return The length of the provided list
 */
unsigned int listLen(list_t list) {
    if(list == NULL) return 0;
    return list->length;
}

/**
 * Returns the index of the first occurence of `obj` in the list
 * 
 * @param list The list to search in
 * @param obj The item to search for
 * 
 * @return The index of obj's first occurence in the list (<0 on failure)
 */
int listFind(list_t list, void* obj) {
    if(list == NULL) return -1;
    
    int i = 0;
    node_t node = list->start;
    while(node != NULL && node->data != obj) {
        ++i;
        node = node->next;
    }

    if(node == NULL) return -1;
    return i;
}