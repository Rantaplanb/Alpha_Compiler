/***************************************************
 *                                                 *
 * file:    symlist.c                              *
 *                                                 *
 * Inplementation for symlist.h                    *
 *                                                 *
 ***************************************************
 */

#include "symlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
 * Make and return a new SymList.
 */
SymList_T SymList_new() {
    SymList_T list = (SymList_T) malloc(sizeof(struct SymList));
    list->head = NULL;
    list->tail = NULL;
    list->length = 0;

    return list;
}


/*
 * Insert given entry in the end of given list.
 */
void SymList_insert(SymList_T list, SymbolTableEntry *entry) {
    list_entry_T *new_node = (list_entry_T *) malloc(sizeof(struct SymList));
    new_node->entry = entry;
    new_node->next = NULL;

    if (list->tail)
        list->tail->next = new_node;
    else
        list->head = new_node;

    list->tail = new_node;

    list->length++;

    return;
}

SymbolTableEntry *SymList_get(SymList_T list, const char *name){
    list_entry_T *iter;
    if(!list)
        return NULL;
    iter = list->head;
    while (iter) {
        if(!strcmp(iter->entry->name, name) && iter->entry->isActive)
            return iter->entry;
        iter = iter->next;
    }
    return NULL;
}
/*
 * Activate all entries on given list.
 */
void SymList_activate(SymList_T list) {
    list_entry_T *iter = list->head;

    while (iter) {
        iter->entry->isActive = 1;
        iter = iter->next;
    }

    return;
}


/*
 * Deactivate all entries on given list.
 */
void SymList_deactivate(SymList_T list) {
    list_entry_T *iter = list->head;

    while (iter) {
        iter->entry->isActive = 0;
        iter = iter->next;
    }

    return;
}


/*
 * Apply given function on all entries of the list.
 */
void SymList_apply_all(SymList_T list, void (*func)(void *entry)) {
    list_entry_T *iter = list->head;
    while (iter) {
        func(iter->entry);
        iter = iter->next;
    }

    return;
}


/*
 * Delete and free given list.
 * (!) Does not free entries.
 * (!) For that use SymList_apply_all(list, free).
 */
void SymList_destruct(SymList_T list) {
    list_entry_T *iter = list->head, *next;

    while (iter) {
        next = iter->next;
        iter->next = NULL;
        free(iter);

        iter = next;
    }

    free(list);

    return;
}
