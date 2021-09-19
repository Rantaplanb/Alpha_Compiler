#ifndef __COMMENT_STACK_C__
#define __COMMENT_STACK_C__

#include "stack.h"

#include <stdio.h>
#include <stdlib.h>

#endif


Stack_T *Create_Empty_Stack() {
    Stack_T *res = malloc(sizeof(Stack_T));

    res->size = 0;
    res->top = NULL;
    return res;
}

stack_node_t *stack_pop(Stack_T *stack) {
    if (stack->size == 0) {
        return NULL;
    }
    stack_node_t *res = stack->top;
    stack->size--;
    stack->top = res->prev;
    res->prev = NULL;
    return res;
}

void insert_stack(Stack_T *stack, int line_number, StackType type) {
    stack_node_t *node = (stack_node_t *)malloc(sizeof(stack_node_t));
    node->line_number = line_number;
    node->prev = stack->top;
    node->type = type;
    stack->top = node;
    stack->size++;
}

int stack_lookup(Stack_T *stack, StackType type) {
    stack_node_t *curr;
    if (!stack) return 0;
    curr = stack->top;
    while (curr) {
        if (curr->type == type) return 1;
        curr = curr->prev;
    }
    return 0;
}

void free_stack(Stack_T *stack) {
    while (stack->size > 0) {
        free(stack_pop(stack));
    }
    free(stack);
}

/**
 * Create a string with format "node->line_number - endl"
 * @param node pointer to stack_node_t whose block ended in line number endl
 * @param endl line number where stack_node_t comment ends
 * @return a pointer to a string with format "node->line_number - endl"
 */
char *lines_to_string(stack_node_t *node, int endl) {
    char *str;
    int digit_count = 0, N = node->line_number;
    while (N > 0) {
        N = (int)N / 10;
        digit_count++;
    }
    N = endl;
    while (N > 0) {
        N = (int)N / 10;
        digit_count++;
    }
    str = malloc((digit_count + 4) * sizeof(char));
    sprintf(str, "%d - %d", node->line_number, endl);
    return str;
}
