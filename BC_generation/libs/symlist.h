#ifndef SYMLIST_H
#define SYMLIST_H

/***************************************************
 *                                                 *
 * file:    symlist.h                              *
 *                                                 *
 * ADT for a basic symbol list                     *
 *                                                 *
 ***************************************************
 */

#pragma once
#include "symtable.h"

typedef struct SymbolTableEntry SymbolTableEntry;

typedef struct list_entry {
    SymbolTableEntry *entry;
    struct list_entry *next;
    struct list_entry *prev;
} list_entry_T;

typedef struct SymList *SymList_T;
struct SymList {
    list_entry_T *head;
    list_entry_T *tail;
    unsigned int length;
};


/*
 * Make and return a new SymList.
 */
SymList_T SymList_new();

/*
 * Insert given entry in the end of given list.
 */
void SymList_insert(SymList_T list, SymbolTableEntry *entry);

/*
 * Get the first (from head to tail) symbol table entry with given <name> field
 * from given list.
 * Returns NULL if no such entry is found.
 */
SymbolTableEntry *SymList_get(SymList_T list, const char *name);

/*
 * Get the first (from tail to head) symbol table entry with given <name> field
 * from given list.
 * Returns NULL if no such entry is found.
 */
SymbolTableEntry *SymList_get_inv(SymList_T list, const char *name);

/*
 * Activate all entries on given list.
 */
void SymList_activate(SymList_T list);

/*
 * Deactivate all entries on given list.
 */
void SymList_deactivate(SymList_T list);

/*
 * Apply given function on all entries of the list.
 */
void SymList_apply_all(SymList_T list, void (*func)(void *entry));

/*
 * Delete and free given list.
 * (!) Does not free entries.
 * (!) For that use SymList_apply_all(list, free).
 */
void SymList_destruct(SymList_T list);

#endif
