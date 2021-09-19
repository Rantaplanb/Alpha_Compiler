#ifndef STACK_H
#define STACK_H
#include "symtable.h"

typedef enum { COMMENT, LOOP, FUNC_BLOCK, FUNC_OFFSET } StackType;

typedef struct stack_node {
    unsigned int line_number;
    StackType type;
    SymbolTableEntry *entry;
    struct stack_node *prev;
} stack_node_t;

typedef struct stack {
    int size;
    stack_node_t *top;

} Stack_T;


/**
 * Create new comment stack
 * @return pointer to allocated comment stack (initialized)
 */
Stack_T *Create_Empty_Stack();

/**
 * Remove stack's top element
 * @return pointer to stack_node_t
 */
stack_node_t *stack_pop(Stack_T *stack);

/**
 * Return stack's top element
 * @return pointer to stack_node_t
 */
stack_node_t *stack_top(Stack_T *stack);

/**
 * Check stack for any instances of StackType type
 * @return 1 on success
 *         0 on failure
 */
int stack_lookup(Stack_T *stack, StackType type);

/**
 * Insert key to stack
 * @param key integer with line number parameter
 */
void insert_stack(Stack_T *stack, int key, StackType type,
                  SymbolTableEntry *entry);

/**
 * Create a string with format "node->line_number - endl"
 * @param node pointer to stack_node_t whose block ended in line number endl
 * @param endl line number where stack_node_t comment ends
 * @return a pointer to a string with format "node->line_number - endl"
 */
char *lines_to_string(stack_node_t *node, int endl);

/**
 * Free allocated memory.
 */
void free_stack(Stack_T *stack);

#endif
