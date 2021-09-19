typedef struct stack_node {
    unsigned int line_number;
    struct stack_node *prev;
} stack_node_t;

typedef struct comment_stack {
    int size;
    stack_node_t *top;

} comment_stack_t;

/**
 * Create new comment stack
 * @return pointer to allocated comment stack (initialized)
 */
comment_stack_t *init_comment_stack();
/**
 * Remove stack's top element
 * @return pointer to stack_node_t
 */
stack_node_t *stack_pop(comment_stack_t *stack);

/**
 * Insert key to comment_stack
 * @param key integer with line number parameter
 */
void insert_stack(comment_stack_t *stack, int key);

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
void free_stack(comment_stack_t *stack);
