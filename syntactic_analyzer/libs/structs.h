#pragma once
#include "symlist.h"
extern int nameless_func_count;

typedef struct SymList *SymList_T;
typedef struct Variable Variable;
typedef struct Function Function;
typedef struct SymbolTableEntry SymbolTableEntry;

typedef enum {
    GLOBAL_SYMBOL = 1,
    LOCAL_SYMBOL,
    FORMAL_SYMBOL,
    USERFUNC_SYMBOL,
    LIBFUNC_SYMBOL
} SymbolType;

struct SymbolTableEntry {
    int isActive;
    char *name;
    SymList_T list_of_arguments;
    int func_is_open;
    unsigned int scope;
    unsigned int line;
    //TODO: unsigned int offset; //probably needed for offset in scope space
    SymbolType type;
};


SymbolTableEntry *create_SymbolTableEntry(char *function_name,
                                          SymList_T argument_list,
                                          unsigned int function_scope,
                                          unsigned int declaration_line,
                                          SymbolType symbol_type);
