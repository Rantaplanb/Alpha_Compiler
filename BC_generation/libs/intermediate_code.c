#include "intermediate_code.h"

#define EXPAND_SIZE 1024
#define CURR_SIZE (total_quad_capacity * sizeof(Quad))
#define NEW_SIZE (EXPAND_SIZE * sizeof(Quad) + CURR_SIZE)

Quad *quads = NULL;
unsigned int total_quad_capacity = 0;
unsigned int curr_quad = 0;
unsigned int temp_var_counter = 0;

unsigned int scope = 0;
unsigned int program_variable_offset = 0;
unsigned int function_local_offset = 0;
unsigned int formal_argument_offset = 0;
unsigned int scope_space_counter = 1; /*kata tin isodo se formal args i
 sunartisi auksanetai kata 1, kata tin eksodo apo sunartisi meiwnetai kata 2 */

/*Se periptwsi emfwleumenwn sunartisewn, prepei na swsoume
 * to offset tis periexousas sunartisis prin to midenisoume */
Stack_T *function_offset_stack;

void init_function_offset_stack() {
    function_offset_stack = Create_Empty_Stack();
    return;
}

scope_space get_curr_scope_space() {
    if (scope_space_counter == 1)
        return program_variable;
    else if (scope_space_counter % 2 == 0)
        return formal_argument;
    else
        return function_local;
}

unsigned int get_curr_scope_offset() {
    switch (get_curr_scope_space()) {
        case program_variable:
            return program_variable_offset;
        case function_local:
            return function_local_offset;
        case formal_argument:
            return formal_argument_offset;
        default:
            assert(0);
    }
}

void enter_scope_space() { ++scope_space_counter; }

void exit_scope_space() {
    assert(scope_space_counter > 1);
    --scope_space_counter;
}

void inc_curr_scope_offset() {
    switch (get_curr_scope_space()) {
        case program_variable:
            ++program_variable_offset;
            break;
        case function_local:
            ++function_local_offset;
            break;
        case formal_argument:
            ++formal_argument_offset;
            break;
        default:
            assert(0);
    }
}

Expr *backpatch_tf_lists(Expr *expr) {
    if (!expr) return NULL;
    if (!expr->f_list) return expr;
    Expr *result = create_expr(bool_expr_e);
    result->sym_value = create_temp_symbol();
    patch_list(expr->t_list, next_quad());
    emit(assign_opc, create_boolean_expr(true), NULL, result, yylineno, 0);
    emit(jump_opc, NULL, NULL, NULL, yylineno, next_quad() + 2);
    patch_list(expr->f_list, next_quad());
    emit(assign_opc, create_boolean_expr(false), NULL, result, yylineno, 0);
    return result;
}


void reset_formal_args_offset() { formal_argument_offset = 0; }

void reset_function_locals_offset() { function_local_offset = 0; }

void restore_curr_scope_offset(unsigned int n) {
    switch (get_curr_scope_space()) {
        case program_variable:
            //program_variable_offset = n; TODO TEST THIS
            break;
        case function_local:
            function_local_offset = n;
            break;
        case formal_argument:
            formal_argument_offset = n;
            break;
        default:
            assert(0);
    }
}

ForPrefix *create_forPrefix() { return malloc(sizeof(ForPrefix)); }

Call *create_call(Expr *rev_elist, unsigned char method, char *name) {
    Call *result = calloc(1, sizeof(Call));

    result->rev_elist = rev_elist;
    result->method = method;
    result->name = name;

    return result;
}

LabelList *create_list(unsigned int label) {
    LabelList *res = malloc(sizeof(LabelList));
    res->target_quad = label;
    res->next = NULL;
    return res;
}

LabelList *merge_lists(LabelList *l1, LabelList *l2) {
    if (!l1)
        return l2;
    else if (!l2)
        return l1;
    for (LabelList *curr = l1; 1; curr = curr->next) {
        if (!curr->next) {
            curr->next = l2;
            break;
        }
    }
    return l1;
}

Expr *create_expr(ExprType type) {
    Expr *e = malloc(sizeof(Expr));
    memset(e, 0, sizeof(Expr));
    e->type = type;
    e->f_list = NULL;
    e->t_list = NULL;
    e->b_list = NULL;
    e->c_list = NULL;
    return e;
}

Expr *create_string_expr(char *str) {
    assert(str);
    Expr *new_expr = malloc(sizeof(Expr));
    memset(new_expr, 0, sizeof(Expr));
    new_expr->type = const_string_e;
    new_expr->string_value = str;
    return new_expr;
}

Expr *create_numeric_expr(double num) {
    Expr *new_expr = malloc(sizeof(Expr));
    memset(new_expr, 0, sizeof(Expr));
    new_expr->type = const_num_e;
    new_expr->numeric_value = num;
    return new_expr;
}

Expr *create_boolean_expr(bool boolean) {
    Expr *new_expr = malloc(sizeof(Expr));
    memset(new_expr, 0, sizeof(Expr));
    new_expr->type = const_bool_e;
    new_expr->bool_value = boolean;

    return new_expr;
}

void emit(Opcode op, Expr *arg1, Expr *arg2, Expr *result, unsigned int line,
          unsigned int label) {
    if (total_quad_capacity == curr_quad) {
        expand_quads();
    }

    Quad *new_quad = quads + curr_quad++;
    new_quad->arg1 = arg1;
    new_quad->arg2 = arg2;
    new_quad->result = result;
    new_quad->line = line;
    new_quad->label = label;
    new_quad->op = op;
}

void copy_bytes(void *src, void *dest, size_t n) {
    size_t i;
    char *casted_src = (char *)src;
    char *casted_dest = (char *)dest;
    for (i = 0; i < n; i++) {
        casted_dest[i] = casted_src[i];
    }
}

void expand_quads() {
    Quad *resized = malloc(NEW_SIZE);
    if (!resized) {
        printf("memory allocation failed");
    }
    copy_bytes(quads, resized, CURR_SIZE);
    free(quads);
    total_quad_capacity = total_quad_capacity + EXPAND_SIZE;
    quads = resized;
    return;
}

char *new_temp_name() {
    int digit_count = 0, tmp = temp_var_counter++;
    if (tmp == 0) return "@t0";
    while (tmp > 0) {
        tmp = (int)tmp / 10;
        digit_count++;
    }
    char *new_name = malloc((2 + digit_count) * sizeof(char));
    sprintf(new_name, "@t%d", temp_var_counter - 1);
    return new_name;
}

/* Used for intermediate temporary variables */
SymbolTableEntry *create_temp_symbol() {
    char *name = new_temp_name();
    if (SymTable_contains(symbol_table, name, 0)) {
        SymbolTableEntry *tmp = SymTable_get(symbol_table, name);
        if (tmp->isActive) return tmp;
    }
    SymbolTableEntry *new_symbol = create_SymbolTableEntry(
        name, NULL, 0, yylineno, LOCAL_SYMBOL);
    new_symbol->offset = program_variable_offset;
    program_variable_offset++;
    SymTable_put(symbol_table, new_symbol->name, new_symbol);
    return new_symbol;
}

Expr *lvalue_expr(SymbolTableEntry *symbol) {
    assert(symbol);
    Expr *new_expr = NULL;

    switch (symbol->type) {
        case GLOBAL_SYMBOL:
        case LOCAL_SYMBOL:
        case FORMAL_SYMBOL:
            new_expr = create_expr(var_e);
            break;
        case USERFUNC_SYMBOL:
            new_expr = create_expr(program_func_e);
            break;
        case LIBFUNC_SYMBOL:
            new_expr = create_expr(library_func_e);
            break;
        default:
            assert(0);
    }

    new_expr->sym_value = symbol;
    return new_expr;
}

void reset_temp_var_counter() {
    temp_var_counter = 0;
    return;
}

SymbolTableEntry *create_temp_var_symbol(SymbolType type) {
    char *symbol_name = new_temp_name();
    if (SymTable_contains(symbol_table, symbol_name, current_scope)) {
        printf("Symbol already exists!");
        return SymTable_get(symbol_table, symbol_name);
    } else {
        return create_SymbolTableEntry(symbol_name, NULL, current_scope,
                                       yylineno, type);
    }
}

Expr *member_item(Expr *lv, char *name) {
    lv = emit_if_table_item(lv);  // Emit code if r-value use of table item
    Expr *ti = create_expr(table_item_e);  // Make a new expression
    ti->sym_value = lv->sym_value;
    ti->index = create_string_expr(name);  // Const string index
    return ti;
}

Expr *emit_if_table_item(Expr *e) {
    if (e->type != table_item_e)
        return e;
    else {
        Expr *result = create_expr(var_e);
        result->sym_value = create_temp_symbol();
        emit(tablegetelem_opc, e, e->index, result, e->sym_value->line, 0);
        return result;
    }
}

Expr *make_call(Expr *lv, Expr *rev_elist, unsigned int line) {
    Expr *func = emit_if_table_item(lv);
    while (rev_elist) {
        emit(param_opc, rev_elist, NULL, NULL, line, 0);
        rev_elist = rev_elist->next;
    }
    emit(call_opc, func, NULL, NULL, line, 0);
    Expr *result = create_expr(var_e);
    result->sym_value = create_temp_symbol();
    emit(getretval_opc, NULL, NULL, result, line, 0);
    return result;
}

// Use this function to check correct use of
// of expression in arithmetic operation
void check_arith_expr(Expr *e, const char *context) {
    if (e->type == const_bool_e || e->type == const_string_e ||
        e->type == nil_e || e->type == new_table_e ||
        e->type == program_func_e || e->type == library_func_e ||
        e->type == bool_expr_e) {
        char error_msg[150];
        sprintf(error_msg, "illegal expr used in %s", context);
        yyerror(error_msg);
        IR_GEN_FAILED = 1;
    }
}

void check_matching_expr(Expr *e1, Expr *e2, const char *context) {
    if (e1->type == var_e || e2->type == var_e) return;
    if ((e1->type == e2->type && e1->type != undefined_e) ||
        (e1->type == arith_expr_e && e2->type == const_num_e) ||
        (e2->type == arith_expr_e && e1->type == const_num_e) ||
        (e1->type == bool_expr_e && e2->type == const_bool_e) ||
        (e2->type == bool_expr_e && e1->type == const_bool_e))
        return;

    char error_msg[150];
    sprintf(error_msg, "in %s the expression types must match", context);
    yyerror(error_msg);
    IR_GEN_FAILED = 1;

    return;
}

char *opcode_to_string(Opcode op) {
    switch (op) {
        case assign_opc:
            return "assign";
        case add_opc:
            return "add";
        case sub_opc:
            return "sub";
        case mul_opc:
            return "mul";
        case div_opc:
            return "div";
        case mod_opc:
            return "mod";
        case uminus_opc:
            return "uminus";
        case and_opc:
            return "and";
        case or_opc:
            return "or";
        case not_opc:
            return "not";
        case if_eq_opc:
            return "if_eq";
        case if_noteq_opc:
            return "if_noteq";
        case if_lesseq_opc:
            return "if_lesseq";
        case if_greatereq_opc:
            return "if_greatereq";
        case if_less_opc:
            return "if_less";
        case if_greater_opc:
            return "if_greater";
        case call_opc:
            return "call";
        case param_opc:
            return "param";
        case ret_opc:
            return "ret";
        case getretval_opc:
            return "getretval";
        case funcstart_opc:
            return "funcstart";
        case funcend_opc:
            return "funcend";
        case tablecreate_opc:
            return "tablecreate";
        case tablegetelem_opc:
            return "tablegetelem";
        case tablesetelem_opc:
            return "tablesetelem";
        case jump_opc:
            return "jump";
        default:
            assert(0);
    }
}

char *expr_to_string(Expr *expr) {
    char *tmp = calloc(16, sizeof(char));
    char *result = calloc(25, sizeof(char));

    if (!expr) return "";

    switch (expr->type) {
        case (undefined_e):
            assert(0);  // TODO: check what to do
            free(tmp);
            break;
        case (var_e):
            free(tmp);
            sprintf(result, "\"%s\"", expr->sym_value->name);
            return expr->sym_value->name;
        case (table_item_e):
            free(tmp);
            return expr->sym_value->name;
        case (program_func_e):
            free(tmp);
            return expr->sym_value->name;
        case (library_func_e):
            free(tmp);
            return expr->sym_value->name;
        case (arith_expr_e):
            free(tmp);
            return expr->sym_value->name;
        case (bool_expr_e):
            free(tmp);
            return expr->sym_value->name;
        case (assign_expr_e):
            free(tmp);
            return expr->sym_value->name;
        case (new_table_e):
            free(tmp);
            return expr->sym_value->name;
        case (const_bool_e):
            free(tmp);
            return expr->bool_value ? "true" : "false";
        case (const_string_e):
            free(tmp);
            snprintf(result, 25, "\"%s\"", expr->string_value);
            return result;
        case (nil_e):
            free(tmp);
            return "Nil";
        case (const_num_e):
            sprintf(tmp, "%.0d%.8g", (int)(expr->numeric_value) / 10,
                    (expr->numeric_value) - ((int)(expr->numeric_value) -
                                             (int)(expr->numeric_value) % 10));
            return tmp;
        default:
            assert(0);
    }
}

unsigned int is_temp_name(char *s) { return *s == '@'; }

unsigned int is_temp_expr(Expr *e) {
    return e->sym_value && is_temp_name(e->sym_value->name);
}

void write_quads_to_file(FILE *fp) {
    unsigned int i;
    if (!fp) {
        printf("File could not be opened!\n");
        return;
    }
    fprintf(fp,
            "   Quad#"
            "        Opcode"
            "        Result"
            "          Arg1"
            "          Arg2"
            "   Label\n");
    fprintf(fp,
            "-----------------------------------------------------------------"
            "-------\n");

    for (i = 0; i < curr_quad; i++) {
        Quad curr = quads[i];
        fprintf(fp, "%8d%14s%14s%14s%14s", i + 1, opcode_to_string(curr.op),
                expr_to_string(curr.result), expr_to_string(curr.arg1),
                expr_to_string(curr.arg2));
        if (curr.op >= if_eq_opc && curr.op <= jump_opc)
            fprintf(fp, "%8d", curr.label + 1);
        fprintf(fp, "\n");
    }
}

void patch_label(unsigned quad_no, unsigned int label) {
    assert(quad_no < curr_quad && !quads[quad_no].label);
    quads[quad_no].label = label;
}

void patch_list(LabelList *head, unsigned int label) {
    for (LabelList *tmp = head; tmp; tmp = tmp->next)
        patch_label(tmp->target_quad, label);
}

unsigned int next_quad(void) { return curr_quad; }  // FIXME

void make_stmt(StmtType *s) { s->break_list = s->cont_list = 0; }

int new_list(int i) {
    quads[i].label = 0;
    return i;
}

bool expr_to_bool(Expr *expr) {
    switch (expr->type) {
        case (const_num_e):
            return expr->numeric_value != 0;
        case (const_bool_e):
            return expr->bool_value;
        case (const_string_e):
            return strcmp(expr->string_value, "");
        case (nil_e):
            return false;
        case (program_func_e):
        case (library_func_e):
        case (new_table_e):
            return true;
        default:
            assert(0);  // TODO: check what to do here
    }
}

bool expr_transformable_to_bool(Expr *expr) {
    switch (expr->type) {
        case (const_num_e):
        case (const_bool_e):
        case (const_string_e):
        case (nil_e):
        case (program_func_e):
        case (library_func_e):
        case (new_table_e):
            return true;
        default:
            return false;
    }
}


bool is_known_at_comptime(Expr *expr) {
    switch (expr->type) {
        case (const_num_e):
        case (const_bool_e):
        case (const_string_e):
        case (nil_e):
            return true;
        default:
            return false;
    }
}

Expr *reverse_elist(Expr *list_head) {
    if (!list_head || !list_head->next) return list_head;

    Expr *result = reverse_elist(list_head->next);
    list_head->next->next = list_head;
    list_head->next = NULL;

    return result;
}

// TODO: const evaluation
Expr *handle_arith_expr_op_expr(Expr *operand1, Opcode operator,
                                Expr * operand2) {
    Expr *result;

    result = create_expr(arith_expr_e);
    result->sym_value = create_temp_symbol();
    emit(operator, operand1, operand2, result, yylineno, 0);
    return result;
}
