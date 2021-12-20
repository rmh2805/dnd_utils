#ifndef _JSON_H_
#define _JSON_H_

#include <stdlib.h>
#include <stdbool.h>

typedef struct jsonArray_s * jsonArray_t;
typedef struct jsonObject_s * jsonObject_t;

typedef enum jsonType_e {
    kObject, kArray, kString, kInteger, kFraction, kBoolean, kNull
} jsonType_t;

typedef struct jsonValue_s {
    jsonType_t type;
    union {
        jsonObject_t object;
        jsonArray_t array;
        char * string;
        long integer;
        double fraction;
        bool boolean;
    } value;
} jsonValue_t;

//==============================<Init and Free>===============================//
/**
 * Allocates an empty json array
 * 
 * @return An empty json array
 */
jsonArray_t mkJsonArray();

/**
 * Allocates an empty json object
 * 
 * @return An empty json object
 */
jsonObject_t mkJsonObject();

/**
 * Allocates a null json value
 */
jsonValue_t mkJsonValue();

/**
 * Frees a json array (and all of its contents)
 * 
 * @param array The Array to free
 */
void rmJsonArray(jsonArray_t array);

/**
 * Frees a json object (and all of its contents)
 * 
 * @param object The object to free
 */
void rmJsonObject(jsonObject_t object);

/**
 * Frees a json Value
 */
void rmJsonValue(jsonValue_t value);

//===========================<Getters and Setters>============================//

/**
 * Sets a value in the json array at a given index
 * 
 * @param array the array to modify
 * @param idx The index to set
 * @param value The value to set
 * 
 * @return 0 on success, <0 on failure
 */
int setArrayIdx(jsonArray_t array, size_t idx, jsonValue_t value);

/**
 * Gets a value in the json array at a given index
 * 
 * @param array The array to access
 * @param idx The index to access
 * @param value A return pointer for the grabbed value
 * 
 * @return 0 on success, <0 on failure
 */
int getArrayIdx(jsonArray_t array, size_t idx, jsonValue_t * value);

/**
 * Sets the value from the object for the requested key
 * 
 * @param object The object to query
 * @param key The key to access
 * @param value The value to set
 * 
 * @return 0 on success, <0 on failure
 */
int setObjectValue(jsonObject_t object, const char * key, jsonValue_t value);

/**
 * Gets the value from the object for the requested key
 * 
 * @param object The object to query
 * @param key The key to access
 * @param value A return pointer for the requested value
 * 
 * @return 0 on success, <0 on failure
 */
int getObjectValue(jsonObject_t object, const char * key, jsonValue_t * value);

/**
 * Gets the length of a json value
 * 
 * @param value The jsonValue to query
 * 
 * @return The number of entries within that value (0 for kNull, 1 for numbers, 
 *         bools, and strings, variable for arrays and objects, and <0 on error)
 */
size_t getValueLength(jsonValue_t value);

#endif