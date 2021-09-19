#include "symtable.h"

#include <assert.h>
#include <stdlib.h>

#include "symlist.h"

#define INITIAL_BUCKETS 509
#define INITIAL_SCOPES 10
#define MAX_BUCKETS 65521
#define HASH_MULTIPLIER 65599

/* Static functions that will be used locally only. */
/****************************************************/

/*
 * Hash Function for any element. Does return a Hash
 * entry regardless of oSymTable buckets size.
 */
static unsigned int SymTable_hash(const char *pcKey);

/* Compares the 2 given strings. */
static int ms_compare(const char *s1, const char *s2);

/*
 * Increases the number of buckets on oSymTable.
 * Asserts oSymTable != NULL and bucket_count == size
 * for the given oSymTable.
 */
static void increase_buckets(SymTable_T oSymTable);

/*
 * Copies src string into dest. dest must already have
 * allocated memory, vulnerable to segmentation faults.
 */
static char *ms_copy(char *dest, const char *src);

/*
 * Calculates and returns the length of the given string.
 * Asserts pcStr != NULL
 */
static size_t ms_length(const char *pcStr);

/*
 * Insert given entry on given scope list of given symbol table.
 * Expands the scope lists table of given symbol table if needed.
 */
static void scope_insert(SymTable_T symTable, size_t scope,
                         SymbolTableEntry *entry);


SymTable_T SymTable_new(void) {
    int i;
    SymTable_T new_table = malloc(sizeof(struct SymTable));
    new_table->size = INITIAL_BUCKETS;
    new_table->buckets = malloc(INITIAL_BUCKETS * sizeof(node_t));
    new_table->binding_count = 0;
    /* Initialize each bucket with NULL pointer. */
    for (i = 0; i < INITIAL_BUCKETS; i++) {
        new_table->buckets[i] = NULL;
    }

    new_table->scopes = (SymList_T *)malloc(sizeof(SymList_T) * INITIAL_SCOPES);
    new_table->scopes_size = INITIAL_SCOPES;
    for (i = 0; i < new_table->scopes_size; i++)
        new_table->scopes[i] = SymList_new();

    return new_table;
}

void SymTable_init_lib_functions(SymTable_T oSymTable) {
    SymTable_put(oSymTable, "print",
                 create_SymbolTableEntry("print", NULL, 0, 0, LIBFUNC_SYMBOL));
    SymTable_put(oSymTable, "input",
                 create_SymbolTableEntry("input", NULL, 0, 0, LIBFUNC_SYMBOL));
    SymTable_put(oSymTable, "objectmemberkeys",
                 create_SymbolTableEntry("objectmemberkeys", NULL, 0, 0,
                                         LIBFUNC_SYMBOL));
    SymTable_put(oSymTable, "objecttotalmembers",
                 create_SymbolTableEntry("objecttotalmembers", NULL, 0, 0,
                                         LIBFUNC_SYMBOL));
    SymTable_put(
        oSymTable, "objectcopy",
        create_SymbolTableEntry("objectcopy", NULL, 0, 0, LIBFUNC_SYMBOL));
    SymTable_put(
        oSymTable, "totalarguments",
        create_SymbolTableEntry("totalarguments", NULL, 0, 0, LIBFUNC_SYMBOL));
    SymTable_put(
        oSymTable, "argument",
        create_SymbolTableEntry("argument", NULL, 0, 0, LIBFUNC_SYMBOL));
    SymTable_put(oSymTable, "typeof",
                 create_SymbolTableEntry("typeof", NULL, 0, 0, LIBFUNC_SYMBOL));
    SymTable_put(
        oSymTable, "strtonum",
        create_SymbolTableEntry("strtonum", NULL, 0, 0, LIBFUNC_SYMBOL));
    SymTable_put(oSymTable, "sqrt",
                 create_SymbolTableEntry("sqrt", NULL, 0, 0, LIBFUNC_SYMBOL));
    SymTable_put(oSymTable, "cos",
                 create_SymbolTableEntry("cos", NULL, 0, 0, LIBFUNC_SYMBOL));
    SymTable_put(oSymTable, "sin",
                 create_SymbolTableEntry("sin", NULL, 0, 0, LIBFUNC_SYMBOL));
}

void SymTable_free(SymTable_T oSymTable) {
    size_t N, i;
    node_t curr_node, prev_node;
    if (!oSymTable) return;
    if (!oSymTable->buckets) /* This is practically No-Man's land, but I simply
                                return from this function since afterall we make
                                no assertions at all. */
        return;
    N = oSymTable->size;
    for (i = 0; i < N; i++) {
        curr_node = oSymTable->buckets[i];
        prev_node = curr_node;
        while (curr_node) { /* Free allocated memory for node's key and the node
                               itself. */
            curr_node = curr_node->next;
            free(prev_node->key);
            free(prev_node);
            prev_node = curr_node;
        }
    }
    for (i = 0; i < oSymTable->scopes_size; i++)
        SymList_destruct(oSymTable->scopes[i]);
    free(oSymTable->scopes);
    free(oSymTable->buckets);
    free(oSymTable);
}

unsigned int SymTable_getLength(
    SymTable_T oSymTable) { /* Returns node count. */
    assert(oSymTable);
    return oSymTable->binding_count;
}

int SymTable_put(SymTable_T oSymTable, const char *pcKey,
                 SymbolTableEntry *pvValue) {
    size_t index;
    node_t curr_node;
    assert(oSymTable);
    assert(pcKey);
    /* If binding_count reached size and size hasnt reached MAX_BUCKETS yet, we
       increase the number of total buckets for the Bonus part of this
       assignment. */
    if (oSymTable->binding_count == oSymTable->size &&
        oSymTable->size != MAX_BUCKETS)
        increase_buckets(oSymTable);
    index = SymTable_hash(pcKey) % oSymTable->size;
    oSymTable->binding_count++;
    curr_node = malloc(sizeof(struct node));
    curr_node->entry = pvValue;
    curr_node->key = malloc((ms_length(pcKey) + 1) * sizeof(char));
    ms_copy(curr_node->key, pcKey);
    curr_node->next = oSymTable->buckets[index];
    oSymTable->buckets[index] = curr_node;
    scope_insert(oSymTable, curr_node->entry->scope, curr_node->entry);
    return 1;
}

int SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
    size_t index = SymTable_hash(pcKey) % oSymTable->size;
    node_t curr_node, prev_node = NULL;
    for (curr_node = oSymTable->buckets[index]; curr_node;
         curr_node = curr_node->next) { /* check every node on that bucket. If
                                       found, we need to remove the node, update
                                       bucket_count if necessary and return 1 */
        if (ms_compare(curr_node->key, pcKey) == 0) {
            if (prev_node)
                prev_node->next = curr_node->next;
            else {
                oSymTable->buckets[index] = curr_node->next;
            }
            oSymTable->binding_count--;
            free(curr_node->key);
            free(curr_node);
            return 1;
        }
        prev_node = curr_node;
    }

    return 0;
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey,
                      unsigned int scope) {
    size_t index;
    node_t curr_node;
    assert(oSymTable);
    assert(pcKey);
    index = SymTable_hash(pcKey) % oSymTable->size;
    for (curr_node = oSymTable->buckets[index]; curr_node;
         curr_node = curr_node->next)
        if (ms_compare(curr_node->key, pcKey) == 0 &&
            scope == curr_node->entry->scope && curr_node->entry->isActive)
            /* If we found a node with matching info we return 1 */
            return 1;
    return 0;
}

SymbolTableEntry *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    size_t index;
    node_t curr_node;
    assert(oSymTable);
    assert(pcKey);
    index = SymTable_hash(pcKey) % oSymTable->size;
    for (curr_node = oSymTable->buckets[index]; curr_node;
         curr_node = curr_node->next)
        if (ms_compare(curr_node->key, pcKey) == 0 &&
            curr_node->entry->isActive == 1)
            return curr_node->entry; /* If we found a node with a matching
                                        string we return its pvValue */
    return NULL;
}

SymbolTableEntry *SymTable_get_entry_at_scope(SymTable_T oSymTable,
                                              const char *key,
                                              const unsigned int scope) {
    assert(oSymTable);
    if (scope >= oSymTable->scopes_size) return NULL;
    list_entry_T *iter = oSymTable->scopes[scope]->head;

    while (iter) {
        if (!ms_compare(iter->entry->name, key) && iter->entry->isActive) {
            return iter->entry;
        }
        iter = iter->next;
    }
    return NULL;
}

SymbolTableEntry *SymTable_get_closest_scope(SymTable_T oSymTable,
                                             const char *pcKey,
                                             unsigned int scope) {
    long index;
    list_entry_T *curr_node;

    assert(oSymTable);
    assert(pcKey);

    for (index = scope; index >= 0; --index) {  // FIXME: three uses in
                                                // parser.y nedded "index > 0"
        curr_node = oSymTable->scopes[index]->head;
        while (curr_node) {
            if (ms_compare(curr_node->entry->name, pcKey) == 0 &&
                curr_node->entry->isActive == 1)
                return curr_node->entry; /* If we found a node with a matching
                                        string we return its pvValue */
            curr_node = curr_node->next;
        }
    }
    return NULL;
}

int SymTable_open_func(SymTable_T oSymTable, SymbolTableEntry *entry,
                       int max_scope) {
    int i;
    list_entry_T *curr_node;
    assert(oSymTable);
    assert(entry);
    i = entry->scope;
    curr_node = oSymTable->scopes[i]->head;
    while (curr_node->entry != entry) {
        curr_node = curr_node->next;
    }
    for (; i < max_scope; i++) {
        while (curr_node != NULL) {
            if (curr_node->entry->func_is_open) return 1;
            curr_node = curr_node->next;
        }
        curr_node = oSymTable->scopes[i + 1]->head;
    }
    while (curr_node != NULL) {
        if (curr_node->entry->func_is_open) return 1;
        curr_node = curr_node->next;
    }
    return 0;
}

SymbolTableEntry *SymTable_get_open_func(SymTable_T oSymTable,
                                         unsigned int index) {
    int i;
    list_entry_T *curr_node;
    assert(oSymTable);
    for (i = index; i >= 0; --i) {
        curr_node = oSymTable->scopes[i]->tail;
        while (curr_node != NULL) {
            if (curr_node->entry->func_is_open) return curr_node->entry;
            curr_node = curr_node->prev;
        }
    }
    return NULL;
}

void SymTable_map(SymTable_T oSymTable,
                  void (*pfApply)(const char *pcKey, SymbolTableEntry *pvValue,
                                  void *pvExtra),
                  void *pvExtra) {
    size_t i;
    node_t curr_node;
    assert(oSymTable);
    assert(pfApply);
    for (i = 0; i < oSymTable->size; i++)
        for (curr_node = oSymTable->buckets[i]; curr_node;
             curr_node = curr_node->next) /* Apply to every node pfApply func */
            pfApply(curr_node->key, curr_node->entry, pvExtra);
}

static unsigned int SymTable_hash(
    const char *pcKey) { /* Simple Hash Function */
    size_t ui;
    unsigned int uiHash = 0U;
    for (ui = 0U; pcKey[ui] != '\0'; ui++)
        uiHash = uiHash * HASH_MULTIPLIER + pcKey[ui];
    return uiHash;
}

/*
 * strcmp - Compares the 2 given string and returns -1, 0 and 1 if s1 is found
 * respectively to be less than, to match or to be greater than s2 - asserts not
 * null pointers.
 */
static int ms_compare(const char *s1, const char *s2) {
    const char *s1_curr = s1, *s2_curr = s2;
    /* Assert not null pointers */
    assert(s1);
    assert(s2);
    /* While none of them reached null byte, we compare their characters */
    while (*s1_curr && *s2_curr)
        if (*(s1_curr) < *(s2_curr))
            return -1;
        else if (*(s1_curr++) > *(s2_curr++))
            return 1;

    /* If s1_curr != '\0' return 1 (len(s1_curr) > len(s2_curr)) */
    if (*s1_curr) return 1;
    /* If s2_curr != '\0' return -1 (len(s1_curr) < len(s2_curr)) */
    if (*s2_curr) return -1;
    return 0;
}

/*
 * Increases the number of available buckets in hashtable. Asserts
 * oSymTable->size == oSymTable->binding_count.
 */
static void increase_buckets(SymTable_T oSymTable) {
    size_t i, old_N = oSymTable->size, N, index;
    node_t *new_buckets, curr_node, prev_node;
    assert(oSymTable);
    assert(oSymTable->size == oSymTable->binding_count);
    switch (oSymTable->size) {
        case INITIAL_BUCKETS:
            oSymTable->size = 1021;
            break;
        case 1021:
            oSymTable->size = 2053;
            break;
        case 2053:
            oSymTable->size = 4093;
            break;
        case 4093:
            oSymTable->size = 8191;
            break;
        case 8191:
            oSymTable->size = 16381;
            break;
        case 16381:
            oSymTable->size = 32771;
            break;
        case 32771:
            oSymTable->size = MAX_BUCKETS;
            break;
        default:
            /* No man's land */
            assert(0);
            break;
    }
    N = oSymTable->size;
    new_buckets = malloc(N * sizeof(node_t));
    for (i = 0; i < N; i++) new_buckets[i] = NULL;
    for (i = 0; i < old_N; i++) {
        curr_node = oSymTable->buckets[i];
        while (curr_node) { /* New hash function, therefore we need to reassign
                               nodes to their proper buckets. */
            index = SymTable_hash(curr_node->key) % N;
            prev_node = curr_node;
            curr_node = curr_node->next;
            prev_node->next = new_buckets[index];
            new_buckets[index] = prev_node;
        }
    }
    free(oSymTable->buckets);
    oSymTable->buckets = new_buckets;
}

/*
 * strcpy - Copies src string into dest. Asserts not null arguments, careful for
 * buffer overrun! Does not check if there is enough memory allocated for dest
 * parameter - asserts not null pointers.
 */
static char *ms_copy(char *dest, const char *src) {
    const char *src_curr = src;
    char *dest_curr = dest;
    /* Assert that our pointers are not NULL pointers */
    assert(dest);
    assert(src);
    /* Increase each pointer by 1 after assignment */
    while (*src_curr) *(dest_curr++) = *(src_curr++);
    /* include \0 at the end of dest string */
    *dest_curr = *src_curr;
    return dest;
}

/*
 * strlen - returns the number of characters up until null byte '\0'. Asserts
 * not null input.
 */
static size_t ms_length(const char *pcStr) {
    const char *pcStrEnd = pcStr;

    assert(pcStr);
    /* while pcstrend != '\0' */
    while (*pcStrEnd) pcStrEnd++;
    /* sizeof(char) == 1, so their memory difference is the string's length :)
     */
    return pcStrEnd - pcStr;
}

/*
 * Insert given entry on given scope list of given symbol table.
 * Expands the scope lists table of given symbol table if needed.
 */
static void scope_insert(SymTable_T symTable, size_t scope,
                         SymbolTableEntry *entry) {
    /* expand the scopes table if needed */
    if (scope >= symTable->scopes_size) {
        size_t difference = (scope + 1) - symTable->scopes_size;
        SymList_T *scopes_table = (SymList_T *)malloc(
            sizeof(SymTable_T) * (symTable->scopes_size + 4 * difference));
        for (int i = 0; i < symTable->scopes_size; i++)
            scopes_table[i] = symTable->scopes[i];
        for (int i = symTable->scopes_size;
             i < symTable->scopes_size + 4 * difference; i++)
            scopes_table[i] = SymList_new();

        free(symTable->scopes);
        symTable->scopes_size = symTable->scopes_size + 4 * difference;
        symTable->scopes = scopes_table;
    }

    SymList_insert(symTable->scopes[scope], entry);
}
