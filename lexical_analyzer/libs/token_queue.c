/********************************************
 * File: token_queue.c                      *
 *                                          *
 * Implementation for token_queue.h         *
 ********************************************/

#include "token_queue.h"

#include <stdlib.h>
#include <string.h>


/*
 * Create a new, empty token_queue and return it.
 */
token_queue_t *init_token_queue() {
    token_queue_t *queue = (token_queue_t *)malloc(sizeof(token_queue_t));

    queue->head = NULL;
    queue->tail = NULL;

    return queue;
}

/*
 * Initialize an alpha_token_t with given values.
 * Warning: space for token_literal is dynamically taken within the function.
 * It should be freed outside of it.
 */
void initialize_token(alpha_token_t *new_token, unsigned int line_num,
                      unsigned int token_num, char *token_literal,
                      char *token_type, char *token_value,
                      char *internal_type) {
    new_token->line_num = line_num;
    new_token->token_num = token_num;
    new_token->token_literal =
        (char *)malloc((strlen(token_literal) + 1) * sizeof(char));
    strcpy(new_token->token_literal, token_literal);
    new_token->token_type = token_type;
    new_token->token_value =
        (char *)malloc((strlen(token_value) + 1) * sizeof(char));
    strcpy(new_token->token_value, token_value);
    new_token->internal_type = internal_type;
    new_token->next = NULL;
}

/*
 * Insert a token with the given attributes to the token_queue.
 */
void insert_token(token_queue_t *queue, alpha_token_t *new_token) {
    if (queue->tail) {
        queue->tail->next = new_token;
        queue->tail = new_token;
    } else {
        queue->head = new_token;
        queue->tail = new_token;
    }
}

/*
 * Delete the given token_queue.
 */
void free_queue(token_queue_t *queue) {
    alpha_token_t *iter = queue->head, *next;

    while (iter != NULL) {
        next = iter->next;
        free(iter->token_literal);
        free(iter->token_value);
        free(iter);
        iter = next;
    }

    queue->head = NULL;
    queue->tail = NULL;
    free(queue);
    return;
}

/*
 * Print the given token_queue.
 */
void print_queue(token_queue_t *queue, FILE *alpha_yyout) {
    alpha_token_t *tmp = queue->head;
    while (tmp != NULL) {
        if (!strcmp(tmp->token_type, "STRING") ||
            !strcmp(tmp->token_type, "ID"))
            fprintf(alpha_yyout, "%d:  #%d  \"%s\"  %s  \"%s\"  <-%s\n",
                    tmp->line_num, tmp->token_num, tmp->token_literal,
                    tmp->token_type, tmp->token_value, tmp->internal_type);
        else
            fprintf(alpha_yyout, "%d:  #%d  \"%s\"  %s  %s  <-%s\n",
                    tmp->line_num, tmp->token_num, tmp->token_literal,
                    tmp->token_type, tmp->token_value, tmp->internal_type);
        tmp = tmp->next;
    }
}
