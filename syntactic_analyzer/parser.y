%{
    #include <stdio.h>
    #include <stdlib.h>

    #include "./libs/stack.h"
    #include "libs/structs.h"
    #include "libs/symtable.h"
    #define FUNC_TYPE 1
    #define VAR_TYPE 2

    void yyerror(char *yacc_error_message);
    int yylex(void);
    extern int yylineno;
    extern char *yytext;
    extern FILE *yyin;
    extern Stack_T *comment_stack;
    extern Stack_T *block_stack;
    extern SymTable_T symbol_table;
    extern unsigned int current_scope;
    static void ArgList_to_SymTable(SymTable_T sym_table, SymList_T arg_list);
    static void print_node_if_active(void *node);
    static int insert_variable(SymbolTableEntry *);
    void print_active_scopes(SymTable_T oSymTable);
    static int global_only;
    SymList_T arguments_list;
    static int ERROR_FLAG = 0;


    void print_elem(const char *pcKey, SymbolTableEntry *pvValue,
                    void *pvExtra);
%}

%start program

/* contains all the types that symbols (terminals or nonterminals) can get */
%union {
char *string;
int integer;
double real;
struct SymbolTableEntry *symbol_table_entry;
unsigned int expression_type;
}

/* termatika sumvola dilwnontai me: %token <union_type> termatiko_sumvolo */
/* Mi termatika sumvola dilwnontai me: %type */
%token<string>  ASSIGNMENT ADDITION SUBTRACTION MULTIPLICATION DIVISION MODULO
                EQUALITY INEQUALITY INCREMENT DECREMENT GREATER_THAN LESS_THAN
                GREATER_OR_EQUAL LESS_OR_EQUAL UNARY_MINUS
%token<string>  IF ELSE WHILE FOR FUNCTION RETURN BREAK CONTINUE AND NOT OR
                LOCAL TRUE FALSE NIL
%token<string>  STRING ID ERROR
%token<string>  LEFT_CURLY_BRACKET RIGHT_CURLY_BRACKET LEFT_SQUARE_BRACKET
                RIGHT_SQUARE_BRACKET LEFT_PARENTHESES RIGHT_PARENTHESES
                SEMICOLON COMMA COLON DOUBLE_COLON DOT DOUBLE_DOT
%token<integer> INTEGER
%token<real>    REAL

/* %type <sth> lvalue, expression?, terminal? */
%type<symbol_table_entry> lvalue
%type<expression_type> expression
%type<expression_type> assignment_expression
%type<expression_type> terminal
%type<expression_type> primary


/* priority grows downwards */
/* left: kanoume apotimisi apo ta aristera pros ta deksia */
/* right: kanoume apotimisi apo deksia pros ta aristera */
%right ASSIGNMENT
%left OR
%left AND
%nonassoc EQUALITY INEQUALITY
%nonassoc GREATER_THAN LESS_THAN GREATER_OR_EQUAL LESS_OR_EQUAL
%left ADDITION SUBTRACTION
%left MULTIPLICATION DIVISION MODULO
%right NOT INCREMENT DECREMENT UNARY_MINUS
%left DOT DOUBLE_DOT
%left LEFT_SQUARE_BRACKET RIGHT_SQUARE_BRACKET
%left LEFT_PARENTHESES RIGHT_PARENTHESES
%nonassoc PUREIF
%nonassoc ELSE


%%
/* Grammar */
program
    : all_statements {
        stack_node_t *popped = stack_pop(comment_stack);
        if (popped) {
            fprintf(stderr, "Comment opened in line %d, but was never "
                    "closed.\n", popped->line_number);
            free(popped);
            YYABORT;
        }
        if (ERROR_FLAG) YYABORT;
    }
    ;

all_statements
    : all_statements statement
    | /* empty */
    ;

statement
    : expression SEMICOLON
    | if_statement
    | while_statement
    | for_statement
    | return_statement {
        if (stack_lookup(block_stack, FUNC_BLOCK) == 0) {
            yyerror("Cannot return outside of a function");
        }
    }
    | BREAK SEMICOLON {
        if (!block_stack->top || block_stack->top->type != LOOP) {
            yyerror("Cannot BREAK outside of a loop");
        }
    }
    | CONTINUE SEMICOLON {
        if (!block_stack->top || block_stack->top->type != LOOP) {
            yyerror("Cannot CONTINUE outside of a loop");
        }
    }
    | block
    | function_definition
    | SEMICOLON
    ;

expression
    : assignment_expression { $$ = $1; }
    | expression ADDITION expression {
        if ($1 == FUNC_TYPE || $3 == FUNC_TYPE) {
            yyerror("Cannot use ADDITION operator in a function");
        }
        $$ = VAR_TYPE;
    }
    | expression SUBTRACTION expression {
        if ($1 == FUNC_TYPE || $3 == FUNC_TYPE) {
            yyerror("Cannot use SUBTRACTION operator in a function");
        }
        $$ = VAR_TYPE;
    }
    | expression MULTIPLICATION expression {
        if ($1 == FUNC_TYPE || $3 == FUNC_TYPE) {
            yyerror("Cannot use MULTIPLICATION operator in a function");
        }
        $$ = VAR_TYPE;
    }
    | expression DIVISION expression {
        if ($1 == FUNC_TYPE || $3 == FUNC_TYPE) {
            yyerror("Cannot use DIVISION operator in a function");
        }
        $$ = VAR_TYPE;
    }
    | expression MODULO expression {
        if ($1 == FUNC_TYPE || $3 == FUNC_TYPE) {
            yyerror("Cannot use MODULO operator in a function");
        }
        $$ = VAR_TYPE;
    }
    | expression GREATER_THAN expression {
        if ($1 == FUNC_TYPE || $3 == FUNC_TYPE) {
            yyerror("Cannot use GREATER_THAN operator in a function");
        }
        $$ = VAR_TYPE;
    }
    | expression LESS_THAN expression {
        if ($1 == FUNC_TYPE || $3 == FUNC_TYPE) {
            yyerror("Cannot use LESS_THAN operator in a function");
        }
        $$ = VAR_TYPE;
    }
    | expression GREATER_OR_EQUAL expression {
        if ($1 == FUNC_TYPE || $3 == FUNC_TYPE) {
            yyerror("Cannot use GREATER_OR_EQUAL operator in a function");
        }
        $$ = VAR_TYPE;
    }
    | expression LESS_OR_EQUAL expression {
        if ($1 == FUNC_TYPE || $3 == FUNC_TYPE) {
            yyerror("Cannot use LESS_OR_EQUAL operator in a function");
        }
        $$ = VAR_TYPE;
    }
    | expression EQUALITY expression {
        if ($1 == FUNC_TYPE || $3 == FUNC_TYPE) {
            yyerror("Cannot use EQUALITY operator in a function");
        }
        $$ = VAR_TYPE;
    }
    | expression INEQUALITY expression {
        if ($1 == FUNC_TYPE || $3 == FUNC_TYPE) {
            yyerror("Cannot use INEQUALITY operator in a function");
        }
        $$ = VAR_TYPE;
    }
    | expression AND expression {
        if ($1 == FUNC_TYPE || $3 == FUNC_TYPE) {
            yyerror("Cannot use AND operator in a function");
        }
        $$ = VAR_TYPE;
    }
    | expression OR expression {
        if ($1 == FUNC_TYPE || $3 == FUNC_TYPE) {
            yyerror("Cannot use OR operator in a function");
        }
        $$ = VAR_TYPE;
    }
    | terminal { $$ = $1; }
    ;

terminal
    : LEFT_PARENTHESES expression RIGHT_PARENTHESES { $$ = $2; }
    | SUBTRACTION expression %prec UNARY_MINUS {
        if ($2 == FUNC_TYPE) {
            yyerror("Cannot use UNARY_MINUS operator in a function");
        }
        $$ = VAR_TYPE;
    }
    | NOT expression {
        if ($2 == FUNC_TYPE) {
            yyerror("Cannot use NOT operator in a function");
        }
        $$ = VAR_TYPE;
    }
    | INCREMENT lvalue {
        insert_variable($2);

        $$ = VAR_TYPE;
    }
    | lvalue INCREMENT {
        insert_variable($1);
        $$ = VAR_TYPE;
    }
    | DECREMENT lvalue {
        insert_variable($2);
        $$ = VAR_TYPE;
    }
    | lvalue DECREMENT {
        insert_variable($1);
        $$ = VAR_TYPE;
    }
    | primary { $$ = $1; }
    | ERROR { YYABORT; }
    ;

assignment_expression
    : lvalue ASSIGNMENT expression {
        insert_variable($1);
        $$ = $3;
    }
    ;

lvalue
    : ID {
        SymbolType symbol_type =
            (current_scope == 0 ? GLOBAL_SYMBOL : LOCAL_SYMBOL);
        global_only = 0;
        $$ = create_SymbolTableEntry($1, NULL, current_scope, yylineno,
                                     symbol_type);
    }
    | LOCAL ID {
        SymbolType symbol_type =
            (current_scope == 0 ? GLOBAL_SYMBOL : LOCAL_SYMBOL);
        SymbolTableEntry *new_entry =
            create_SymbolTableEntry($2, NULL, current_scope, yylineno,
                                    symbol_type);
        if (SymTable_contains(symbol_table, $2, current_scope) == 0) {
            if (SymTable_contains(symbol_table, $2, 0) == 1)
                if (SymTable_get(symbol_table, $2)->type == LIBFUNC_SYMBOL) {
                    yyerror("Local var name overshadows library function");
                } else
                    SymTable_put(symbol_table, new_entry->name, new_entry);
            else
            	SymTable_put(symbol_table, new_entry->name, new_entry);
            $$ = NULL;
        } else {
            free(new_entry);
            $$ = SymTable_get(symbol_table, $2);
        }
        global_only = 0;
    }
    | DOUBLE_COLON ID {
        if (SymTable_contains(symbol_table, $2, 0) == 0) {
            yyerror("No matching global variable has been defined");
            $$ = NULL;
        } else {
            global_only = 1;
            $$ = SymTable_get_entry_at_scope(symbol_table, $2, 0);
        }
    }
    | member { $$ = NULL; }
    ;

primary
    : lvalue {
        if ($1 != NULL) {
            /* Checking CURRENT scope */
            if (SymTable_contains(symbol_table, $1->name, current_scope) == 0) {
                if (current_scope != 0) {
                    SymbolTableEntry *curr = SymTable_get_closest_scope(
                        symbol_table, $1->name, current_scope - 1);
                    /* Checking if not found at any scope before globals */
                    if (!curr && global_only == 0) {
                        if (SymTable_contains(symbol_table, $1->name, 0) == 0)
                            SymTable_put(symbol_table, $1->name, $1);
                    } else if (global_only == 0 &&
                               SymTable_open_func(symbol_table, curr,
                                                  current_scope) == 1) {
                        /* If found and there's an open function between the
                         * found one and our current one. */
                        yyerror("Cannot access this variable");
                        $$ = VAR_TYPE;
                    } else
                        $$ = SymTable_get(symbol_table, $1->name)->type >
                                          FORMAL_SYMBOL ? FUNC_TYPE : VAR_TYPE;
                } else {
                    SymTable_put(symbol_table, $1->name, $1);
                    $$ = SymTable_get(symbol_table, $1->name)->type >
                                      FORMAL_SYMBOL ? FUNC_TYPE : VAR_TYPE;
                }
            } else {
                $$ = SymTable_get(symbol_table, $1->name)->type > FORMAL_SYMBOL
                                  ? FUNC_TYPE : VAR_TYPE;
            }
        } else {
            $$ = VAR_TYPE;
        }
    }
    | call { $$ = VAR_TYPE; }
    | object_definition { $$ = VAR_TYPE; }
    | LEFT_PARENTHESES function_definition RIGHT_PARENTHESES { $$ = FUNC_TYPE; }
    | constant { $$ = VAR_TYPE; }
    ;

member
    : lvalue DOT ID
    | lvalue LEFT_SQUARE_BRACKET expression RIGHT_SQUARE_BRACKET
    | call DOT ID
    | call LEFT_SQUARE_BRACKET expression RIGHT_SQUARE_BRACKET
    ;

call
    : call LEFT_PARENTHESES elist RIGHT_PARENTHESES
    | lvalue callsuffix {
        if ($1 != NULL) {
            SymbolTableEntry *curr =
                SymTable_get_closest_scope(symbol_table, $1->name,
                                           current_scope);
            if (!curr) {
                if (!SymTable_get_entry_at_scope(symbol_table, $1->name, 0))
                    SymTable_put(symbol_table, $1->name, $1);
            } else if (curr->type <= FORMAL_SYMBOL &&
                       SymTable_open_func(symbol_table, curr,
                                          current_scope) == 1) {
                yyerror("Cannot access this variable");
            }
        }
    }
    | LEFT_PARENTHESES function_definition RIGHT_PARENTHESES LEFT_PARENTHESES
      elist RIGHT_PARENTHESES
    ;

callsuffix
    : normal_call
    | method_call
    ;

normal_call
    : LEFT_PARENTHESES elist RIGHT_PARENTHESES
    ;

method_call
    : DOUBLE_DOT ID LEFT_PARENTHESES elist RIGHT_PARENTHESES
    ;

elist
    : expression param
    | /* empty */
    ;

param
    : COMMA expression param
    | /* empty */
    ;

object_definition
    : LEFT_SQUARE_BRACKET elist RIGHT_SQUARE_BRACKET
    | LEFT_SQUARE_BRACKET indexed RIGHT_SQUARE_BRACKET
    ;

indexed
    : indexedelem other_indexed
    ;

other_indexed
    : COMMA indexedelem other_indexed
    | /* empty */
    ;

indexedelem
    : LEFT_CURLY_BRACKET expression COLON expression RIGHT_CURLY_BRACKET
    ;

block
    : LEFT_CURLY_BRACKET { current_scope++; }
    all_statements RIGHT_CURLY_BRACKET {
        SymList_deactivate(symbol_table->scopes[current_scope]);
        current_scope--;
    }
    ;

function_definition
    : FUNCTION ID LEFT_PARENTHESES idlist RIGHT_PARENTHESES {
        insert_stack(block_stack, yylineno, FUNC_BLOCK);
        SymbolTableEntry *new_entry = create_SymbolTableEntry(
            $2, arguments_list, current_scope, yylineno, USERFUNC_SYMBOL);
        if (SymTable_contains(symbol_table, new_entry->name,
                              current_scope) == 0) {
            SymbolTableEntry *tmp = SymTable_get(symbol_table, new_entry->name);
            if (tmp && tmp->type == LIBFUNC_SYMBOL) {
                yyerror("Cannot redefine library function");
            } else {
                SymTable_put(symbol_table, new_entry->name, new_entry);
                ArgList_to_SymTable(symbol_table, arguments_list);
            }
        } else {
            yyerror("Function name already used");
            if (arguments_list) {
                SymList_apply_all(arguments_list, free);
                SymList_destruct(arguments_list);
            }
        }
        arguments_list = NULL;
    }
    block {
        stack_pop(block_stack);
        SymTable_get(symbol_table, $2)->func_is_open = 0;
    }
    | FUNCTION LEFT_PARENTHESES idlist RIGHT_PARENTHESES {
        insert_stack(block_stack, yylineno, FUNC_BLOCK);
        SymbolTableEntry *new_entry = create_SymbolTableEntry(
            NULL, arguments_list, current_scope, yylineno, USERFUNC_SYMBOL);
        SymTable_put(symbol_table, new_entry->name, new_entry);
        ArgList_to_SymTable(symbol_table, arguments_list);
        arguments_list = NULL;
    }
    block {
        stack_pop(block_stack);
        char *name = malloc(10 * sizeof(char));
        sprintf(name, "$%d", --nameless_func_count);
        SymTable_get(symbol_table, name)->func_is_open = 0;
        free(name);
    }
    ;

constant
    : INTEGER
    | REAL
    | STRING
    | NIL
    | TRUE
    | FALSE
    ;

idlist
    : ID {
        if (SymTable_contains(symbol_table, $1, 0) == 1)
            if (SymTable_get(symbol_table, $1)->type == LIBFUNC_SYMBOL) {
                yyerror("Argument name overshadows library function");
            }
    }
    other_id {
        if (!arguments_list) arguments_list = SymList_new();
        SymbolTableEntry *new_entry = create_SymbolTableEntry(
            $1, NULL, current_scope + 1, yylineno, FORMAL_SYMBOL);
        if (SymList_get(arguments_list, new_entry->name)) {
            yyerror("Argument name already used");
        } else
            SymList_insert(arguments_list, new_entry);
    }
    | { arguments_list = NULL; }
    ;

other_id
    : COMMA ID {
        if (SymTable_contains(symbol_table, $2, 0) == 1)
            if (SymTable_get(symbol_table, $2)->type > FORMAL_SYMBOL) {
                yyerror("Argument name overshadows library function");
            }
    }
    other_id {
        if (!arguments_list) arguments_list = SymList_new();
        SymbolTableEntry *new_entry = create_SymbolTableEntry(
            $2, NULL, current_scope + 1, yylineno, FORMAL_SYMBOL);
        if (SymList_get(arguments_list, new_entry->name)) {
            yyerror("Argument name already used");
        } else
            SymList_insert(arguments_list, new_entry);
    }
    | /* empty */
    ;

if_statement
    : IF LEFT_PARENTHESES expression RIGHT_PARENTHESES statement %prec PUREIF
    | IF LEFT_PARENTHESES expression RIGHT_PARENTHESES statement ELSE statement
    ;

while_statement
    : WHILE { insert_stack(block_stack, yylineno, LOOP); }
    LEFT_PARENTHESES expression RIGHT_PARENTHESES statement {
        free(stack_pop(block_stack));
    };

    for_statement : FOR { insert_stack(block_stack, yylineno, LOOP); }
    LEFT_PARENTHESES elist SEMICOLON expression SEMICOLON elist
    RIGHT_PARENTHESES statement {
        free(stack_pop(block_stack));
    }
    ;

return_statement
    : RETURN SEMICOLON
    | RETURN expression SEMICOLON
    ;
%%


/* Epilogos - aftousios kwdikas pou parexoume ston yacc */

static void ArgList_to_SymTable(SymTable_T sym_table, SymList_T arg_list) {
    list_entry_T *iter = NULL;
    if (arg_list) iter = arg_list->head;

    while (iter) {
        SymTable_put(sym_table, iter->entry->name, iter->entry);
        iter = iter->next;
    }

    return;
}


void yyerror(char *yacc_error_message) {
    fprintf(stderr, "%s: at line %d\n", yacc_error_message, yylineno);
    ERROR_FLAG = 1;
}


static int insert_variable(SymbolTableEntry *entry) {
    if (entry != NULL) {
        /* Checking CURRENT scope */
        if (SymTable_contains(symbol_table, entry->name, current_scope) == 0) {
            if (current_scope != 0) {
                SymbolTableEntry *curr = SymTable_get_closest_scope(
                    symbol_table, entry->name, current_scope - 1);
                /* Checking if not found at any scope before globals */
                if (!curr && global_only == 0) {
                    if (SymTable_contains(symbol_table, entry->name, 0) == 0)
                        SymTable_put(symbol_table, entry->name, entry);
                    else if (SymTable_get_entry_at_scope(symbol_table,
                                                         entry->name, 0)
                                 ->type > FORMAL_SYMBOL) {
                        yyerror("Cannot change the value of a function");
                        return 0;
                    }
                } else if (global_only == 0 &&
                           SymTable_open_func(symbol_table, curr,
                                              current_scope) == 1) {
                    /* If found and there's an open function between the found
                     * one and our current one. */
                    yyerror("Cannot access this variable");
                    free(entry);
                    return 0;

                } else {
                    SymbolTableEntry *tmp =
                        SymTable_get(symbol_table, entry->name);
                    if (tmp && tmp->type > FORMAL_SYMBOL) {  // fixme
                        yyerror("Cannot change the value of a function");
                        return 0;
                    }
                }
            } else { /* Case we are checking the global scope  */
                SymTable_put(symbol_table, entry->name, entry);
            }
        } else if (SymTable_get_entry_at_scope(symbol_table, entry->name,
                                               current_scope)
                       ->type > FORMAL_SYMBOL) {
            yyerror("Cannot change the value of a function");
            return 0;
        }
    }
    return 1;
}


/***************** Debugging functions ***********************/

static void print_node_if_active(void *node) {
    char *type;

    SymbolTableEntry *entry = (SymbolTableEntry *)node;
    switch (entry->type) {
        case GLOBAL_SYMBOL:
            type = "[global variable]";
            break;
        case LOCAL_SYMBOL:
            type = "[local variable]";
            break;
        case FORMAL_SYMBOL:
            type = "[formal argument]";
            break;
        case USERFUNC_SYMBOL:
            type = "[user function]";
            break;
        case LIBFUNC_SYMBOL:
            type = "[library function]";
            break;
        default:
            break;
    }
    fprintf(stdout, "%s \"%s\" (line %d) (scope %d) (%d)\n", type, entry->name,
            entry->line, entry->scope, entry->isActive);
}


void print_active_scopes(SymTable_T oSymTable) {
    int i;
    printf("\n\n--------------------------------\n");
    for (i = 0; i < oSymTable->scopes_size; i++) {
        if (oSymTable->scopes[i]->length != 0) {
            SymList_apply_all(oSymTable->scopes[i], print_node_if_active);
        }
    }
    printf("********************************\n\n");
}

