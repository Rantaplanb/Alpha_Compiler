#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>

#include "arraylist.h"
#include "stack.h"
#include "symtable.h"

/* For scanner.l */
extern unsigned int current_scope;
extern Stack_T *comment_stack;
extern Stack_T *block_stack;
extern Stack_T *function_offset_stack;
extern SymTable_T symbol_table;
extern FILE *scopes_file;

/* For parser.y */
void yyerror(const char *yacc_error_message);
int yylex(void);
extern int yylineno;
extern char *yytext;
extern FILE *yyin;
extern int IR_GEN_FAILED;

/* For byte code generation */
extern ArrayList *instructions;
extern ArrayList *userfunctions;
extern ArrayList *const_strings;
extern ArrayList *libfuncs_used;
extern ArrayList *const_numbers;

#endif
