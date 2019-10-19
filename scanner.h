#include <stdio.h>
#include <stdlib.h>

#define INITIAL_SIZE 20
#define REALLOC_SIZE 5

#define STRING_END "\0"

#define RET_ERR { state = ERROR; return LEXICAL_ERROR; }
#define APPEND if(append_string(&(token->string), read)){ return INTERNAL_ERROR;}


typedef struct
{
    char *str;
    unsigned int length;
    unsigned int size;
}string_t;


typedef struct
{
    string_t string;
    unsigned int type;
}token_t;

enum state{
    START,
    LIT,
    SPACE,
    INT,
    RIGHT,
    LEFT,
    PLUS,
    MINUS,
    DIVISION,
    FLOR_DIV,
    MULTI,
    END,
    LESS_E,
    MORE_E,
    LESS,
    MORE,
    IS_EQUAL,
    ASSIG,
    N_EQUAL,
    NEG,
    IDENT,
    COMMENT,
    BLOCK,
    BLOCK1,
    BLOCK2,
    BLOCK_B,
    BLOCK_B1,
    LIT_H1,
    LIT_H,
    LIT_B,
    LIT_FIN,
    FLOAT_E,
    FLOAT_N,
    FLOAT_S,
    FLOAT_D,
    FLOAT,
    COMMA,
    COLON,
    ERROR
};

enum token_type{
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_UP,
    TOKEN_DOWN,
    TOKEN_LIT,
    TOKEN_NEG,
    TOKEN_N_EQUAL,
    TOKEN_ASSIG,
    TOKEN_IS_EQUAL,
    TOKEN_MORE,
    TOKEN_MORE_E,
    TOKEN_LESS,
    TOKEN_LESS_E,
    TOKEN_END,
    TOKEN_MULTI,
    TOKEN_DIVISION,
    TOKEN_FLOR_DIV,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_LEFT,
    TOKEN_RIGHT,
    TOKEN_DEF,
    TOKEN_ELSE,
    TOKEN_IF,
    TOKEN_NONE,
    TOKEN_PASS,
    TOKEN_RETURN,
    TOKEN_WHILE,
    TOKEN_COMMA,
    TOKEN_COLON
};
