#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdio.h>

extern int nameless_func_count;
typedef struct LabelList LabelList;
typedef struct SymList *SymList_T;
typedef struct Variable Variable;
typedef struct Function Function;
typedef struct SymbolTableEntry SymbolTableEntry;
extern LabelList *create_list(unsigned int label);

typedef enum { program_variable, function_local, formal_argument } scope_space;

typedef enum {
    GLOBAL_SYMBOL = 1,
    LOCAL_SYMBOL,
    FORMAL_SYMBOL,
    USERFUNC_SYMBOL,
    LIBFUNC_SYMBOL
} SymbolType;

/* SymTable Data Structure */

/* Nodes contained in each bucket */
typedef struct node {
    char *key;
    SymbolTableEntry *entry;
    struct node *next;
} * node_t;

/*
 * size -> the current number of buckets contained in SymTable.
 * bucket_count -> the number of non-NULL buckets.
 * buckets -> pointer to buckets.
 */
struct SymTable {
    size_t size;
    size_t binding_count;
    node_t *buckets;
    SymList_T *scopes;
    size_t scopes_size;
};
typedef struct SymTable *SymTable_T;

struct SymbolTableEntry {
    int isActive;
    char *name;
    SymList_T list_of_arguments;
    int func_is_open;
    scope_space space;
    unsigned int scope;
    unsigned int line;
    unsigned int offset;
    unsigned int taddress;
    LabelList *address_list;
    SymbolType type;
};

/*
 * Create and return a SymbolTableEntry
 */
SymbolTableEntry *create_SymbolTableEntry(char *function_name,
                                          SymList_T argument_list,
                                          unsigned int function_scope,
                                          unsigned int declaration_line,
                                          SymbolType symbol_type);

/*
 * Create and return a new allocated SymTable_T
 */
SymTable_T SymTable_new(void);

/*
 * Initialize given symbol table with library function definitions.
 */
void SymTable_init_lib_functions(SymTable_T oSymTable);

/*
 * Free allocated memory for oSymTable. If oSymTable == NULL do nothing
 */
void SymTable_free(SymTable_T oSymTable);

/*
 * Count and return the number of saved elements
 */
unsigned int SymTable_getLength(SymTable_T oSymTable);

/*
 * Insert element to oSymTable. Assert oSymTable and pcKey not NULL and return 1
 * on success, or if element with the same string as pcKey exists and is active
 * return 0.
 */
int SymTable_put(SymTable_T oSymTable, const char *pcKey,
                 SymbolTableEntry *pvValue);

/*
 * Look up and remove element with the same string as pcKey. Return 1 on
 * success, 0 if not found.
 */
int SymTable_remove(SymTable_T oSymTable, const char *pcKey);

/*
 * Look up and return 1 if element with the same string as pcKey exists, or else
 * 0.
 */
int SymTable_contains(SymTable_T oSymTable, const char *pcKey,
                      unsigned int scope);

/*
 * Look up and return pvValue of element with the same string as pcKey from
 * oSymTable if found, or else NULL. Assert oSymTable and pcKey not being NULL.
 */
SymbolTableEntry *SymTable_get(SymTable_T oSymTable, const char *pcKey);

/*
 * Get first valid SymbolTableEntry with given key from given scope.
 * Return NULL if no such entry exists.
 */
SymbolTableEntry *SymTable_get_entry_at_scope(SymTable_T oSymTable,
                                              const char *key,
                                              const unsigned int scope);

SymbolTableEntry *SymTable_get_closest_scope(SymTable_T oSymTable,
                                             const char *pcKey,
                                             unsigned int scope);

int SymTable_open_func(SymTable_T oSymTable, SymbolTableEntry *entry,
                       int max_scope);

SymbolTableEntry *SymTable_get_open_func(SymTable_T oSymTable,
                                         unsigned int index);

void SymTable_append_addresslist(SymbolTableEntry *entry, unsigned int label);

/*
 * Apply to all oSymTable elements the function pfApply. Assert oSymTable and
 * pfApply not being NULL.
 */
void SymTable_map(SymTable_T oSymTable,
                  void (*pfApply)(const char *pcKey, SymbolTableEntry *pvValue,
                                  void *pvExtra),
                  void *pvExtra);


#endif
