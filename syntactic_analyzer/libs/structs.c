#include "structs.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int nameless_func_count = 0;


SymbolTableEntry *create_SymbolTableEntry(char *name,
                                          SymList_T argument_list,
                                          unsigned int scope,
                                          unsigned int declaration_line,
                                          SymbolType symbol_type) {
    SymbolTableEntry *new_entry = malloc(sizeof(SymbolTableEntry));
    if (name)
        new_entry->name = strdup(name);
    else {
        new_entry->name = malloc(10 * sizeof(char));
        sprintf(new_entry->name, "$%d", nameless_func_count++);
    }
    if(symbol_type == USERFUNC_SYMBOL)
        new_entry->func_is_open = 1;
    else
        new_entry->func_is_open = 0;
    new_entry->scope = scope;
    new_entry->line = declaration_line;
    new_entry->isActive = 1;
    new_entry->type = symbol_type;
    new_entry->list_of_arguments = argument_list;
    return new_entry;
}
