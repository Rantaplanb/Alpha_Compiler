%{

#include "../libs/token_queue.h"
#include "../libs/comment_stack.h"
#include <assert.h>
#define YY_DECL int alpha_yylex (alpha_token_t* yylval)


unsigned int token_counter = 0;
token_queue_t *tokens;
comment_stack_t *stack;
enum
{
    DEFAULT_STATE = 0,
    COMMENT_BLOCK = 1,
    LINE_COMMENT = 2
} state = DEFAULT_STATE;

int str_format(char **res, const char *string);

%}


/* Flex parameters and options */
%option yylineno
%option header-file="./scanner.h"
%option prefix="alpha_yy"
%option noyywrap





/*----------------*/
/* Keyword macros */
/*----------------*/
IF                          "if"
ELSE                        "else"
WHILE                       "while"
FOR                         "for"
FUNCTION                    "function"
RETURN                      "return"
BREAK                       "break"
CONTINUE                    "continue"
AND                         "and"
NOT                         "not"
OR                          "or"
LOCAL                       "local"
TRUE                        "true"
FALSE                       "false"
NIL                         "nil"


/*-----------------*/
/* Operator macros */
/*-----------------*/
ASSIGNMENT                  "="
ADDITION                    "+"
SUBTRACTION                 "-"
MULTIPLICATION              "*"
DIVISION                    "/"
MODULO                      "%"
EQUALITY                    "=="
INEQUALITY                  "!="
INCREMENT                   "++"
DECREMENT                   "--"
GREATER_THAN                ">"
LESS_THAN                   "<"
GREATER_OR_EQUAL            ">="
LESS_OR_EQUAL               "<="


/*---------------------*/
/* Regular Expressions */
/*---------------------*/
ID                          [a-zA-Z][a-zA-Z_0-9]*
INTEGER                     [0-9]+
REAL                        [0-9]*\.[0-9]+
DELIMITER                   [\r\n \t\v\r]
STRING                      \"(\\.|[^"\\])*\"
INCOMPLETE_STRING           \"
UNKNOWN_CHAR                [!@#$^&~`'|?\\]

/*-------------*/
/* Punctuation */
/*-------------*/
LEFT_CURLY_BRACKET          "{"
RIGHT_CURLY_BRACKET         "}"
LEFT_SQUARE_BRACKET         "["
RIGHT_SQUARE_BRACKET        "]"
LEFT_PARENTHESES            "("
RIGHT_PARENTHESES           ")"
SEMICOLON                   ";"
COMMA                       ","
COLON                       ":"
DOUBLE_COLON                "::"
DOT                         "."
DOUBLE_DOT                  ".."
LINE_COMMENT                "//"
BLOCK_COMMENT_OPEN          "/*"
BLOCK_COMMENT_CLOSE         "*/"



/* Flex rules */
%%
{IF}                        {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "KEYWORD", "IF", "enumerated");
                                    return 0;
                                }
                            }
{ELSE}                      {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "KEYWORD", "ELSE", "enumerated");
                                    return 0;
                                }
                            }
{WHILE}                     {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "KEYWORD", "WHILE", "enumerated");
                                    return 0;
                                }
                            }
{FOR}                       {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "KEYWORD", "FOR", "enumerated");
                                    return 0;
                                }
                            }
{FUNCTION}                  {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "KEYWORD", "FUNCTION", "enumerated");
                                    return 0;
                                }
                            }
{RETURN}                    {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "KEYWORD", "RETURN", "enumerated");
                                    return 0;
                                }
                            }
{BREAK}                     {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "KEYWORD", "BREAK", "enumerated");
                                    return 0;
                                }
                            }
{CONTINUE}                  {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "KEYWORD", "CONTINUE", "enumerated");
                                    return 0;
                                }
                            }
{AND}                       {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "KEYWORD", "AND", "enumerated");
                                    return 0;
                                }
                            }
{NOT}                       {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "KEYWORD", "NOT", "enumerated");
                                    return 0;
                                }
                            }
{OR}                        {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "KEYWORD", "OR", "enumerated");
                                    return 0;
                                }
                            }
{LOCAL}                     {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "KEYWORD", "LOCAL", "enumerated");
                                    return 0;
                                }
                            }
{TRUE}                      {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "KEYWORD", "TRUE", "enumerated");
                                    return 0;
                                }
                            }
{FALSE}                     {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "KEYWORD", "FALSE", "enumerated");
                                    return 0;
                                }
                            }
{NIL}                       {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "KEYWORD", "NIL", "enumerated");
                                    return 0;
                                }
                            }


{ASSIGNMENT}                {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "OPERATOR", "ASSIGNMENT", "enumerated");
                                    return 0;
                                }
                            }
{ADDITION}                 {    if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "OPERATOR", "ADDITION", "enumerated");
                                    return 0;
                                }
                            }
{SUBTRACTION}               {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "OPERATOR", "SUBTRACTION", "enumerated");
                                    return 0;
                                }
                            }
{MULTIPLICATION}            {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "OPERATOR", "MULTIPLICATION", "enumerated");
                                    return 0;
                                }
                            }
{DIVISION}                  {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "OPERATOR", "DIVISION", "enumerated");
                                    return 0;
                                }
                            }
{MODULO}                    {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "OPERATOR", "MODULO", "enumerated");
                                    return 0;
                                }
                            }
{EQUALITY}                  {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "OPERATOR", "EQUALITY", "enumerated");
                                    return 0;
                                }
                            }
{INEQUALITY}                {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "OPERATOR", "INEQUALITY", "enumerated");
                                    return 0;
                                }
                            }
{INCREMENT}                 {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "OPERATOR", "INCREMENT", "enumerated");
                                    return 0;
                                }
                            }
{DECREMENT}                 {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "OPERATOR", "DECREMENT", "enumerated");
                                    return 0;
                                }
                            }
{GREATER_THAN}              {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "OPERATOR", "GREATER_THAN", "enumerated");
                                    return 0;
                                }
                            }
{LESS_THAN}                 { if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "OPERATOR", "LESS_THAN", "enumerated");
                                    return 0;
                                    }
                              }
{GREATER_OR_EQUAL}          { if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "OPERATOR", "GREATER_OR_EQUAL", "enumerated");
                                    return 0;
                                    }
                              }
{LESS_OR_EQUAL}             { if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "OPERATOR", "LESS_OR_EQUAL", "enumerated");
                                    return 0;
                                    }
                             }


{ID}                        {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter, yytext, "ID", yytext, "char*");
                                    return 0;
                                }
                            }
{INTEGER}                   {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "INTEGER", yytext, "int");
                                    return 0;
                                }
                            }
{REAL}                      {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter, yytext, "REAL", yytext, "real");
                                    return 0;
                                }
                            }
{DELIMITER}                 { if(yytext[0] == '\n')
                                state = state == LINE_COMMENT? DEFAULT_STATE:state;
                            }


{LEFT_CURLY_BRACKET}        {    if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "PUNCTUATION", "LEFT_CURLY_BRACKET", "enumerated");
                                    return 0;
                                }
                            }
{RIGHT_CURLY_BRACKET}       {    if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "PUNCTUATION", "RIGHT_CURLY_BRACKET", "enumerated");
                                return 0;
                                }
                            }
{LEFT_SQUARE_BRACKET}       {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "PUNCTUATION", "LEFT_SQUARE_BRACKET", "enumerated");
                                    return 0;
                                }
                            }
{RIGHT_SQUARE_BRACKET}      {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "PUNCTUATION", "RIGHT_SQUARE_BRACKET", "enumerated");
                                    return 0;
                                }
                            }
{LEFT_PARENTHESES}          {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "PUNCTUATION", "LEFT_PARENTHESES", "enumerated");
                                    return 0;
                                }
                            }
{RIGHT_PARENTHESES}         {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "PUNCTUATION", "RIGHT_PARENTHESES", "enumerated");
                                    return 0;
                                }
                            }
{SEMICOLON}                 {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "PUNCTUATION", "SEMICOLON", "enumerated");
                                    return 0;
                                }
                            }
{COMMA}                     {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "PUNCTUATION", "COMMA", "enumerated");
                                    return 0;
                                }
                            }
{COLON}                     {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "PUNCTUATION", "COLON", "enumerated");
                                    return 0;
                                }
                            }
{DOUBLE_COLON}              {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "PUNCTUATION", "DOUBLE_COLON", "enumerated");
                                    return 0;
                                }
                            }
{DOT}                       {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "PUNCTUATION", "DOT", "enumerated");
                                    return 0;
                                }
                            }
{DOUBLE_DOT}                {   if(state == DEFAULT_STATE){
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext, "PUNCTUATION", "DOUBLE_DOT", "enumerated");
                                    return 0;
                                }
                            }
{BLOCK_COMMENT_OPEN}        {   if(state != LINE_COMMENT){
                                    insert_stack(stack, yylineno);
                                    state = COMMENT_BLOCK;
                                }
                            }

{BLOCK_COMMENT_CLOSE}       {   if(state != LINE_COMMENT){
                                    stack_node_t *popped = stack_pop(stack);
                                    if(popped == NULL){
                                        alpha_token_t *tmp = malloc(sizeof(alpha_token_t));
                                        initialize_token(tmp, yylineno, ++token_counter,
                                         "*", "OPERATOR", "MULTIPLICATION", "enumerated");
                                        insert_token(tokens, tmp);
                                        initialize_token(yylval, yylineno, ++token_counter,
                                         "/", "OPERATOR", "DIVISION", "enumerated");
                                        return 0;
                                    }
                                    char *token_literal = lines_to_string(popped, yylineno);
                                    if(stack->size == 0){
                                        state = DEFAULT_STATE;
                                        initialize_token(yylval, popped->line_number, ++token_counter,
                                                         token_literal, "COMMENT", "BLOCK_COMMENT", "enumerated");
                                    }else{
                                        initialize_token(yylval, popped->line_number, ++token_counter,
                                                         token_literal, "COMMENT", "NESTED", "enumerated");
                                    }
                                    free(token_literal);
                                    free(popped);
                                    return 0;
                                }
                            }

{STRING}                    {   char *string;
                                if(state == DEFAULT_STATE){
                                    if(!str_format(&string, yytext))
                                        return 1;
                                    initialize_token(yylval, yylineno, ++token_counter,
                                                     string, "STRING", string, "char*");
                                    free(string);
                                    return 0;
                                }
                            }

{INCOMPLETE_STRING}         {   if(yytext[0] == '\"'){
                                    fprintf(stderr, "String opened in line %d, but was never closed.\n", yylineno);
                                    return 1;
                                }
                                return 0;
                            }

<<EOF>>                     {   if(yylval)
                                    free(yylval);
                                if(state == COMMENT_BLOCK){
                                    stack_node_t *popped = stack_pop(stack);
                                    assert(popped);
                                    fprintf(stderr, "Comment opened in line %d, but was never closed.\n", popped->line_number);
                                    free(popped);
                                    return 1;
                                }

                                return 2;
                            }

{LINE_COMMENT}              {   if(state == DEFAULT_STATE){
                                    state = LINE_COMMENT;
                                    initialize_token(yylval, yylineno, ++token_counter,
                                     yytext + 2, "COMMENT", "LINE_COMMENT", "enumerated");
                                    return 0;
                                } else if(state == COMMENT_BLOCK){
                                    unput(yytext[1]);
                                }
                            }

{UNKNOWN_CHAR}              {   if(state != COMMENT_BLOCK && state != LINE_COMMENT){
                                    fprintf(stderr, "Unknown character \'%s\' found in line %d.\n", yytext, yylineno);
                                    return 1;
                                }
                            }
%%



/*
 * Format strings.
 * Return 0 if input string contains non-valid escape characters,
 * 1 otherwise.
 */
int str_format(char **res, const char *string) {
    *res = (char *)malloc(sizeof(char)*strlen(string) - 1);
    int i, j, N = strlen(string);
    for(i = 1, j = 0; i < N - 1; i++, j++) {
        if(string[i] == '\\') {
            switch(string[i + 1]) {
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
                    fprintf(stderr, "In line %d, string contained an invalid escape character: \\%c\n",
                            yylineno, string[i + 1]);
                    return 0;
            }
        } else {
            (*res)[j] = string[i];
        }
    }

    (*res)[j] = '\0';

    return 1;
}


int main(int argc, char** argv) {
    if(argc > 3) {
        fprintf(stderr, "Too many input arguments\n");
        return 1;
    }
    if(argc > 1) {
        if(!(alpha_yyin = fopen(argv[1], "r"))){
            fprintf(stderr, "Cannot read file: %s\n", argv[1]);
            return 1;
        }
    } else
        alpha_yyin = stdin;

    stack = init_comment_stack();
    tokens = init_token_queue();

    while(1){
        alpha_token_t *curr = (alpha_token_t *) malloc(sizeof(alpha_token_t));
        int x = alpha_yylex(curr);
        if(x == 1){
            free_stack(stack);
            free_queue(tokens);
            return 0;
        } else if(x == 2) {
            break;
        }
        insert_token(tokens, curr);
    }

    if(argc > 1)
        fclose(alpha_yyin);

    if(argc == 3) {
        FILE *out_file;
        out_file = fopen(argv[2], "w");
        print_queue(tokens, out_file);
        fclose(out_file);
    } else
        print_queue(tokens, stdout);

    free_stack(stack);
    free_queue(tokens);

    return 0;
}

