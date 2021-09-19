#ifndef HANDLERS_H
#define HANDLERS_H

#include "intermediate_code.h"

int is_arithmetic(Opcode oper);

int is_boolean(Opcode oper);

int is_relop(Opcode oper);

Expr* handle_expr_op_expr(Expr* operant1, Opcode oper, Expr* operant2);

#endif
