#include "json.h"

struct jsonArray_s {
    size_t length;
    size_t size;
    jsonValue_t * data;
};

typedef struct jsonPair_s {
    char * key;
    jsonValue_t value;
} jsonPair_t;

struct jsonObject_s {
    jsonArray_t keys;
    jsonArray_t values;
};

//==============================<Init and Free>===============================//
jsonArray_t mkJsonArray() {
    jsonArray_t array = malloc(sizeof(struct jsonArray_s));
    if(array == NULL) {
        return NULL;
    }

    array->length = 0;
    array->size = 0;
    array->data = NULL;
    return array;
}

jsonObject_t mkJsonObject() {
    jsonObject_t object = malloc(sizeof(struct jsonObject_s));
    if(object == NULL) {
        return NULL;
    }

    object->keys = mkJsonArray();
    object->values = mkJsonArray();

    if(object->keys == NULL || object->values == NULL) {
        rmJsonObject(object);
        return NULL;
    }

    return object;
}

jsonValue_t mkJsonValue(){
    jsonValue_t value;
    for(size_t i = 0; i < sizeof(value); ++i) {
        ((char *)&value)[i] = 0;
    }
    value.type = kNull;
    return value;
}

void rmJsonArray(jsonArray_t array) {
    if(array == NULL) {
        return;
    }

    if(array->data != NULL) {
        for(size_t i = 0; i < array->length; i++) {
            rmJsonValue(array->data[i]);
        }
        free(array->data);
    }

    free(array);
}

void rmJsonObject(jsonObject_t object) {
    if(object == NULL) {
        return;
    }

    rmJsonArray(object->keys);
    rmJsonArray(object->values);
}

void rmJsonValue(jsonValue_t value) {
    switch(value.type) {
        case kObject:
            rmJsonObject(value.value.object);
            break;
        case kArray:
            rmJsonArray(value.value.array);
            break;
        case kString:
            free(value.value.string);
            break;
    }
}

//===========================<Getters and Setters>============================//

int setArrayIdx(jsonArray_t array, size_t idx, jsonValue_t value) {
    if(array == NULL) {
        return -1;
    }

    // If no data has been allocated, allocate a basic block
    if(array->data == NULL) {
        array->data = calloc(4, sizeof(jsonValue_t));
        if(array->data == NULL) {
            return -1;
        }
        array->size = idx + 1;
    } 
    
    // Double the size of the array until idx fits inside
    while (idx >= array->size) {
        // Try to double the array's allocated space
        jsonValue_t * newData = realloc(array->data, 
            (array->size * sizeof(jsonValue_t)) * 2);

        // If the reallocation failed, fail out
        if(newData == NULL) {
            return -2;
        }

        array->data == newData;
    }

    // Fill any gaps from data[length] to data[idx-1] with kNull jsonValues
    for(int i = array->length; i < idx; i++) {
        array->data[i] = mkJsonValue();
    }

    // Set the new value in the index
    array->data[idx] = value;

    // length <- max(length, idx + 1)
    if(idx + 1 > array->length) {
        array->length = idx + 1;
    }
    return 0;
}

int getArrayIdx(jsonArray_t array, size_t idx, jsonValue_t * value) {
    if(array == NULL || array->data == NULL || idx >= array->length) {
        return -1;
    }
    
    *value = array->data[idx];
    return 0;
}

int setObjectValue(jsonObject_t object, const char * key, jsonValue_t value) {
    return -1;
}

int getObjectValue(jsonObject_t object, const char * key, jsonValue_t * value) {
    return -1;
}

size_t getValueLength(jsonValue_t value) {
    switch(value.type) {
        case kNull:
            return 0;
        case kArray:
            jsonArray_t array = value.value.array;
            if(array == NULL) {
                return -1;
            }
            return array->length;
        case kObject:
            jsonObject_t object = value.value.object;
            if(object == NULL || object->keys == NULL) {
                return -1;
            }
            return object->keys->length;
        default:
            return 1;
    }
}
