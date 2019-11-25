
#ifndef HEADER_SCANNER
#define HEADER_SCANNER

#include <stdio.h>
#include <stdlib.h>
#include "my_string.h"

#define STACK_CAPACITY 100
#define STACK_REALLOC 20

#define RETURN_ERR do { state = STATE_ERROR; return RET_LEXICAL_ERROR; } while(0);
#define APPEND(read) if(append_char_to_string(&(token->string), read)) { return RET_INTERNAL_ERROR; }
#define APPEND_HEXCODE(read)  if(convert_char_to_hexcode(read, token)) { return RET_INTERNAL_ERROR; }

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
    STATE_LIT, // 0
    STATE_INT, // 1
    STATE_FLOAT, // 2
    STATE_FLOAT_D, // 3
    STATE_FLOAT_E, // 4
    STATE_FLOAT_S, // 5
    STATE_FLOAT_Z, // 6
    STATE_DIVISION, // 7
    STATE_LESS, // 8
    STATE_MORE, // 9
    STATE_NEG, // 10
    STATE_ASSIGN, // 11
    STATE_IDENTIFIER, // 12
    STATE_COMMENT, // 13
    STATE_BLOCK1, // 14
    STATE_BLOCK2, // 15
    STATE_BLOCK3, // 16
    STATE_BLOCK_B1, // 17
    STATE_BLOCK_B2, // 18
    STATE_BLOCK_ES1, // 19
    STATE_LIT_H1, // 20
    STATE_LIT_H, // 21
    STATE_LIT_B, // 22
    STATE_ERROR, // 23
    STATE_START, // 24
    STATE_EOL, // 25
    STATE_EOL_SP, // 26
};

typedef enum token_type
{
    TOKEN_INT, // 0
    TOKEN_FLOAT, // 1
    TOKEN_LEFT, // 2
    TOKEN_RIGHT, // 3
    TOKEN_LIT, // 4
    TOKEN_IS_EQUAL, // 5
    TOKEN_N_EQUAL, // 6
    TOKEN_MORE, // 7
    TOKEN_MORE_E, // 8
    TOKEN_LESS, // 9
    TOKEN_LESS_E, // 10
    TOKEN_MULTI, // 11
    TOKEN_DIVISION, // 12
    TOKEN_FLOR_DIV, // 13
    TOKEN_MINUS, // 14
    TOKEN_PLUS, // 15
    TOKEN_NONE, // 16
    TOKEN_PASS, // 17
    TOKEN_RETURN, // 18
    TOKEN_COMMA, // 19
    TOKEN_COLON, // 20
    TOKEN_SPACE, // 21
    TOKEN_DEF, // 22
    TOKEN_IF, // 23
    TOKEN_ELSE, // 24
    TOKEN_WHILE, // 25
    TOKEN_ASSIGN, // 26
    TOKEN_INDENT, // 27
    TOKEN_DEDENT, // 28
    TOKEN_EOL, // 29
    TOKEN_EOF, // 30
    TOKEN_IDENTIFIER, // 31
    TOKEN_DOC, // 32
} token_type;

#define TOKEN_INVALID 404

#endif // HEADER_SCANNER
