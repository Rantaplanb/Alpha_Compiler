#include "custom_scanner.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNKNOWN_TOKEN -1
#define MAX_STATE 16
#define STATE(s) s
#define TOKEN_SHIFT (MAX_STATE + 1)
#define TOKEN(t) TOKEN_SHIFT + (t)
#define istoken(s) ((s) > MAX_STATE)
#define MAX_LEXEME 1024


enum token_types {
    DELIMITER,
    ID,
    INTEGER,
    REAL,
    STRING,
    LINE_COMMENT,
    BLOCK_COMMENT_OPEN,
    BLOCK_COMMENT_CLOSE,
    ASSIGNMENT,
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    MODULO,
    EQUALITY,
    INEQUALITY,
    INCREMENT,
    DECREMENT,
    GREATER_THAN,
    LESS_THAN,
    GREATER_OR_EQUAL,
    LESS_OR_EQUAL,
    LEFT_CURLY_BRACKET,
    RIGHT_CURLY_BRACKET,
    LEFT_SQUARE_BRACKET,
    RIGHT_SQUARE_BRACKET,
    LEFT_PARENTHESES,
    RIGHT_PARENTHESES,
    SEMICOLON,
    COMMA,
    COLON,
    DOUBLE_COLON,
    DOT,
    DOUBLE_DOT,
    STRING_UNFINISHED,
    END_OF_FILE,
    UNKNOWN_CHARACTER
};

typedef int (*StateFunc)(char c);

StateFunc state_functions[MAX_STATE + 1] = {
    &initial_state,    &state_function1,  &state_function2,  &state_function3,
    &state_function4,  &state_function5,  &state_function6,  &state_function7,
    &state_function8,  &state_function9,  &state_function10, &state_function11,
    &state_function12, &state_function13, &state_function14, &state_function15,
    &state_function16};

char look_ahead = '\0';
int use_look_ahead = 0;
FILE *input_file = NULL;
char lexeme[MAX_LEXEME];
unsigned int curr = 0;
unsigned int line_number = 1;
unsigned int token_count = 0;
int current_state = 0;
bool comment_block = false;
bool escape_char = false;
comment_stack_t *stack;
token_queue_t *queue;


void reset_lexeme(void) { curr = 0; }

char *get_lexeme(void) {
    lexeme[curr] = '\0';
    return lexeme;
}

char get_next_char(void) {
    if (use_look_ahead) {
        use_look_ahead = 0;
        return look_ahead;
    } else
        return fgetc(input_file);
}

void retract(char c) {
    assert(!use_look_ahead);
    use_look_ahead = 1;
    look_ahead = c;
}

void check_line(char c) {
    if (c == '\n') line_number++;
    return;
}

void extend_lexeme(char c) {
    if (curr > MAX_LEXEME - 1) {
        fprintf(stderr, "Max token length is %d.\n", MAX_LEXEME);
        exit(-1);
    }

    lexeme[curr++] = c;
}

bool is_keyword(char *str) {
    if (!strcmp(str, "if") || !strcmp(str, "else") || !strcmp(str, "while") ||
        !strcmp(str, "for") || !strcmp(str, "function") ||
        !strcmp(str, "return") || !strcmp(str, "break") ||
        !strcmp(str, "continue") || !strcmp(str, "and") ||
        !strcmp(str, "not") || !strcmp(str, "or") || !strcmp(str, "local") ||
        !strcmp(str, "true") || !strcmp(str, "false") || !strcmp(str, "nil"))
        return (true);
    else
        return (false);
}

bool is_delimiter(char c) {
    if (c == '\n') {
        line_number++;
        return true;
    }
    if (c == ' ' || c == '\r' || c == '\t' || c == '\v')
        return true;
    else
        return false;
}

bool is_unknown_character(char c) {
    if (c == '@' || c == '#' || c == '$' || c == '^' || c == '~' || c == '`' ||
        c == '\'' || c == '|' || c == '?' || c == '&')
        return true;
    else
        return false;
}

bool is_single_char_punctuation(char c) {
    if (c == '{' || c == '}' || c == '[' || c == ']' || c == '(' || c == ')' ||
        c == ';' || c == ',')
        return true;
    else
        return false;
}

int initial_state(char c) {
    if (isalpha(c)) return STATE(1);
    if (isdigit(c)) return STATE(2);
    if (is_delimiter(c)) return TOKEN(DELIMITER);
    if (c == '\"') return STATE(4);
    if (is_unknown_character(c)) return TOKEN(UNKNOWN_CHARACTER);
    if (c == ':') return STATE(6);
    if (c == '.') return STATE(7);
    if (is_single_char_punctuation(c)) {
        extend_lexeme(c);
        return state_function5(c);
    }
    if (c == '=') return STATE(8);
    if (c == '+') return STATE(9);
    if (c == '-') return STATE(10);
    if (c == '<') return STATE(11);
    if (c == '>') return STATE(12);
    if (c == '!') return STATE(13);
    if (c == '%') {
        extend_lexeme(c);
        return TOKEN(MODULO);
    }
    if (c == '*') return STATE(14);
    if (c == '/') return STATE(15);
    if (c == EOF) return TOKEN(END_OF_FILE);
    return TOKEN(UNKNOWN_CHARACTER);
}

int gettoken2() {
    current_state = 0;
    reset_lexeme();
    while (1) {
        char c = get_next_char();
        current_state = (*state_functions[current_state])(c);

        if (current_state == -1)
            return UNKNOWN_TOKEN;
        else if (istoken(current_state)) {
            return current_state - TOKEN_SHIFT;
        } else
            extend_lexeme(c);
    }
}


int state_function1(char c) {
    if (isdigit(c) || isalpha(c) || c == '_') return STATE(1);
    retract(c);
    return TOKEN(ID);
}

int state_function2(char c) {
    if (isdigit(c)) return STATE(2);
    if (c == '.') return STATE(3);
    retract(c);
    return TOKEN(INTEGER);
}

int state_function3(char c) {
    if (isdigit(c)) return STATE(3);
    retract(c);
    return TOKEN(REAL);
}

int state_function4(char c) {
    if (c == '\\') {
        escape_char = !escape_char;
        return STATE(4);
    }
    if (c == '\"' && !escape_char) {
        extend_lexeme(c);
        return TOKEN(STRING);
    } else if (c == EOF)
        return TOKEN(STRING_UNFINISHED);
    escape_char = false;
    return STATE(4);
}

int state_function5(char c) {
    switch (c) {
        case '{':
            return TOKEN(LEFT_CURLY_BRACKET);
        case '}':
            return TOKEN(RIGHT_CURLY_BRACKET);
        case '[':
            return TOKEN(LEFT_SQUARE_BRACKET);
        case ']':
            return TOKEN(RIGHT_SQUARE_BRACKET);
        case '(':
            return TOKEN(LEFT_PARENTHESES);
        case ')':
            return TOKEN(RIGHT_PARENTHESES);
        case ';':
            return TOKEN(SEMICOLON);
        case ',':
            return TOKEN(COMMA);
        default:
            assert(0);
    }
}

int state_function6(char c) {
    if (c == ':') {
        extend_lexeme(c);
        return TOKEN(DOUBLE_COLON);
    }
    retract(c);
    return TOKEN(COLON);
}

int state_function7(char c) {
    if (c == '.') {
        extend_lexeme(c);
        return TOKEN(DOUBLE_DOT);
    } else if (isdigit(c))
        return STATE(3);
    retract(c);
    return TOKEN(DOT);
}

int state_function8(char c) {
    if (c == '=') {
        extend_lexeme(c);
        return TOKEN(EQUALITY);
    }
    retract(c);
    return TOKEN(ASSIGNMENT);
}

int state_function9(char c) {
    if (c == '+') {
        extend_lexeme(c);
        return TOKEN(INCREMENT);
    }
    retract(c);
    return TOKEN(ADDITION);
}

int state_function10(char c) {
    if (c == '-') {
        extend_lexeme(c);
        return TOKEN(DECREMENT);
    }
    retract(c);
    return TOKEN(SUBTRACTION);
}

int state_function11(char c) {
    if (c == '=') {
        extend_lexeme(c);
        return TOKEN(LESS_OR_EQUAL);
    }
    retract(c);
    return TOKEN(LESS_THAN);
}

int state_function12(char c) {
    if (c == '=') {
        extend_lexeme(c);
        return TOKEN(GREATER_OR_EQUAL);
    }
    retract(c);
    return TOKEN(GREATER_THAN);
}

int state_function13(char c) {
    if (c == '=') {
        extend_lexeme(c);
        return TOKEN(INEQUALITY);
    }
    retract(c);
    return TOKEN(UNKNOWN_CHARACTER);
}

int state_function14(char c) {
    if (c == '/') return TOKEN(BLOCK_COMMENT_CLOSE);
    retract(c);
    return TOKEN(MULTIPLICATION);
}

int state_function15(char c) {
    if (c == '/') return STATE(16);
    if (c == '*') return TOKEN(BLOCK_COMMENT_OPEN);
    retract(c);
    return TOKEN(DIVISION);
}

int state_function16(char c) {
    if (feof(input_file) || c == '\n') {
        return TOKEN(LINE_COMMENT);
    }

    if (comment_block) return STATE(0);

    return STATE(16);
}


/*
 * Converts the source code contained in the input_file to lexical tokens,
 * which are then inserted to the token_queue.
 */
int tokenize() {
    int token;
    comment_block = false;
    while (1) {
        token = gettoken2();
        if (comment_block) {
            stack_node_t *tmp;
            if (token == BLOCK_COMMENT_OPEN) {
                insert_stack(stack, line_number);
            } else if (token == BLOCK_COMMENT_CLOSE) {
                tmp = stack_pop(stack);
                alpha_token_t *new_token = malloc(sizeof(alpha_token_t));
                char *token_literal = lines_to_string(tmp, line_number);

                if (stack->size == 0) {
                    initialize_token(new_token, tmp->line_number, ++token_count,
                                     token_literal, "COMMENT", "BLOCK_COMMENT",
                                     "enumerated");
                    comment_block = false;
                } else
                    initialize_token(new_token, tmp->line_number, ++token_count,
                                     token_literal, "COMMENT", "NESTED",
                                     "enumerated");

                free(token_literal);
                insert_token(queue, new_token);
                free(tmp);
            } else if (token == END_OF_FILE) {
                tmp = stack_pop(stack);
                fprintf(
                    stderr,
                    "Block comment opened in line %d, but was never closed.\n",
                    tmp->line_number);
                free(tmp);
                return -1;
            }
        } else {
            alpha_token_t *new_token = malloc(sizeof(alpha_token_t));
            char *string;
            switch (token) {
                case DELIMITER:
                    free(new_token);
                    continue;
                case ID:
                    if (is_keyword(get_lexeme())) {
                        char *string_value = strupr(get_lexeme());
                        initialize_token(new_token, line_number, ++token_count,
                                         get_lexeme(), "KEYWORD", string_value,
                                         "enumerated");
                        free(string_value);
                    } else {
                        initialize_token(new_token, line_number, ++token_count,
                                         get_lexeme(), "ID", get_lexeme(),
                                         "char*");
                    }
                    break;
                case INTEGER:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "INTEGER", get_lexeme(),
                                     "int");
                    break;
                case REAL:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "REAL", get_lexeme(),
                                     "real");
                    break;
                case STRING:
                    if (str_format(&string, get_lexeme())) {
                        initialize_token(new_token, line_number, ++token_count,
                                         string, "STRING", string, "char*");
                        free(string);
                    } else {
                        fprintf(stderr,
                                "Unknown escape character found in string that "
                                "starts in line %d.\n",
                                line_number);
                        return -1;
                    }
                    break;
                case LINE_COMMENT:
                    initialize_token(new_token, line_number, ++token_count,
                                     &(get_lexeme()[2]), "COMMENT",
                                     "LINE_COMMENT", "enumerated");
                    line_number++;
                    break;
                case END_OF_FILE:
                    free(new_token);
                    return 0;
                case MODULO:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "OPERATOR", "MODULO",
                                     "enumerated");
                    break;
                case MULTIPLICATION:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "OPERATOR", "MULTIPLICATION",
                                     "enumerated");
                    break;
                case BLOCK_COMMENT_OPEN:
                    insert_stack(stack, line_number);
                    comment_block = true;
                    free(new_token);
                    continue;
                case BLOCK_COMMENT_CLOSE:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "OPERATOR", "MULTIPLICATION",
                                     "enumerated");
                    insert_token(queue, new_token);
                    new_token = malloc(sizeof(alpha_token_t));
                    initialize_token(new_token, line_number, ++token_count, "/",
                                     "OPERATOR", "DIVISION", "enumerated");
                    break;
                case UNKNOWN_CHARACTER:
                    fprintf(stderr,
                            "Unsupported character %s parsed in line %d.\n",
                            get_lexeme(), line_number);
                    free(new_token);
                    return -1;
                case STRING_UNFINISHED:
                    fprintf(stderr,
                            "String opened in line %d, but was never closed.\n",
                            line_number);
                    free(new_token);
                    return -1;
                case LEFT_CURLY_BRACKET:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "PUNCTUATION",
                                     "LEFT_CURLY_BRACKET", "enumerated");
                    break;
                case RIGHT_CURLY_BRACKET:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "PUNCTUATION",
                                     "RIGHT_CURLY_BRACKET", "enumerated");
                    break;
                case LEFT_SQUARE_BRACKET:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "PUNCTUATION",
                                     "LEFT_SQUARE_BRACKET", "enumerated");
                    break;
                case RIGHT_SQUARE_BRACKET:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "PUNCTUATION",
                                     "RIGHT_SQUARE_BRACKET", "enumerated");
                    break;
                case LEFT_PARENTHESES:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "PUNCTUATION",
                                     "LEFT_PARENTHESES", "enumerated");
                    break;
                case RIGHT_PARENTHESES:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "PUNCTUATION",
                                     "RIGHT_PARENTHESES", "enumerated");
                    break;
                case SEMICOLON:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "PUNCTUATION", "SEMICOLON",
                                     "enumerated");
                    break;
                case COMMA:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "PUNCTUATION", "COMMA",
                                     "enumerated");
                    break;
                case COLON:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "PUNCTUATION", "COLON",
                                     "enumerated");
                    break;
                case DOUBLE_COLON:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "PUNCTUATION",
                                     "DOUBLE_COLON", "enumerated");
                    break;
                case DOT:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "PUNCTUATION", "DOT",
                                     "enumerated");
                    break;
                case DOUBLE_DOT:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "PUNCTUATION", "DOUBLE_DOT",
                                     "enumerated");
                    break;
                case ASSIGNMENT:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "OPERATOR", "ASSIGNMENT",
                                     "enumerated");
                    break;
                case ADDITION:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "OPERATOR", "ADDITION",
                                     "enumerated");
                    break;
                case SUBTRACTION:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "OPERATOR", "SUBTRACTION",
                                     "enumerated");
                    break;
                case DIVISION:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "OPERATOR", "DIVISION",
                                     "enumerated");
                    break;
                case EQUALITY:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "OPERATOR", "EQUALITY",
                                     "enumerated");
                    break;
                case INEQUALITY:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "OPERATOR", "INEQUALITY",
                                     "enumerated");
                    break;
                case INCREMENT:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "OPERATOR", "INCREMENT",
                                     "enumerated");
                    break;
                case DECREMENT:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "OPERATOR", "DECREMENT",
                                     "enumerated");
                    break;
                case GREATER_THAN:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "OPERATOR", "GREATER_THAN",
                                     "enumerated");
                    break;
                case LESS_THAN:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "OPERATOR", "LESS_THAN",
                                     "enumerated");
                    break;
                case GREATER_OR_EQUAL:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "OPERATOR",
                                     "GREATER_OR_EQUAL", "enumerated");
                    break;
                case LESS_OR_EQUAL:
                    initialize_token(new_token, line_number, ++token_count,
                                     get_lexeme(), "OPERATOR", "LESS_OR_EQUAL",
                                     "enumerated");
                    break;
                default:
                    assert(0);
            }
            insert_token(queue, new_token);
        }
    }
}


int main(int argc, char **argv) {
    if (argc > 3) {
        fprintf(stderr, "Too many input arguments\n");
        return 1;
    }
    if (argc > 1) {
        if (!(input_file = fopen(argv[1], "r"))) {
            fprintf(stderr, "Cannot read file: %s\n", argv[1]);
            return 1;
        }
    } else
        input_file = stdin;

    stack = init_comment_stack();
    queue = init_token_queue();

    if (tokenize() == 0) {
        if (argc == 3) {
            FILE *out_file;
            out_file = fopen(argv[2], "w");
            print_queue(queue, out_file);
            fclose(out_file);
        } else
            print_queue(queue, stdout);
    }

    if (argc > 1) fclose(input_file);

    free_stack(stack);
    free_queue(queue);

    return 0;
}


/*
 * Format strings.
 * Return 0 if input string contains non-valid escape characters,
 * 1 otherwise.
 */
int str_format(char **res, const char *string) {
    *res = (char *)malloc(sizeof(char) * strlen(string) - 1);
    size_t i, j, N = strlen(string);
    for (i = 1, j = 0; i < N - 1; i++, j++) {
        if (string[i] == '\\') {
            switch (string[i + 1]) {
                case 'n':
                    (*res)[j] = '\n';
                    i++;
                    break;
                case '\\':
                    (*res)[j] = '\\';
                    i++;
                    break;
                case '"':
                    (*res)[j] = '"';
                    i++;
                    break;
                case 't':
                    (*res)[j] = '\t';
                    i++;
                    break;
                case 'r':
                    (*res)[j] = '\r';
                    i++;
                    break;
                case 'v':
                    (*res)[j] = '\v';
                    i++;
                    break;
                default:
                    return 0;
            }
        } else {
            (*res)[j] = string[i];
        }
    }

    (*res)[j] = '\0';

    return 1;
}


char *strupr(char *string) {
    int size = strlen(string) + 1;
    int i = 0;
    char *res = (char *)malloc(sizeof(char) * size);

    while (string[i] != '\0') {
        res[i] = toupper(string[i]);
        i++;
    }
    res[i] = string[i];
    return res;
}
