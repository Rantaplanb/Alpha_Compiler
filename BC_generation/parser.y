%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <assert.h>

    #include "libs/globals.h"
    #include "libs/stack.h"
    #include "libs/symtable.h"
    #include "libs/symlist.h"
    #include "libs/intermediate_code.h"
    #include "libs/abc_writer.h"
    #include "libs/tcode_generation.h"

    static void ArgList_to_SymTable(SymTable_T sym_table, SymList_T arg_list);
    static void print_elem(const char *pcKey, SymbolTableEntry *pvValue,
                    void *pvExtra);
    static void print_scopes(SymTable_T oSymTable);
    static void print_node(void *node);

    static SymList_T arguments_list;
    int IR_GEN_FAILED = 0;
    static FILE *air_file;
    static FILE *abc_file;
    static FILE *out_file;

    #define IS_DUMMY(x) x->type == dummy_e
    static Expr *expr_dummy;

    /* For printing lines of syntax errors */
    #define YYERROR_VERBOSE 1
%}



%start program

/* Contains all the types that symbols (terminals or nonterminals) can get */
%union {
    char *string;
    int integer;
    double real;
    struct SymbolTableEntry *symbol_table_entry;
    struct Expr *expression;
    struct Call *call;
    struct ForPrefix *for_prefix;
}

/* Terminal symbol declaration: %token<union_type> terminal_symbol */
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

/* Non-terminal symbol declaration: %type<union_type> non_terminal_symbol */
%type<expression> expression lvalue member primary call assignment_expression
%type<expression> logical_expression statement all_statements block terminal
%type<expression> object_definition constant rev_elist param indexedelem
%type<expression> indexed other_indexed argument if_statement
%type<symbol_table_entry> function_definition function_name function_prefix
%type<call> callsuffix normal_call method_call
%type<integer> function_body if_prefix else_prefix while_start N M
%type<integer> while_condition
%type<for_prefix> for_prefix

/* Priority grows downwards */
/* left: we evaluate from left to right */
/* right: we evaluate from right to left */
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
        if (IR_GEN_FAILED) YYABORT;
    }
    ;

all_statements
    : all_statements statement {
        reset_temp_var_counter();
        $$ = create_expr(dummy_e);
        if ($1 && $2) {
            $$->t_list = merge_lists($1->t_list, $2->t_list);
            $$->f_list = merge_lists($1->f_list, $2->f_list);
            $$->b_list = merge_lists($1->b_list, $2->b_list);
            $$->c_list = merge_lists($1->c_list, $2->c_list);
        } else if ($1) {
            $$->t_list = $1->t_list;
            $$->f_list = $1->f_list;
            $$->b_list = $1->b_list;
            $$->c_list = $1->c_list;
        } else if ($2) {
            $$->t_list = $2->t_list;
            $$->f_list = $2->f_list;
            $$->b_list = $2->b_list;
            $$->c_list = $2->c_list;
        }
    }
    | { $$ = NULL; } /* empty */
    ;

statement
    : expression SEMICOLON {
        $$ = backpatch_tf_lists($1);
    }
    | if_statement
    | while_statement { $$ = NULL; }
    | for_statement { $$ = NULL; }
    | return_statement {
        if (stack_lookup(block_stack, FUNC_BLOCK) == 0) {
            yyerror("cannot return outside of a function");
        } else {
            // Get last open function and insert next_quad number on its list
            LabelList *new_node = create_list(next_quad());
            new_node->next = SymTable_get_open_func(symbol_table, current_scope)
                                       ->address_list->next;
            SymTable_get_open_func(symbol_table, current_scope)
                ->address_list->next = new_node;
            emit(jump_opc, NULL, NULL, NULL, yylineno, 0);
        }

        $$ = NULL;
    }
    | BREAK SEMICOLON {
        $$ = create_expr(dummy_e);
        $$->b_list = NULL;
        $$->c_list = NULL;

        if (!block_stack->top || block_stack->top->type != LOOP) {
            yyerror("cannot BREAK outside of a loop");
        } else {
            $$->b_list = create_list(next_quad());
            emit(jump_opc, NULL, NULL, NULL, yylineno, 0);
        }
    }
    | CONTINUE SEMICOLON {
        $$ = create_expr(dummy_e);
        $$->b_list = NULL;
        $$->c_list = NULL;
        if (!block_stack->top || block_stack->top->type != LOOP)
            yyerror("cannot CONTINUE outside of a loop");
        else {
            $$->c_list = create_list(next_quad());
            emit(jump_opc, NULL, NULL, NULL, yylineno, 0);
        }
    }
    | block
    | function_definition { $$ = NULL; }
    | SEMICOLON { $$ = NULL; }
    ;

// TODO: check for const evaluations
expression
    : assignment_expression
    | expression ADDITION expression {
        if (IS_DUMMY($1) || IS_DUMMY($3))
            $$ = create_expr(dummy_e);
        else {
            check_arith_expr($1, "ADDITION");
            check_arith_expr($3, "ADDITION");
            $$ = handle_arith_expr_op_expr($1, add_opc, $3);
        }
    }
    | expression SUBTRACTION expression {
        if (IS_DUMMY($1) || IS_DUMMY($3))
            $$ = create_expr(dummy_e);
        else {
            check_arith_expr($1, "SUBTRACTION");
            check_arith_expr($3, "SUBTRACTION");
            $$ = handle_arith_expr_op_expr($1, sub_opc, $3);
        }
    }
    | expression MULTIPLICATION expression {
        if (IS_DUMMY($1) || IS_DUMMY($3))
            $$ = create_expr(dummy_e);
        else {
            check_arith_expr($1, "MULTIPLICATION");
            check_arith_expr($3, "MULTIPLICATION");
            $$ = handle_arith_expr_op_expr($1, mul_opc, $3);
        }
    }
    | expression DIVISION expression {
        if (IS_DUMMY($1) || IS_DUMMY($3))
            $$ = create_expr(dummy_e);
        else {
            check_arith_expr($1, "DIVISION");
            check_arith_expr($3, "DIVISION");
            $$ = handle_arith_expr_op_expr($1, div_opc, $3);
        }
    }
    | expression MODULO expression {
        if (IS_DUMMY($1) || IS_DUMMY($3))
            $$ = create_expr(dummy_e);
        else {
            check_arith_expr($1, "MODULO");
            check_arith_expr($3, "MODULO");
            $$ = handle_arith_expr_op_expr($1, mod_opc, $3);
        }
    }
    | logical_expression
    | terminal
    ;

// TODO: check for const evaluations
logical_expression
    : expression GREATER_THAN expression {
        if (IS_DUMMY($1) || IS_DUMMY($3))
            $$ = create_expr(dummy_e);
        else {
            check_arith_expr($1, "GREATER_THAN");
            check_arith_expr($3, "GREATER_THAN");
            $1 = backpatch_tf_lists($1);
            $3 = backpatch_tf_lists($3);
            $$ = create_expr(bool_expr_e);
            $$->sym_value = NULL;
            $$->t_list = create_list(next_quad());
            $$->f_list = create_list(next_quad() + 1);
            emit(if_greater_opc, $1, $3, NULL, yylineno, 0);
            emit(jump_opc, NULL, NULL, NULL, yylineno, 0);
        }
    }
    | expression LESS_THAN expression {
        if (IS_DUMMY($1) || IS_DUMMY($3))
            $$ = create_expr(dummy_e);
        else {
            check_arith_expr($1, "LESS_THAN");
            check_arith_expr($3, "LESS_THAN");
            $1 = backpatch_tf_lists($1);
            $3 = backpatch_tf_lists($3);
            $$ = create_expr(bool_expr_e);
            $$->sym_value = NULL;
            $$->t_list = create_list(next_quad());
            $$->f_list = create_list(next_quad() + 1);
            emit(if_less_opc, $1, $3, NULL, yylineno, 0);
            emit(jump_opc, NULL, NULL, NULL, yylineno, 0);
        }
    }
    | expression GREATER_OR_EQUAL expression {
        if (IS_DUMMY($1) || IS_DUMMY($3))
            $$ = create_expr(dummy_e);
        else {
            check_arith_expr($1, "GREATER_OR_EQUAL");
            check_arith_expr($3, "GREATER_OR_EQUAL");
            $1 = backpatch_tf_lists($1);
            $3 = backpatch_tf_lists($3);
            $$ = create_expr(bool_expr_e);
            $$->sym_value = NULL;
            $$->t_list = create_list(next_quad());
            $$->f_list = create_list(next_quad() + 1);
            emit(if_greatereq_opc, $1, $3, NULL, yylineno, 0);
            emit(jump_opc, NULL, NULL, NULL, yylineno, 0);
        }
    }
    | expression LESS_OR_EQUAL expression {
        if (IS_DUMMY($1) || IS_DUMMY($3))
            $$ = create_expr(dummy_e);
        else {
            check_arith_expr($1, "LESS_OR_EQUAL");
            check_arith_expr($3, "LESS_OR_EQUAL");
            $1 = backpatch_tf_lists($1);
            $3 = backpatch_tf_lists($3);
            $$ = create_expr(bool_expr_e);
            $$->sym_value = NULL;
            $$->t_list = create_list(next_quad());
            $$->f_list = create_list(next_quad() + 1);
            emit(if_lesseq_opc, $1, $3, NULL, yylineno, 0);
            emit(jump_opc, NULL, NULL, NULL, yylineno, 0);
        }
    }
    | expression EQUALITY expression {
        if (IS_DUMMY($1) || IS_DUMMY($3))
            $$ = create_expr(dummy_e);
        else {
            check_matching_expr($1, $3, $2);
            $1 = backpatch_tf_lists($1);
            $3 = backpatch_tf_lists($3);
            $$ = create_expr(bool_expr_e);
            $$->sym_value = NULL;
            $$->t_list = create_list(next_quad());
            $$->f_list = create_list(next_quad() + 1);
            emit(if_eq_opc, $1, $3, NULL, yylineno, 0);
            emit(jump_opc, NULL, NULL, NULL, yylineno, 0);
        }
    }
    | expression INEQUALITY expression {
        if (IS_DUMMY($1) || IS_DUMMY($3))
            $$ = create_expr(dummy_e);
        else {
            check_matching_expr($1, $3, $2);
            $1 = backpatch_tf_lists($1);
            $3 = backpatch_tf_lists($3);
            $$ = create_expr(bool_expr_e);
            $$->sym_value = NULL;
            $$->t_list = create_list(next_quad());
            $$->f_list = create_list(next_quad() + 1);
            emit(if_noteq_opc, $1, $3, NULL, yylineno, 0);
            emit(jump_opc, NULL, NULL, NULL, yylineno, 0);
        }
    }
    | expression AND M {
        patch_list($1->t_list, $3);
        if (!$1->f_list) {
            $1->f_list = create_list(next_quad() + 1);
            emit(if_eq_opc, $1, create_boolean_expr(true),  NULL, yylineno,
                 next_quad() + 2);
            emit(jump_opc, NULL, NULL, NULL, yylineno, 0);
        }
    } expression {
        if (IS_DUMMY($1) || IS_DUMMY($5))
            $$ = create_expr(dummy_e);
        else {
            if (!$5->f_list) {
                $5->t_list = create_list(next_quad());
                $5->f_list = create_list(next_quad() + 1);
                emit(if_eq_opc, $5, create_boolean_expr(true), NULL,
                     yylineno, 0);
                emit(jump_opc, NULL, NULL, NULL, yylineno, 0);
            }
            $$->f_list = merge_lists($1->f_list, $5->f_list);
            $$->t_list = $5->t_list;
        }
    }
    | expression OR M {
        patch_list($1->f_list, $3);
        if (!$1->t_list) {
            $1->t_list = create_list(next_quad());
            emit(if_eq_opc, $1, create_boolean_expr(true), NULL,
                 yylineno, 0);
            emit(jump_opc, NULL, NULL, NULL, yylineno, next_quad() + 1);
        }
    } expression {
        if (IS_DUMMY($1) || IS_DUMMY($5))
            $$ = create_expr(dummy_e);
        else {
            if (!$5->f_list) {
                $5->t_list = create_list(next_quad());
                $5->f_list = create_list(next_quad() + 1);
                emit(if_eq_opc, $5, create_boolean_expr(true), NULL,
                     yylineno, 0);
                emit(jump_opc, NULL, NULL, NULL, yylineno, 0);
            }
            $$->t_list = merge_lists($1->t_list, $5->t_list);
            $$->f_list = $5->f_list;
        }
    }
    ;

// TODO: check if const evaluation applies in UNARY_MINUS
terminal
    : LEFT_PARENTHESES expression RIGHT_PARENTHESES { $$ = $2; }
    | SUBTRACTION expression %prec UNARY_MINUS {
        if (IS_DUMMY($2))
            $$ = create_expr(dummy_e);
        else {
            check_arith_expr($2, "UNARY_MINUS");
            $$ = create_expr(arith_expr_e);
            $$->sym_value = create_temp_symbol();
            emit(uminus_opc, $2, NULL, $$, yylineno, 0);
        }
    }
    | NOT expression {
        if (IS_DUMMY($2))
            $$ = create_expr(dummy_e);
        else {
            if (!$2->f_list) {
                $2->t_list = create_list(next_quad());
                $2->f_list = create_list(next_quad() + 1);
                emit(if_eq_opc, $2, create_boolean_expr(true), NULL,
                     yylineno, 0);
                emit(jump_opc, NULL, NULL, NULL, yylineno, 0);
            }
            if (expr_transformable_to_bool($2)) // checks for const evaluation
                $$ = create_boolean_expr(!expr_to_bool($2));
            else {
                $$ = create_expr(bool_expr_e);
                $$->sym_value = NULL;
            }
            $$->f_list = $2->t_list;
            $$->t_list = $2->f_list;
        }
    }
    | INCREMENT lvalue {
        if (IS_DUMMY($2))
            $$ = create_expr(dummy_e);
        else {
            check_arith_expr($2, "INCREMENT VARIABLE");
            SymbolTableEntry *entry =
                SymTable_get_closest_scope(symbol_table,
                                           $2->sym_value->name, current_scope);

            if (entry->scope != 0 && entry->type < USERFUNC_SYMBOL &&
                SymTable_open_func(symbol_table, entry, current_scope)) {
                yyerror("cannot access this variable");
                $$ = create_expr(dummy_e);
            } else if ($2->type == table_item_e) {
                $$ = emit_if_table_item($2);
                emit(add_opc, $$, create_numeric_expr(1), $$, yylineno, 0);
                emit(tablesetelem_opc, $2, $2->index, $$, yylineno, 0);
            } else {
                emit(add_opc, $2, create_numeric_expr(1), $2, yylineno, 0);
                $$ = create_expr(arith_expr_e);
                $$->sym_value = create_temp_symbol();
                emit(assign_opc, $2, NULL, $$, yylineno, 0);
            }
        }
    }
    | lvalue INCREMENT {
        if (IS_DUMMY($1))
            $$ = create_expr(dummy_e);
        else {
            check_arith_expr($1, "VARIABLE INCREMENT");
            SymbolTableEntry *entry =
                SymTable_get_closest_scope(symbol_table,
                                           $1->sym_value->name, current_scope);

            $$ = create_expr(var_e);
            $$->sym_value = create_temp_symbol();
            if (entry->scope != 0 && entry->type < USERFUNC_SYMBOL &&
                SymTable_open_func(symbol_table, entry, current_scope)) {
                yyerror("cannot access this variable");
                $$ = create_expr(dummy_e);
            } else if ($1->type == table_item_e) {
                Expr *val = emit_if_table_item($1);
                emit(assign_opc, val, NULL, $$, yylineno, 0);
                emit(add_opc, val, create_numeric_expr(1), val, yylineno, 0);
                emit(tablesetelem_opc, $1, $1->index, val, yylineno, 0);
            } else {
                emit(assign_opc, $1, NULL, $$, yylineno, 0);
                emit(add_opc, $1, create_numeric_expr(1), $1, yylineno, 0);
            }
        }
    }
    | DECREMENT lvalue {
        if (IS_DUMMY($2))
            $$ = create_expr(dummy_e);
        else {
            check_arith_expr($2, "DECREMENT VARIABLE");
            SymbolTableEntry *entry =
                SymTable_get_closest_scope(symbol_table,
                                           $2->sym_value->name, current_scope);

            if (entry->scope != 0 && entry->type < USERFUNC_SYMBOL &&
                SymTable_open_func(symbol_table, entry, current_scope)) {
                yyerror("cannot access this variable");
                $$ = create_expr(dummy_e);
            } else if ($2->type == table_item_e) {
                $$ = emit_if_table_item($2);
                emit(sub_opc, $$, create_numeric_expr(1), $$, yylineno, 0);
                emit(tablesetelem_opc, $2, $2->index, $$, yylineno, 0);
            } else {
                emit(sub_opc, $2, create_numeric_expr(1), $2, yylineno, 0);
                $$ = create_expr(arith_expr_e);
                $$->sym_value = create_temp_symbol();
                emit(assign_opc, $2, NULL, $$, yylineno, 0);
            }
        }
    }
    | lvalue DECREMENT {
        if (IS_DUMMY($1))
            $$ = create_expr(dummy_e);
        else {
            check_arith_expr($1, "VARIABLE DECREMENT");
            SymbolTableEntry *entry =
                SymTable_get_closest_scope(symbol_table,
                                           $1->sym_value->name, current_scope);

            $$ = create_expr(var_e);
            $$->sym_value = create_temp_symbol();
            if (entry->scope != 0 && entry->type < USERFUNC_SYMBOL &&
                SymTable_open_func(symbol_table, entry, current_scope)) {
                yyerror("cannot access this variable");
                $$ = create_expr(dummy_e);
            } else if ($1->type == table_item_e) {
                Expr *val = emit_if_table_item($1);
                emit(assign_opc, val, NULL, $$, yylineno, 0);
                emit(sub_opc, val, create_numeric_expr(1), val, yylineno, 0);
                emit(tablesetelem_opc, $1, $1->index, val, yylineno, 0);
            } else {
                emit(assign_opc, $1, NULL, $$, yylineno, 0);
                emit(sub_opc, $1, create_numeric_expr(1), $1, yylineno, 0);
            }
        }
    }
    | primary
    | ERROR { YYABORT; }
    ;

assignment_expression
    : lvalue ASSIGNMENT expression {
        if (IS_DUMMY($1) || IS_DUMMY($3))
            $$ = create_expr(dummy_e);
        else {
            $3 = backpatch_tf_lists($3);
            SymbolTableEntry *entry =
                SymTable_get_closest_scope(symbol_table,
                                           $1->sym_value->name, current_scope);

            if (entry->type >= USERFUNC_SYMBOL)
                yyerror("cannot change the value of a function");
            else if (entry->scope != 0 &&
                     SymTable_open_func(symbol_table, entry, current_scope))
                yyerror("cannot access this variable");

            if ($1->type == table_item_e) {
                emit(tablesetelem_opc, $3, $1->index,  $1, yylineno, 0);
                $$ = emit_if_table_item($1);
                $$->type = assign_expr_e;
            } else {
                emit(assign_opc, $3, NULL, $1, yylineno, 0);
                $$ = create_expr(assign_expr_e);
                $$->sym_value = create_temp_symbol();
                emit(assign_opc, $1, NULL, $$, yylineno, 0);
            }
        }
    }
    ;

lvalue
    : ID {
        SymbolType symbol_type =
            (current_scope == 0 ? GLOBAL_SYMBOL : LOCAL_SYMBOL);
        SymbolTableEntry *sym;
        sym = SymTable_get_closest_scope(symbol_table, $1, current_scope);

        if (sym == NULL) {
            sym = create_SymbolTableEntry($1, NULL, current_scope, yylineno,
                                          symbol_type);
            SymTable_put(symbol_table, sym->name, sym);
            sym->space = get_curr_scope_space();
            sym->offset = get_curr_scope_offset();
            inc_curr_scope_offset();
        }

        $$ = lvalue_expr(sym);
    }
    | LOCAL ID {
        SymbolType symbol_type =
            (current_scope == 0 ? GLOBAL_SYMBOL : LOCAL_SYMBOL);
        SymbolTableEntry *sym;
        sym = SymTable_get_entry_at_scope(symbol_table, $2, current_scope);
    	if (SymTable_get_entry_at_scope(symbol_table, $2, 0) &&
    		SymTable_get_entry_at_scope(symbol_table, $2, 0)->type
            == LIBFUNC_SYMBOL) {
            yyerror("local var name overshadows library function");
            $$ = create_expr(dummy_e);
        } else if (sym == NULL) {
            sym = create_SymbolTableEntry($2, NULL, current_scope, yylineno,
                                          symbol_type);
            SymTable_put(symbol_table, sym->name, sym);
            sym->space = get_curr_scope_space();
            sym->offset = get_curr_scope_offset();
            inc_curr_scope_offset();
            $$ = lvalue_expr(sym);
        } else
            $$ = lvalue_expr(sym);
    }
    | DOUBLE_COLON ID {
        if (SymTable_contains(symbol_table, $2, 0) == 0) {
            yyerror("no matching global variable has been defined");
            $$ = create_expr(dummy_e);
        } else
            $$ = lvalue_expr(SymTable_get_entry_at_scope(symbol_table, $2, 0));
    }
    | member
    ;

primary
    : lvalue {
        if (IS_DUMMY($1))
            $$ = create_expr(dummy_e);
        else {
            SymbolTableEntry *entry =
                SymTable_get_closest_scope(symbol_table,
                                           $1->sym_value->name, current_scope);

            if (entry->scope != 0 && entry->type < USERFUNC_SYMBOL &&
                SymTable_open_func(symbol_table, entry, current_scope)) {
                yyerror("cannot access this variable");
                $$ = create_expr(dummy_e);
            } else
                $$ = emit_if_table_item($1);
        }
    }
    | call
    | object_definition
    | LEFT_PARENTHESES function_definition RIGHT_PARENTHESES {
        $$ = lvalue_expr($2);
    }
    | LEFT_PARENTHESES error RIGHT_PARENTHESES { $$ = create_expr(dummy_e); }
    | constant
    ;

member
    : lvalue DOT ID {
        if (IS_DUMMY($1))
            $$ = create_expr(dummy_e);
        else {
            SymbolTableEntry *entry =
                SymTable_get_closest_scope(symbol_table,
                                           $1->sym_value->name, current_scope);

            if (entry->type >= USERFUNC_SYMBOL) {
                yyerror("functions have no members");
                $$ = create_expr(dummy_e);
            } else if (entry->scope != 0 &&
                       SymTable_open_func(symbol_table, entry, current_scope)) {
                yyerror("cannot access this variable");
                $$ = create_expr(dummy_e);
            } else
                $$ = member_item($1, $3);
        }
    }
    | lvalue LEFT_SQUARE_BRACKET expression RIGHT_SQUARE_BRACKET {
        if (IS_DUMMY($1) || IS_DUMMY($3))
            $$ = create_expr(dummy_e);
        else {
            SymbolTableEntry *entry =
                SymTable_get_closest_scope(symbol_table,
                                           $1->sym_value->name, current_scope);
            $3 = backpatch_tf_lists($3);
            if (entry->type >= USERFUNC_SYMBOL) {
                yyerror("functions have no members");
                $$ = create_expr(dummy_e);
            } else if (entry->scope != 0 &&
                       SymTable_open_func(symbol_table, entry, current_scope)) {
                yyerror("cannot access this variable");
                $$ = create_expr(dummy_e);
            } else {
                $1 = emit_if_table_item($1);
                $$ = create_expr(table_item_e);
                $$->sym_value = $1->sym_value;
                $$->index = $3;
            }
        }
    }
    | lvalue LEFT_SQUARE_BRACKET error RIGHT_SQUARE_BRACKET {
        $$ = create_expr(dummy_e);
    }
    | call DOT ID {
        if (IS_DUMMY($1))
            $$ = create_expr(dummy_e);
        else
            $$ = member_item($1, $3);
    }
    | call LEFT_SQUARE_BRACKET expression RIGHT_SQUARE_BRACKET {
        if (IS_DUMMY($1) || IS_DUMMY($3))
            $$ = create_expr(dummy_e);
        else {
            $3 = backpatch_tf_lists($3);
            $1 = emit_if_table_item($1);
            $$ = create_expr(table_item_e);
            $$->sym_value = $1->sym_value;
            $$->index = $3;
        }
    }
    | call LEFT_SQUARE_BRACKET error RIGHT_SQUARE_BRACKET {
        $$ = create_expr(dummy_e);
    }
    ;

call
    : call LEFT_PARENTHESES rev_elist RIGHT_PARENTHESES {
        if (IS_DUMMY($1) || ($3 && IS_DUMMY($3)))
            $$ = create_expr(dummy_e);
        else
            $$ = make_call($1, $3, yylineno);
    }
    | call LEFT_PARENTHESES error RIGHT_PARENTHESES {
        $$ = create_expr(dummy_e);
    }
    | lvalue callsuffix {
        if (IS_DUMMY($1))
            $$ = create_expr(dummy_e);
        else {
            SymbolTableEntry *curr =
                SymTable_get_closest_scope(symbol_table, $1->sym_value->name,
                                           current_scope);

            if (curr->type <= FORMAL_SYMBOL && curr->scope != 0 &&
                SymTable_open_func(symbol_table, curr, current_scope) == 1) {
                yyerror("cannot access this variable");
                $$ = create_expr(dummy_e);
            } else {
                $1 = emit_if_table_item($1);
                if ($2->method) {
                    Expr *t = $1;
                    $1 = emit_if_table_item(member_item(t, $2->name));
                    if ($2->rev_elist)
                        $2->rev_elist->next = t;
                    else
                        $2->rev_elist = t;
                }
                $$ = make_call($1, $2->rev_elist, yylineno);
            }
        }
    }
    | LEFT_PARENTHESES function_definition RIGHT_PARENTHESES
      LEFT_PARENTHESES rev_elist RIGHT_PARENTHESES {
        if ($5 && IS_DUMMY($5))
            $$ = create_expr(dummy_e);
        else {
            Expr *func = create_expr(program_func_e);
            func->sym_value = $2;
            $$ = make_call(func, $5, yylineno);
        }
    }
    | LEFT_PARENTHESES error RIGHT_PARENTHESES
      LEFT_PARENTHESES rev_elist RIGHT_PARENTHESES {
        $$ = create_expr(dummy_e);
    }
    | LEFT_PARENTHESES function_definition RIGHT_PARENTHESES
      LEFT_PARENTHESES error RIGHT_PARENTHESES {
        $$ = create_expr(dummy_e);
    }
    | LEFT_PARENTHESES error RIGHT_PARENTHESES
      LEFT_PARENTHESES error RIGHT_PARENTHESES {
        $$ = create_expr(dummy_e);
    }
    ;

callsuffix
    : normal_call
    | method_call
    ;

normal_call
    : LEFT_PARENTHESES rev_elist RIGHT_PARENTHESES {
        $$ = create_call($2, 0, NULL);
    }
    ;

method_call
    : DOUBLE_DOT ID LEFT_PARENTHESES rev_elist RIGHT_PARENTHESES {
        $$ = create_call($4, 1, $2);
    }
    ;

rev_elist
    : argument param {
        if (IS_DUMMY($1) || ($2 && IS_DUMMY($2)))
            $$ = create_expr(dummy_e);
        else {
            $1->next = $2;

            /* Reverse the elist */
            Expr *head = reverse_elist($1);
            $$ = head;
        }
    }
    | { $$ = NULL; }
    ;

param
    : COMMA argument param {
        if (($3 && IS_DUMMY($3)) || IS_DUMMY($2))
            $$ = create_expr(dummy_e);
        else {
            $2->next = $3;
            $$ = $2;
        }
    }
    | { $$ = NULL; }
    ;

argument
    : expression {
        $$ = backpatch_tf_lists($1);
    }
    ;

object_definition
    : LEFT_SQUARE_BRACKET rev_elist RIGHT_SQUARE_BRACKET {
        if ($2 && IS_DUMMY($2))
            $$ = create_expr(dummy_e);
        else {
            $2 = reverse_elist($2);
            Expr *result = create_expr(new_table_e);
            result->sym_value = create_temp_symbol();
            emit(tablecreate_opc, result, NULL, NULL, yylineno, 0);
            for (size_t i = 0; $2; $2 = $2->next)
                emit(tablesetelem_opc, $2, create_numeric_expr(i++), result,
                     yylineno, 0);

            $$ = result;
        }
    }
    | LEFT_SQUARE_BRACKET indexed RIGHT_SQUARE_BRACKET {
        if (IS_DUMMY($2))
            $$ = create_expr(dummy_e);
        else {
            Expr *result = create_expr(new_table_e);
            result->sym_value = create_temp_symbol();
            emit(tablecreate_opc, result, NULL, NULL, yylineno, 0);
            for (size_t i = 0; $2; $2 = $2->next)
                emit(tablesetelem_opc, $2->index, $2, result, yylineno, 0);
            $$ = result;
        }
    }
    | LEFT_SQUARE_BRACKET error RIGHT_SQUARE_BRACKET {
        $$ = create_expr(dummy_e);
    }
    ;

indexed
    : indexedelem other_indexed {
        // No list reversing was needed this time around.
        $1->next = $2;
        $$ = $1;
    }
    ;

other_indexed
    : COMMA indexedelem other_indexed {
        $2->next = $3;
        $$ = $2;
    }
    | { $$ = NULL; }
    ;

indexedelem
    : LEFT_CURLY_BRACKET expression COLON {
        $2 = backpatch_tf_lists($2);
    } expression RIGHT_CURLY_BRACKET {
        $2->index = backpatch_tf_lists($5);
        $$ = $2;
    }
    | LEFT_CURLY_BRACKET error COLON expression RIGHT_CURLY_BRACKET {
        $$ = create_expr(dummy_e);
    }
    | LEFT_CURLY_BRACKET expression COLON error RIGHT_CURLY_BRACKET {
        $$ = create_expr(dummy_e);
    }
    | LEFT_CURLY_BRACKET error COLON error RIGHT_CURLY_BRACKET {
        $$ = create_expr(dummy_e);
    }
    ;

block
    : LEFT_CURLY_BRACKET {
        current_scope++;
    } all_statements RIGHT_CURLY_BRACKET {
        SymList_deactivate(symbol_table->scopes[current_scope]);
        current_scope--;
        $$ = $3;
    }
    | LEFT_CURLY_BRACKET error RIGHT_CURLY_BRACKET {
        $$ = create_expr(dummy_e);
    }
    ;

function_name
    : ID {
        SymbolTableEntry *new_entry =
            create_SymbolTableEntry($1, arguments_list, current_scope,
                                    yylineno, USERFUNC_SYMBOL);

        if (SymTable_contains(symbol_table, new_entry->name,
                              current_scope) == 0) {
            SymbolTableEntry *tmp = SymTable_get(symbol_table, new_entry->name);
            if (tmp && tmp->type == LIBFUNC_SYMBOL)
                yyerror("cannot redefine library function");
        } else {
            if (SymTable_get(symbol_table, new_entry->name)->type
                < USERFUNC_SYMBOL)
                yyerror("function name already declared as a variable");
            else
                yyerror("redefinition of function");
        }

        SymTable_put(symbol_table, new_entry->name, new_entry);
        $$ = new_entry;
    }
    | {
        SymbolTableEntry *new_entry =
            create_SymbolTableEntry(NULL, arguments_list, current_scope,
                                    yylineno, USERFUNC_SYMBOL);
        SymTable_put(symbol_table, new_entry->name, new_entry);
        $$ = new_entry;
    }
    ;

function_prefix
    : FUNCTION function_name {
        $$ = $2;
        $$->address_list = create_list(next_quad());
        emit(jump_opc, NULL, NULL, NULL, yylineno, 0);
        $2->taddress = next_quad();
        emit(funcstart_opc, lvalue_expr($$), NULL, NULL, yylineno, 0);
        insert_stack(function_offset_stack, get_curr_scope_offset(),
                     FUNC_OFFSET, NULL);
        enter_scope_space();
        reset_formal_args_offset();
    }
    ;

function_arguments
    : LEFT_PARENTHESES idlist RIGHT_PARENTHESES {
        insert_stack(block_stack, yylineno, FUNC_BLOCK, NULL);
        ArgList_to_SymTable(symbol_table, arguments_list);
        arguments_list = NULL;
        enter_scope_space();
        reset_function_locals_offset();
    }
    ;

function_body
    : block {
        stack_pop(block_stack);
        $$ = get_curr_scope_offset();
        exit_scope_space();
    }
    ;

function_definition
    : function_prefix function_arguments function_body {
        exit_scope_space();
        $1->offset = $3;
        int offset = stack_pop(function_offset_stack)->line_number;
        restore_curr_scope_offset(offset);
        $$ = $1;
        patch_list($1->address_list->next, next_quad());
        emit(funcend_opc, lvalue_expr($1), NULL, NULL, yylineno, 0);
        patch_label($1->address_list->target_quad, next_quad());
        SymTable_get(symbol_table, $1->name)->func_is_open = 0;
    }
    ;

if_prefix
    : IF LEFT_PARENTHESES expression RIGHT_PARENTHESES {
        $3 = backpatch_tf_lists($3);
        emit(if_eq_opc, $3, create_boolean_expr(1), NULL, yylineno,
             next_quad() + 2);
        $$ = next_quad();
        emit(jump_opc, NULL, NULL, NULL, yylineno, 0);
    }
    ;

if_statement
    : if_prefix statement %prec PUREIF {
        patch_label($1, next_quad());
        $$ = $2;
    }
    | if_prefix statement else_prefix statement {
        patch_label($1, $3 + 1);
        patch_label($3, next_quad());
        if ($2 && $4) {
            $$ = $2;
            $$->t_list = NULL;
            $$->f_list = NULL;
            $$->b_list = merge_lists($2->b_list, $4->b_list);
            $$->c_list = merge_lists($2->c_list, $4->c_list);
        } else if ($2)
            $$ = $2;
        else if ($4)
            $$ = $4;
        else
            $$ = NULL;
    }
    ;

else_prefix
    : ELSE {
        $$ = next_quad();
        emit(jump_opc, NULL, NULL, NULL, yylineno, 0);
    }
    ;

while_start
    : WHILE {
        $$ = next_quad();
        insert_stack(block_stack, yylineno, LOOP, NULL);
    }
    ;

while_condition
    : LEFT_PARENTHESES expression RIGHT_PARENTHESES {
        $2 = backpatch_tf_lists($2);
        emit(if_eq_opc, $2, create_boolean_expr(1), NULL, yylineno,
             next_quad() + 2);
        $$ = next_quad();
        emit(jump_opc, NULL, NULL, NULL, yylineno, 0);
    }
    ;

while_statement
    : while_start while_condition statement {
        emit(jump_opc, NULL, NULL, NULL, yylineno, $1);
        patch_label($2, next_quad());
        if ($3) {
            patch_list($3->b_list, next_quad());
            patch_list($3->c_list, $1);
        }
        free(stack_pop(block_stack));
    }
    ;

N   : {
        $$ = next_quad();
        emit(jump_opc, NULL, NULL, NULL, yylineno, 0);
    }
    ;

M   : {
        $$ = next_quad();
    }
    ;

for_prefix
    : FOR LEFT_PARENTHESES rev_elist SEMICOLON M expression SEMICOLON {
        insert_stack(block_stack, yylineno, LOOP, NULL);
        $6 = backpatch_tf_lists($6);

        $$ = create_forPrefix();
        $$->test = $5;
        $$->enter = next_quad();
        emit(if_eq_opc, $6, create_boolean_expr(1), NULL, yylineno, 0);
    }
    ;

for_statement
    : for_prefix N rev_elist RIGHT_PARENTHESES N statement N {
        patch_label($1->enter, $5 + 1);     // true     jump
        patch_label($2, next_quad());       // false    jump
        patch_label($5, $1->test);          // loop     jump
        patch_label($7, $2 + 1);            // closure  jump
        if ($6) {
               patch_list($6->b_list, next_quad());
               patch_list($6->c_list, $2 + 1);
        }
    }
    ;

constant
    : INTEGER   { $$ = create_numeric_expr($1);     }
    | REAL      { $$ = create_numeric_expr($1);     }
    | STRING    { $$ = create_string_expr($1);      }
    | NIL       { $$ = create_expr(nil_e);          }
    | TRUE      { $$ = create_boolean_expr(true);   }
    | FALSE     { $$ = create_boolean_expr(false);  }
    ;

idlist
    : ID {
        if ((SymTable_contains(symbol_table, $1, 0) == 1) &&
            (SymTable_get(symbol_table, $1)->type == LIBFUNC_SYMBOL))
            yyerror("argument name overshadows library function");
        if (!arguments_list)
            arguments_list = SymList_new();
        SymbolTableEntry *new_entry =
            create_SymbolTableEntry($1, NULL, current_scope + 1, yylineno,
                                    FORMAL_SYMBOL);
        SymList_insert(arguments_list, new_entry);
    } other_id { }
    | { arguments_list = NULL; }
    ;

other_id
    : COMMA ID {
        if ((SymTable_contains(symbol_table, $2, 0) == 1) &&
            (SymTable_get(symbol_table, $2)->type == LIBFUNC_SYMBOL))
            yyerror("argument name overshadows library function");

        SymbolTableEntry *new_entry =
            create_SymbolTableEntry($2, NULL, current_scope + 1, yylineno,
                                    FORMAL_SYMBOL);
        if (SymList_get(arguments_list, new_entry->name))
            yyerror("argument name already used");
        else
            SymList_insert(arguments_list, new_entry);
    } other_id { }
    | /* empty */
    ;

return_statement
    : RETURN SEMICOLON {
        emit(ret_opc, NULL, NULL, NULL, yylineno, 0);
    }
    | RETURN expression SEMICOLON {
        $2 = backpatch_tf_lists($2);
        emit(ret_opc, NULL, NULL, $2, yylineno, 0);
    }
    ;
%%



/* Epilogue - untouched code we give to yacc */

void yyerror(const char *yacc_error_message) {
    fprintf(stderr, "At line %d: %s\n", yylineno, yacc_error_message);
    IR_GEN_FAILED = 1;
}

static void ArgList_to_SymTable(SymTable_T sym_table, SymList_T arg_list) {
    list_entry_T *iter = NULL;
    if (arg_list)
        iter = arg_list->head;

    while (iter) {
        iter->entry->space = get_curr_scope_space();
        iter->entry->offset = get_curr_scope_offset();
        inc_curr_scope_offset();
        SymTable_put(sym_table, iter->entry->name, iter->entry);
        iter = iter->next;
    }

    return;
}


FILE *open_file(char *file_name, char *mode) {
    if (!strcmp(file_name, "stdout"))
        return stdout;
    if (!strcmp(file_name, "stderr"))
        return stderr;
    return fopen(file_name, mode);
}


int main(int argc, char** argv) {
    int ret_val;
    yyin = stdin;
    scopes_file = NULL;
    air_file = NULL;
    abc_file = NULL;
    out_file = NULL;

    for (size_t i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-air") && argc > i + 1) {
            air_file = open_file(argv[++i], "w");
        } else if ((!strcmp(argv[i], "--scopesfile") ||
                   !strcmp(argv[i], "-sf")) && argc > i + 1) {
            scopes_file = open_file(argv[++i], "w");
        } else if (!strcmp(argv[i], "-o") && argc > i + 1) {
            out_file = open_file(argv[++i], "wb");
        } else if (!strcmp(argv[i], "-abc") && argc > i + 1) {
            abc_file = open_file(argv[++i], "wb");
        } else {
            yyin = fopen(argv[i], "r");
            if (!out_file) {
                char *prefix = strsep(&argv[i], ".");
                char out_file_name[256];
                strncpy(out_file_name, prefix, 256);
                strncat(out_file_name, ".abc", 256 - strlen(prefix));
                out_file = fopen(out_file_name, "wb");
            }
        }
    }
    if (!out_file)
        out_file = fopen("out.abc", "wb");

    symbol_table = SymTable_new();
    SymTable_init_lib_functions(symbol_table);
    comment_stack = Create_Empty_Stack();
    block_stack = Create_Empty_Stack();
    function_offset_stack = Create_Empty_Stack();
    expr_dummy = create_expr(dummy_e);
    ret_val = yyparse();

    if (yyin != stdin)
        fclose(yyin);
    if (ret_val == 0 && scopes_file) {
        print_scopes(symbol_table);
        fclose(scopes_file);
    }

    if (!IR_GEN_FAILED) {
        if (air_file)
            write_quads_to_file(air_file);
        generate_final_instructions();
        if (abc_file)
            print_tcode(abc_file);
        write_abc(out_file, instructions, userfunctions, libfuncs_used,
                  const_numbers, const_strings);
    }
    fclose(out_file);
    free_stack(comment_stack);
    free_stack(block_stack);
    SymTable_free(symbol_table);

    return 0;
}


static void print_scopes(SymTable_T oSymTable) {
    int i;

    if (!scopes_file)
        return;

    for (i = 0; i < oSymTable->scopes_size; i++) {
        if (oSymTable->scopes[i]->length != 0) {
            fprintf(scopes_file,
                    "\n-----------     Scope #%d     -----------\n", i);
            SymList_apply_all(oSymTable->scopes[i], print_node);
        }
    }
}


static void print_node(void *node) {
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
            assert(0); /* No man's land */
            break;
    }
    fprintf(scopes_file, "%s \"%s\" (line %d) (scope %d)\n", type, entry->name,
            entry->line, entry->scope);
}

