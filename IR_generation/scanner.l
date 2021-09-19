%{

#include "libs/globals.h"
#include "libs/stack.h"
#include "libs/symtable.h"
#include "parser.h"
#include <assert.h>

static unsigned int indexing = 0;
static unsigned int token_counter = 0;
unsigned int current_scope = 0;
Stack_T *comment_stack;
Stack_T *block_stack;
SymTable_T symbol_table;
FILE *scopes_file;

enum {
    DEFAULT_STATE = 0,
    COMMENT_BLOCK = 1,
    LINE_COMMENT = 2
} state = DEFAULT_STATE;

int str_format(char **res, const char *string);
%}

/* Flex parameters and options */
%option yylineno
%option header-file="scanner.h"
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
{IF}                        { 
                                if(state == DEFAULT_STATE)
                                    return IF;
                            }
{ELSE}                      { 
                                if(state == DEFAULT_STATE)
                                    return ELSE;
                            }
{WHILE}                     { 
                                if(state == DEFAULT_STATE)
                                    return WHILE;
                            }
{FOR}                       { 
                                if(state == DEFAULT_STATE)
                                    return FOR;
                            }
{FUNCTION}                  { 
                                if(state == DEFAULT_STATE)
                                    return FUNCTION;
                            }
{RETURN}                    { 
                                if(state == DEFAULT_STATE)
                                    return RETURN;
                            }
{BREAK}                     { 
                                if(state == DEFAULT_STATE)
                                    return BREAK;
                            }
{CONTINUE}                  { 
                                if(state == DEFAULT_STATE)
                                    return CONTINUE;
                            }
{AND}                       { 
                                if(state == DEFAULT_STATE)
                                    return AND;
                            }
{NOT}                       { 
                                if(state == DEFAULT_STATE)
                                    return NOT;
                            }
{OR}                        { 
                                if(state == DEFAULT_STATE)
                                    return OR;
                            }
{LOCAL}                     { 
                                if(state == DEFAULT_STATE)
                                    return LOCAL;
                            }
{TRUE}                      { 
                                if(state == DEFAULT_STATE)
                                    return TRUE;
                            }
{FALSE}                     { 
                                if(state == DEFAULT_STATE)
                                    return FALSE;
                            }
{NIL}                       { 
                                if(state == DEFAULT_STATE)
                                    return NIL;
                            }
{ASSIGNMENT}                { 
                                if(state == DEFAULT_STATE)
                                    return ASSIGNMENT;
                            }
{ADDITION}                  { 
                                if(state == DEFAULT_STATE)
                                    return ADDITION;
                            }
{SUBTRACTION}               { 
                                if(state == DEFAULT_STATE)
                                    return SUBTRACTION;
                            }
{MULTIPLICATION}            { 
                                if(state == DEFAULT_STATE)
                                    return MULTIPLICATION;
                            }
{DIVISION}                  { 
                                if(state == DEFAULT_STATE)
                                    return DIVISION;
                            }
{MODULO}                    { 
                                if(state == DEFAULT_STATE)
                                    return MODULO;
                            }
{EQUALITY}                  { 
                                if(state == DEFAULT_STATE)
                                    return EQUALITY;
                            }
{INEQUALITY}                { 
                                if(state == DEFAULT_STATE)
                                    return INEQUALITY;
                            }
{INCREMENT}                 { 
                                if(state == DEFAULT_STATE)
                                    return INCREMENT;
                            }
{DECREMENT}                 { 
                                if(state == DEFAULT_STATE)
                                    return DECREMENT;
                            }
{GREATER_THAN}              { 
                                if(state == DEFAULT_STATE)
                                    return GREATER_THAN;
                            }
{LESS_THAN}                 { 
                                if(state == DEFAULT_STATE)
                                    return LESS_THAN;
                            }
{GREATER_OR_EQUAL}          { 
                                if(state == DEFAULT_STATE)
                                    return GREATER_OR_EQUAL;
                            }
{LESS_OR_EQUAL}             { 
                                if(state == DEFAULT_STATE)
                                    return LESS_OR_EQUAL;
                            }
{DELIMITER}                 { 
                                if(yytext[0] == '\n')
                                    state = state == LINE_COMMENT? DEFAULT_STATE:state;
                            }
{ID}                        { 
                                if(state == DEFAULT_STATE){
                                    yylval.string = strdup(yytext);
                                    return ID;
                                }
                            }
{INTEGER}                   { 
                                if(state == DEFAULT_STATE){
                                    yylval.integer = atoi(yytext);
                                    return INTEGER;
                                }
                            }
{REAL}                      { 
                                if(state == DEFAULT_STATE){
                                    yylval.real = atof(yytext);
                                    return REAL;
                                }
                            }
{LEFT_CURLY_BRACKET}        { 
                                if(state == DEFAULT_STATE)
                                    return LEFT_CURLY_BRACKET;
                            }
{RIGHT_CURLY_BRACKET}       { 
                                if(state == DEFAULT_STATE)
                                    return RIGHT_CURLY_BRACKET;
                            }
{LEFT_SQUARE_BRACKET}       { 
                                if(state == DEFAULT_STATE){
                                    indexing++;
                                    return LEFT_SQUARE_BRACKET;
                                }
                            }
{RIGHT_SQUARE_BRACKET}      { 
                                if(state == DEFAULT_STATE){
                                    indexing--;
                                    return RIGHT_SQUARE_BRACKET;
                                }
                            }
{LEFT_PARENTHESES}          { 
                                if(state == DEFAULT_STATE)
                                    return LEFT_PARENTHESES;
                            }
{RIGHT_PARENTHESES}         { 
                                if(state == DEFAULT_STATE)
                                    return RIGHT_PARENTHESES;
                            }
{SEMICOLON}                 { 
                                if(state == DEFAULT_STATE)
                                    return SEMICOLON;
                            }
{COMMA}                     { 
                                if(state == DEFAULT_STATE)
                                    return COMMA;
                            }
{COLON}                     { 
                                if(state == DEFAULT_STATE)
                                    return COLON;
                            }
{DOUBLE_COLON}              { 
                                if(state == DEFAULT_STATE)
                                    return DOUBLE_COLON;
                            }
{DOT}                       { 
                                if(state == DEFAULT_STATE)
                                    return DOT;
                            }
{DOUBLE_DOT}                { 
                                if(state == DEFAULT_STATE)
                                    return DOUBLE_DOT;
                            }
{BLOCK_COMMENT_OPEN}        {   
                                if(state != LINE_COMMENT){
                                    insert_stack(comment_stack, yylineno, COMMENT);
                                    state = COMMENT_BLOCK;
                                }
                            }
{BLOCK_COMMENT_CLOSE}       {   
                                if(state != LINE_COMMENT){
                                    stack_node_t *popped = stack_pop(comment_stack);
                                    if(popped == NULL){
                                        return ERROR;
                                    }

                                    if(comment_stack->size == 0){
                                        state = DEFAULT_STATE;
                                    }
                                    free(popped);
                                }
                            }
{STRING}                    {   
                                char *string;
                                if(state == DEFAULT_STATE){
                                    if(!str_format(&string, yytext))
                                        return ERROR;
                                    yylval.string = strdup(string);
                                    free(string);
                                    return STRING;
                                }
                            }
{INCOMPLETE_STRING}         {   
                                fprintf(stderr, "String opened in line %d, but was never closed.\n", yylineno);
                                return ERROR;
                            }
{LINE_COMMENT}              {   
                                if(state == DEFAULT_STATE){
                                    state = LINE_COMMENT;

                                } else if(state == COMMENT_BLOCK){
                                    unput(yytext[1]);
                                }
                            }
{UNKNOWN_CHAR}              {   
                                if(state != COMMENT_BLOCK && state != LINE_COMMENT){
                                    fprintf(stderr, "Unknown character \'%s\' found in line %d.\n", yytext, yylineno);
                                    return ERROR;
                                }
                            }
%%



/*
 * Format strings.
 * Return 0 if input string contains non-valid escape characters,
 * 1 otherwise.
 */
int str_format(char **res, const char *string) {
    *res = (char *)malloc(sizeof(char) * strlen(string) - 1);
    int i, j, N = strlen(string);
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


void print_elem(const char *pcKey, SymbolTableEntry *pvValue, void *pvExtra) {
    if (pvValue->type == USERFUNC_SYMBOL || pvValue->type == LIBFUNC_SYMBOL)
        fprintf(pvExtra, "Function name: %s scope: %d line: %d\n", pcKey,
                pvValue->scope, pvValue->line);
    else
        fprintf(pvExtra, "Variable name: %s scope: %d line: %d\n", pcKey,
                pvValue->scope, pvValue->line);
}


size_t min(size_t a, size_t b) {
    return b < a ? b : a;
}

