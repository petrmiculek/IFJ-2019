
#ifndef SCANNER
#define SCANNER

#include <stdio.h>
#include <stdlib.h>
#include "my_string.h"

#define STACK_CAPACITY 100
#define STACK_REALLOC 20

#define RETURN_ERR do { state = STATE_ERROR; return RET_LEXICAL_ERROR; } while(0);
#define APPEND if(append_string(&(token->string), read)){ return RET_INTERNAL_ERROR; }

// this can't go to stack.c/stack.h, as the variable it interacts with is static
void
free_static_stack();

typedef struct
{
    string_t string;
    unsigned int type;
} token_t;

/**
 * @brief Get the token object
 *
 * @param token token struct to be filled
 * @param file file to read from
 * @return unsigned int INTERNAL_ERROR if one of used functions fails, LEXICAL_ERROR if lexical sructure of
 * text is wrong, otherwise OK
 */
unsigned int
get_token(token_t *token, FILE *file);

enum state
{
    STATE_LIT,
    STATE_INT,
    STATE_FLOAT,
    STATE_FLOAT_D,
    STATE_FLOAT_E,
    STATE_FLOAT_S,
    STATE_FLOAT_Z,
    STATE_DIVISION,
    STATE_LESS,
    STATE_MORE,
    STATE_NEG,
    STATE_ASSIGN,
    STATE_IDENTIFIER,
    STATE_COMMENT,
    STATE_BLOCK1,
    STATE_BLOCK2,
    STATE_BLOCK3,
    STATE_BLOCK_B1,
    STATE_BLOCK_B2,
    STATE_BLOCK_ES1,
    STATE_LIT_H1,
    STATE_LIT_H,
    STATE_LIT_B,
    STATE_ERROR,
    STATE_START,
    STATE_EOL,
    STATE_EOL_SP,
};

typedef enum token_type
{
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_LEFT,
    TOKEN_RIGHT,
    TOKEN_LIT,
    TOKEN_IS_EQUAL,
    TOKEN_N_EQUAL,
    TOKEN_MORE,
    TOKEN_MORE_E,
    TOKEN_LESS,
    TOKEN_LESS_E,
    TOKEN_MULTI,
    TOKEN_DIVISION,
    TOKEN_FLOR_DIV,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_NONE,
    TOKEN_PASS,
    TOKEN_RETURN,
    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_SPACE,
    TOKEN_DEF,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_ASSIGN,
    TOKEN_INDENT,
    TOKEN_DEDENT,
    TOKEN_END,
    TOKEN_EOL,
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_DOC,
} token_type;

#endif
