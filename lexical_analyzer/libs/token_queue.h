/********************************************
 * File: token_queue.h                      *
 *                                          *
 * A token_queue is designed to hold all    *
 * information needed for the output of a   *
 * lexical analyzer.                        *
 ********************************************/

#include <stdio.h>


/*
 * A struct to hold all information needed for a token.
 */
typedef struct alpha_token_t {
    unsigned int line_num;
    unsigned int token_num;
    char *token_literal;
    char *token_type;
    char *token_value;
    char *internal_type;
    struct alpha_token_t *next;
} alpha_token_t;

/*
 * A struct to hold the head and the tail of a queue
 * from alpha_token_t nodes.
 */
typedef struct token_queue {
    alpha_token_t *head;
    alpha_token_t *tail;
} token_queue_t;

/*
 * Create a new, empty token_queue and return it.
 */
token_queue_t *init_token_queue();

/*
 * Initialize an alpha_token_t with given values.
 * Warning: space for token_literal is dynamically taken within the function.
 * It should be freed outside of it.
 */
void initialize_token(alpha_token_t *new_token, unsigned int line_num,
                      unsigned int token_num, char *token_literal,
                      char *token_type, char *token_value, char *internal_type);

/*
 * Insert a token with the given attributes to the token_queue.
 */
void insert_token(token_queue_t *queue, alpha_token_t *new_token);

/*
 * Delete the given token_queue.
 */
void free_queue(token_queue_t *queue);

/*
 * Print the given token_queue.
 */
void print_queue(token_queue_t *queue, FILE *alpha_yyout);
