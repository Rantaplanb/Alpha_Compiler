#include "arraylist.h"

#include <stdlib.h>
#ifndef EXPAND_SIZE_T
#define EXPAND_SIZE_T 1024
#endif

static void resize_arraylist(ArrayList *array);

ArrayList *create_arraylist() {
    ArrayList *new_array = malloc(sizeof(ArrayList));
    new_array->element = NULL;
    new_array->current_size = 0;
    new_array->max_size = 0;
    return new_array;
}

void insert_arraylist(ArrayList *array, void *element) {
    if (array->current_size >= array->max_size) resize_arraylist(array);
    array->element[array->current_size++] = element;
}

void *get_arraylist(ArrayList *array, unsigned int index) {
    assert(index < array->current_size);
    return array->element[index];
}

static void resize_arraylist(ArrayList *array) {
    void *resized = malloc(EXPAND_SIZE_T * sizeof(void *) +
                           array->max_size * sizeof(void *));
    if (!resized) {
        printf("memory allocation failed");
    }
    copy_bytes(array->element, resized, array->max_size * sizeof(void *));
    if (array->element) free(array->element);
    array->max_size = array->max_size + EXPAND_SIZE_T;
    array->element = resized;
}
