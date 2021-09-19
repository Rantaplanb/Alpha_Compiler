#include "handlers.h"

#include <assert.h>
#include <stdbool.h>

#include "globals.h"
#include "string.h"
#include "symtable.h"

// returns 0 if strings are equal
int is_arithmetic(Opcode operator) {
    switch (operator) {
        case (add_opc):
        case (sub_opc):
        case (mul_opc):
        case (div_opc):
        case (mod_opc):
            return 1;
        default:
            return 0;
    }
}

int is_boolean(Opcode operator) {
    switch (operator) {
        case (and_opc):
        case (or_opc):
            return 1;
        default:
            return 0;
    }
}

int is_relop(Opcode operator) {
    switch (operator) {
        case (if_eq_opc):
        case (if_noteq_opc):
        case (if_lesseq_opc):
        case (if_greatereq_opc):
        case (if_less_opc):
        case (if_greater_opc):
            return 1;
        default:
            return 0;
    }
}

int is_numeric_expr(Expr* e) {
    if (e->type == arith_expr_e || e->type == const_num_e) return 1;
    return 0;
}

// TODO: const evaluation
Expr* handle_expr_op_expr(Expr* operand1, Opcode operator, Expr * operand2) {
    Expr* result;

    if (is_arithmetic(operator)) {
        result = create_expr(arith_expr_e);
        result->sym_value = create_temp_symbol();
        emit(operator, operand1, operand2, result, yylineno, 0);
        return result;
    } else if (is_boolean(operator)) {
        result = create_expr(bool_expr_e);
        result->sym_value = create_temp_symbol();
        emit(assign_opc, create_boolean_expr(true), NULL, result, yylineno, 0);
        emit(jump_opc, NULL, NULL, NULL, yylineno, next_quad() + 2);
        emit(assign_opc, create_boolean_expr(false), NULL, result, yylineno, 0);
        return result;
    } else if (is_relop(operator)) {
        result = create_expr(bool_expr_e);
        result->sym_value = create_temp_symbol();
        emit(operator, operand1, operand2, result, yylineno, next_quad() + 2);
        // emit(assign_opc, create_boolean_expr(false), NULL, result, yylineno,
        // 0);
        emit(jump_opc, NULL, NULL, NULL, yylineno, next_quad() + 3);
        emit(assign_opc, create_boolean_expr(true), NULL, result, yylineno, 0);
        emit(jump_opc, NULL, NULL, NULL, yylineno, next_quad() + 2);
        emit(assign_opc, create_boolean_expr(false), NULL, result, yylineno, 0);
        return result;
    }

    assert(0);
}
