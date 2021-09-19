//
// Created by georg on 6/6/2021.
//

#ifndef ALPHAC_DYNAMIC_ARRAY_H
#define ALPHAC_DYNAMIC_ARRAY_H
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct dynamic_array {
    void **element;
    unsigned int current_size;
    unsigned int max_size;
} ArrayList;

ArrayList *create_arraylist();

void insert_arraylist(ArrayList *array, void *element);

void *get_arraylist(ArrayList *array, unsigned int index);

extern void copy_bytes(void *src, void *dest, size_t n);

#endif  // ALPHAC_DYNAMIC_ARRAY_H
