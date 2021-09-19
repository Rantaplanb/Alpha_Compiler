#ifndef INTERMEDIATE_CODE_H
#define INTERMEDIATE_CODE_H

#include <stdbool.h>

#include "stack.h"
#include "structs.h"
#include "symtable.h"

typedef enum {
    assign_opc,
    add_opc,
    sub_opc,
    mul_opc,
    div_opc,
    mod_opc,
    uminus_opc,
    and_opc,
    or_opc,
    not_opc,
    if_eq_opc,
    if_noteq_opc,
    if_lesseq_opc,
    if_greatereq_opc,
    if_less_opc,
    if_greater_opc,
    jump_opc,
    call_opc,
    param_opc,
    ret_opc,
    getretval_opc,
    funcstart_opc,
    funcend_opc,
    tablecreate_opc,
    tablegetelem_opc,
    tablesetelem_opc
} Opcode;

typedef enum {
    undefined_e = 0,  // FIXME: maybe var_e takes its place
    dummy_e,
    var_e,
    table_item_e,
    program_func_e,
    library_func_e,
    arith_expr_e,
    bool_expr_e,
    assign_expr_e,  // FIXME: redundant - check parser?
    new_table_e,
    const_num_e,
    const_bool_e,
    const_string_e,
    nil_e
} ExprType;

typedef struct LabelList {
    unsigned int target_quad;
    struct LabelList* next;
} LabelList;

typedef struct Expr {
    ExprType type;
    char* string_value;
    double numeric_value;
    bool bool_value;
    struct Expr* index;
    SymbolTableEntry* sym_value;
    LabelList* t_list;
    LabelList* f_list;
    LabelList* b_list;
    LabelList* c_list;
    struct Expr* next;
} Expr;

typedef struct Call {
    Expr* rev_elist;
    unsigned char method;
    char* name;
} Call;
// TODO check this if useful
typedef struct StmtType {
    int break_list, cont_list;
} StmtType;

typedef struct ForPrefix {
    int test, enter;
} ForPrefix;

typedef struct Quad {
    Opcode op;
    Expr* result;
    Expr* arg1;
    Expr* arg2;
    unsigned int label;
    unsigned int line;
} Quad;

ForPrefix* create_forPrefix();

LabelList* create_list(unsigned int label);

LabelList* merge_lists(LabelList* l1, LabelList* l2);

Call* create_call(Expr* rev_elist, unsigned char method, char* name);

Expr* create_expr(ExprType type);

Expr* create_string_expr(char* str);

Expr* create_numeric_expr(double num);

Expr* create_boolean_expr(bool boolean);

Expr* backpatch_tf_lists(Expr* expr);

void emit(Opcode op, Expr* arg1, Expr* arg2, Expr* result, unsigned int line,
          unsigned int label);

void copy_bytes(void* src, void* dest, size_t n);

unsigned int get_next_quad_label();

SymbolTableEntry* create_temp_symbol();

void expand_quads();

char* new_temp_name();

void reset_temp_var_counter();

SymbolTableEntry* create_temp_var_symbol(SymbolType type);

Expr* lvalue_expr(SymbolTableEntry* symbol);

void exit_scope_space();

void enter_scope_space();

void restore_curr_scope_offset(unsigned int n);

void reset_formal_args_offset();

void reset_function_locals_offset();

Expr* emit_if_table_item(Expr* e);

Expr* make_call(Expr* lv, Expr* rev_elist, unsigned int line);

void check_arith_expr(Expr* e, const char* context);

void check_matching_expr(Expr* e1, Expr* e2, const char* context);

char* opcode_to_string(Opcode op);

char* expr_to_string(Expr* ex);

unsigned int is_temp_name(char* s);

unsigned int is_temp_expr(Expr* e);

void write_quads_to_file(FILE* fp);

void patch_label(unsigned int quad_no, unsigned int label);

unsigned int next_quad();

void make_stmt(StmtType* s);

int new_list(int i);

void inc_curr_scope_offset();

scope_space get_curr_scope_space();

unsigned int get_curr_scope_offset();

Expr* member_item(Expr* lv, char* name);

bool expr_to_bool(Expr* expr);

bool expr_transformable_to_bool(Expr* expr);

bool is_known_at_comptime(Expr* expr);

Expr* reverse_elist(Expr* list_head);

void patch_list(LabelList* head, unsigned label);

#endif
