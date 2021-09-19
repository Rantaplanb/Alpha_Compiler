#ifndef ALPHAC_TCODE_GENERATION_H
#define ALPHAC_TCODE_GENERATION_H

#include <stdbool.h>
#include <stdio.h>

#include "intermediate_code.h"

typedef struct vmarg VMarg;
typedef struct instruction Instruction;

typedef void (*generator_func_t)(Quad q);


typedef enum {
    label_a = 0,
    global_a,
    formal_a,
    local_a,
    number_a,
    string_a,
    bool_a,
    nil_a,
    userfunc_a,
    libfunc_a,
    retval_a,
    undefined_a
} VMarg_type;

typedef enum {
    assign_t = 0,
    add_t,
    sub_t,
    mul_t,
    division_t,
    mod_t,
    if_eq_t,
    if_noteq_t,
    if_lesseq_t,
    if_greatereq_t,
    if_less_t,
    if_greater_t,
    jump_t,
    callfunc_t,
    pusharg_t,
    enterfunc_t,
    funcend_t,
    tablecreate_t,
    tablegetelem_t,
    tablesetelem_t,
    nop_t
} VMopcode;

struct vmarg {
    VMarg_type type;
    unsigned int val;
};

struct instruction {
    VMopcode opcode;
    VMarg result;
    VMarg arg1;
    VMarg arg2;
    unsigned int srcLine;
};


unsigned int consts_newstring(char *s);

unsigned int consts_newnumber(double n);

unsigned int libfuncs_newused(char *s);

unsigned int userfuncs_newfunc(SymbolTableEntry *sym);

void make_operand(Expr *e, VMarg *arg);

void make_numberoperand(VMarg *arg, double val);

void make_booloperand(VMarg *arg, unsigned int val);

void make_retvaloperand(VMarg *arg);

void generate(VMopcode op, Quad quad);

void generate_final_instructions(void);

void generate_ADD(Quad q);

void generate_SUB(Quad q);

void generate_MUL(Quad q);

void generate_DIV(Quad q);

void generate_UMINUS(Quad q);

void generate_MOD(Quad q);

void generate_NEWTABLE(Quad q);

void generate_TABLEGETELEM(Quad q);

void generate_TABLESETELEM(Quad q);

void generate_ASSIGN(Quad q);

void generate_NOP();

void generate_JUMP(Quad q);

void generate_IF_EQ(Quad q);

void generate_NOTEQ(Quad q);

void generate_GREATER(Quad q);

void generate_GREATEREQ(Quad q);

void generate_LESS(Quad q);

void generate_LESSEQ(Quad q);

void generate_NOT(Quad q);

void generate_OR(Quad q);

void generate_PARAM(Quad q);

void generate_CALL(Quad q);

void generate_GETRETVAL(Quad q);

void generate_FUNCSTART(Quad q);

void generate_RETURN(Quad q);

void generate_FUNCEND(Quad q);

void generate_AND(Quad q);

void emit_t(Instruction instr);

void copy_bytes(void *src, void *dest, size_t n);

void expand_quads();

void init_instruction(Instruction *instruction);

void reset_operand(VMarg *arg);

void print_tcode(FILE *file);

char *vmarg_to_string(VMarg arg);

char *vmtype_to_string(VMarg_type type);

char *vmopc_to_string(VMopcode op);

#endif  // ALPHAC_TCODE_GENERATION_H
