#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#include "../libs/comment_stack.h"
#include "../libs/token_queue.h"


void reset_lexeme(void);

char *get_lexeme(void);

char get_next_char(void);

void retract(char c);

void check_line(char c);

void extend_lexeme(char c);

bool is_keyword(char *str);

bool is_delimiter(char c);

bool is_unknown_character(char c);

bool is_single_char_punctuation(char c);

int initial_state(char c);

int gettoken2();

int state_function1(char c);

int state_function2(char c);

int state_function3(char c);

int state_function4(char c);

int state_function5(char c);

int state_function6(char c);

int state_function7(char c);

int state_function8(char c);

int state_function9(char c);

int state_function10(char c);

int state_function11(char c);

int state_function12(char c);

int state_function13(char c);

int state_function14(char c);

int state_function15(char c);

int state_function16(char c);

int tokenize();

int str_format(char **res, const char *string);

char *strupr(char *string);
