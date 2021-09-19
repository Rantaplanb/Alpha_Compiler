#include "tcode_generation.h"

#include <string.h>

#include "abc_writer.h"
#include "arraylist.h"
#include "globals.h"
#include "symtable.h"

extern Quad *quads;
extern unsigned int curr_quad;
static Stack_T *funcstack;
unsigned int current_processed_quad = 0;
ArrayList *instructions = NULL;
ArrayList *userfunctions = NULL;
ArrayList *const_strings = NULL;
ArrayList *libfuncs_used = NULL;
ArrayList *const_numbers = NULL;

generator_func_t generators[] = {
        generate_ASSIGN, generate_ADD, generate_SUB,
        generate_MUL, generate_DIV, generate_MOD,
        generate_UMINUS, generate_IF_EQ, generate_NOTEQ,
        generate_LESSEQ, generate_GREATEREQ, generate_LESS,
        generate_GREATER, generate_JUMP, generate_CALL,
        generate_PARAM, generate_RETURN, generate_GETRETVAL,
        generate_FUNCSTART, generate_FUNCEND, generate_NEWTABLE,
        generate_TABLEGETELEM, generate_TABLESETELEM, generate_NOP};

/*
 * unsigned int program_variable_offset = 0;
 * unsigned int function_local_offset = 0;
 * unsigned int formal_argument_offset = 0;
 * unsigned int scope_space_counter = 1;
 */

unsigned int consts_newstring(char *s) {
    assert(s);
    if (!const_strings) const_strings = create_arraylist();
    for (int i = 0; i < const_strings->current_size; i++) {
        if (!strcmp(((char *) get_arraylist(const_strings, i)), s)) {
            return i;
        }
    }
    insert_arraylist(const_strings, s);
    return const_strings->current_size - 1;
}

unsigned int consts_newnumber(double n) {
    if (!const_numbers) const_numbers = create_arraylist();
    for (int i = 0; i < const_numbers->current_size; i++) {
        if (*((double *) get_arraylist(const_numbers, i)) == n) {
            return i;
        }
    }
    double *elem = malloc(sizeof(double));
    *elem = n;
    insert_arraylist(const_numbers, elem);
    return const_numbers->current_size - 1;
}

unsigned int libfuncs_newused(char *s) {
    assert(s);
    if (!libfuncs_used) libfuncs_used = create_arraylist();
    for (int i = 0; i < libfuncs_used->current_size; i++) {
        if (!strcmp(((char *) get_arraylist(libfuncs_used, i)), s)) {
            return i;
        }
    }
    insert_arraylist(libfuncs_used, s);
    return libfuncs_used->current_size - 1;
}

unsigned int userfuncs_newfunc(SymbolTableEntry *sym) {
    assert(sym);
    if (!userfunctions) userfunctions = create_arraylist();
    for (int i = 0; i < userfunctions->current_size; i++) {
        SymbolTableEntry *tmp = (SymbolTableEntry *) get_arraylist(userfunctions, i);
        if ((!strcmp(tmp->name, sym->name)) && tmp->taddress == sym->taddress) {
            return i;
        }
    }
    insert_arraylist(userfunctions, sym);
    return userfunctions->current_size - 1;
}

void make_operand(Expr *e, VMarg *arg) {
    if (!e) {
        arg->type = undefined_a;
        arg->val = 0;
        return;
    }

    switch (e->type) {
        /* all those below use a var for storage */
        case assign_expr_e:
        case var_e:
        case table_item_e:
        case arith_expr_e:
        case bool_expr_e:
        case new_table_e:
            assert(e->sym_value);
            arg->val = e->sym_value->offset;

            switch (e->sym_value->space) {
                case program_variable:
                    arg->type = global_a;
                    break;
                case function_local:
                    arg->type = local_a;
                    break;
                case formal_argument:
                    arg->type = formal_a;
                    break;
                default:
                    assert(0);
            }
            break; /* from case new_table_e */
        case const_bool_e:
            arg->val = e->bool_value;
            arg->type = bool_a;
            break;
        case const_string_e:
            arg->val = consts_newstring(e->string_value);
            arg->type = string_a;
            break;
        case const_num_e:
            arg->val = consts_newnumber(e->numeric_value);
            arg->type = number_a;
            break;
        case nil_e:
            arg->type = nil_a;
            break;
            /* Functions */
        case program_func_e:
            arg->type = userfunc_a;
            arg->val = userfuncs_newfunc(e->sym_value);
            break;
        case library_func_e:
            arg->type = libfunc_a;
            arg->val = libfuncs_newused(e->sym_value->name);
            break;
        default:
            assert(0);
    }
}

/*
 * Helper functions to produce common arguments for
 * generated instructions, like 1, 0, "true", "false"
 * and function return values.
 */
void make_numberoperand(VMarg *arg, double val) {
    arg->val = consts_newnumber(val);
    arg->type = number_a;
}

void make_booloperand(VMarg *arg, unsigned int val) {
    arg->val = val;
    arg->type = bool_a;
}

void make_retvaloperand(VMarg *arg) { arg->type = retval_a; }

void generate(VMopcode op, Quad quad) {
    Instruction t;
    init_instruction(&t);
    t.srcLine = quad.line;
    t.opcode = op;
    make_operand(quad.arg1, &t.arg1);
    make_operand(quad.arg2, &t.arg2);
    if (op >= if_eq_t && op <= if_greater_t) {
        t.result.type = label_a;
        t.result.val = quad.label;
    } else
        make_operand(quad.result, &t.result);

    emit_t(t);
}

void generate_ADD(Quad q) { generate(add_t, q); }

void generate_SUB(Quad q) { generate(sub_t, q); }

void generate_MUL(Quad q) { generate(mul_t, q); }

void generate_DIV(Quad q) { generate(division_t, q); }

void generate_UMINUS(Quad q) {
    q.arg2 = create_numeric_expr(-1);
    generate(mul_t, q);
}

void generate_MOD(Quad q) { generate(mod_t, q); }

void generate_NEWTABLE(Quad q) { generate(tablecreate_t, q); }

void generate_TABLEGETELEM(Quad q) { generate(tablegetelem_t, q); }

void generate_TABLESETELEM(Quad q) { generate(tablesetelem_t, q); }

void generate_ASSIGN(Quad q) { generate(assign_t, q); }

void generate_NOP() {
    Instruction t;
    init_instruction(&t);
    t.opcode = nop_t;
    emit_t(t);
}

void generate_JUMP(Quad q) {
    Instruction t;
    init_instruction(&t);
    t.srcLine = q.line;
    t.opcode = jump_t;
    t.result.type = label_a;
    t.result.val = q.label;
    emit_t(t);
}

void generate_IF_EQ(Quad q) { generate(if_eq_t, q); }

void generate_NOTEQ(Quad q) { generate(if_noteq_t, q); }

void generate_GREATER(Quad q) { generate(if_greater_t, q); }

void generate_GREATEREQ(Quad q) { generate(if_greatereq_t, q); }

void generate_LESS(Quad q) { generate(if_less_t, q); }

void generate_LESSEQ(Quad q) { generate(if_lesseq_t, q); }

void generate_PARAM(Quad q) {
    Instruction t;
    init_instruction(&t);
    t.srcLine = q.line;
    t.opcode = pusharg_t;
    make_operand(q.arg1, &t.arg1);  // TODO: check if arg1 is right
    emit_t(t);
}

void generate_CALL(Quad q) {
    Instruction t;
    init_instruction(&t);
    t.srcLine = q.line;
    t.opcode = callfunc_t;
    make_operand(q.arg1, &t.arg1);  // TODO: check if arg1 is right
    emit_t(t);
}

void generate_GETRETVAL(Quad q) {
    Instruction t;
    init_instruction(&t);
    t.srcLine = q.line;
    t.opcode = assign_t;
    make_operand(q.result, &t.result);
    make_retvaloperand(&t.arg1);  // TODO: check if arg1 is right
    emit_t(t);
}

void generate_FUNCSTART(Quad q) {
    Instruction t;
    init_instruction(&t);
    t.srcLine = q.line;

    t.opcode = enterfunc_t;
    make_operand(q.arg1, &t.result);
    emit_t(t);
}

void generate_RETURN(Quad q) {
    Instruction t;
    init_instruction(&t);
    t.srcLine = q.line;
    t.opcode = assign_t;
    make_retvaloperand(&t.result);

    make_operand(q.result, &t.arg1);
    emit_t(t);
}

void reset_operand(VMarg *arg) {
    arg->val = 0;
    arg->type = undefined_a;
}

void init_instruction(Instruction *instruction) {
    reset_operand(&instruction->arg1);
    reset_operand(&instruction->arg2);
    reset_operand(&instruction->result);
    instruction->opcode = nop_t;
}

void generate_FUNCEND(Quad q) {
    Instruction t;
    init_instruction(&t);
    t.srcLine = q.line;
    t.opcode = funcend_t;
    make_operand(q.arg1, &t.result);
    emit_t(t);
}

void generate_final_instructions(void) {
    funcstack = Create_Empty_Stack();
    instructions = create_arraylist();
    for (current_processed_quad = 0; current_processed_quad < curr_quad;
         ++current_processed_quad) {
        assert(quads[current_processed_quad].op <= tablesetelem_opc);
        (*generators[quads[current_processed_quad].op])(
                quads[current_processed_quad]);
    }
}

unsigned int next_instruction_label(void) {
    assert(instructions);
    return instructions->current_size;
}

void emit_t(Instruction instr) {
    Instruction *new_instruction = malloc(sizeof(Instruction));

    new_instruction->arg1 = instr.arg1;
    new_instruction->arg2 = instr.arg2;
    new_instruction->result = instr.result;
    new_instruction->srcLine = instr.srcLine;
    new_instruction->opcode = instr.opcode;
    insert_arraylist(instructions, new_instruction);
}

void print_tcode(FILE *fp) {
    unsigned int i;
    if (!fp) {
        printf("File could not be opened!\n");
        return;
    }
    fprintf(fp,
            "Instruction#"
            "          Opcode"
            "          Result"
            "            Arg1"
            "            Arg2\n");
    fprintf(fp,
            "-----------------------------------------------------------------"
            "-----------\n");

    for (i = 0; i < instructions->current_size; i++) {
        Instruction *curr = get_arraylist(instructions, i);
        fprintf(fp, "%12d%16s%16s%16s%16s\n", i + 1,
                vmopc_to_string(curr->opcode), vmarg_to_string(curr->result),
                vmarg_to_string(curr->arg1), vmarg_to_string(curr->arg2));
    }

    if (const_numbers) {
        fprintf(fp, "\nConst Numbers:\n");
        for (i = 0; i < const_numbers->current_size; ++i)
            fprintf(fp, "%d\t%lf\n", i, *((double *) const_numbers->element[i]));
        fprintf(
                fp,
                "-----------------------------------------------------------------"
                "-----------\n");
    }

    if (const_strings) {
        fprintf(fp, "\nConst Strings:\n");
        for (i = 0; i < const_strings->current_size; ++i)
            fprintf(fp, "%d\t%s\n", i, (char *) const_strings->element[i]);
        fprintf(
                fp,
                "-----------------------------------------------------------------"
                "-----------\n");
    }

    if (userfunctions) {
        fprintf(fp, "\nUser Functions:\n");
        for (i = 0; i < userfunctions->current_size; ++i)
            fprintf(fp, "%d\t%s\n", i,
                    ((SymbolTableEntry *) userfunctions->element[i])->name);
        fprintf(
                fp,
                "-----------------------------------------------------------------"
                "-----------\n");
    }

    if (libfuncs_used) {
        fprintf(fp, "\nLibrary Functions:\n");
        for (i = 0; i < libfuncs_used->current_size; ++i)
            fprintf(fp, "%d\t%s\n", i, (char *) libfuncs_used->element[i]);
        fprintf(
                fp,
                "-----------------------------------------------------------------"
                "-----------\n");
    }
}


char *vmarg_to_string(VMarg arg) {
    char *buffer = malloc(16 * sizeof(char));

    if (arg.type == undefined_a) return "";

    if (arg.type == retval_a)
        sprintf(buffer, "%d(%s)", arg.type, vmtype_to_string(arg.type));
    else
        sprintf(buffer, "%d(%s),%d", arg.type, vmtype_to_string(arg.type),
                arg.type == label_a ? arg.val + 1 : arg.val);

    return buffer;
}

char *vmtype_to_string(VMarg_type type) {
    switch (type) {
        case label_a:
            return "label";
        case global_a:
            return "global";
        case formal_a:
            return "formal";
        case local_a:
            return "local";
        case number_a:
            return "number";
        case string_a:
            return "string";
        case bool_a:
            return "bool";
        case nil_a:
            return "nil";
        case userfunc_a:
            return "userfunc";
        case libfunc_a:
            return "libfunc";
        case retval_a:
            return "retval";
        case undefined_a:
            return "undefined";
        default:
            assert(0);
    }
}

char *vmopc_to_string(VMopcode op) {
    switch (op) {
        case assign_t:
            return "assign";
        case add_t:
            return "add";
        case sub_t:
            return "sub";
        case mul_t:
            return "mul";
        case division_t:
            return "div";
        case mod_t:
            return "mod";
        case if_eq_t:
            return "if_eq";
        case if_noteq_t:
            return "if_noteq";
        case if_lesseq_t:
            return "if_lesseq";
        case if_greatereq_t:
            return "if_greatereq";
        case if_less_t:
            return "if_less";
        case if_greater_t:
            return "if_greater";
        case callfunc_t:
            return "call";
        case pusharg_t:
            return "pusharg";
        case enterfunc_t:
            return "enterfunc";
        case funcend_t:
            return "exitfunc";
        case tablecreate_t:
            return "tablecreate";
        case tablegetelem_t:
            return "tablegetelem";
        case tablesetelem_t:
            return "tablesetelem";
        case jump_t:
            return "jump";
        default:
            assert(0);
    }
}
