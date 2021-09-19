#ifndef __COMMENT_STACK_C__
#define __COMMENT_STACK_C__

#include "comment_stack.h"

#include <stdio.h>
#include <stdlib.h>

#endif


comment_stack_t *init_comment_stack() {
    comment_stack_t *res = malloc(sizeof(comment_stack_t));

    res->size = 0;
    res->top = NULL;
    return res;
}

stack_node_t *stack_pop(comment_stack_t *stack) {
    if (stack->size == 0) {
        return NULL;
    }
    stack_node_t *res = stack->top;
    stack->size--;
    stack->top = res->prev;
    res->prev = NULL;
    return res;
}

void insert_stack(comment_stack_t *stack, int line_number) {
    stack_node_t *node = (stack_node_t *)malloc(sizeof(stack_node_t));
    node->line_number = line_number;
    node->prev = stack->top;
    stack->top = node;
    stack->size++;
}

void free_stack(comment_stack_t *stack) {
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
