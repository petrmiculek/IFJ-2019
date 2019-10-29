#include <stdlib.h>
#include <stdio.h>

#define STACK_CAPACITY 100
#define STACK_REALLOC 20

#define RET_ERR do { state = STATE_ERROR; return LEXICAL_ERROR; } while(0);
#define APPEND if(append_string(&(token->string), read)){ return INTERNAL_ERROR;}

typedef struct
{
    unsigned int top;
    unsigned int capacity;
    unsigned int *array;
} stack_t;

typedef struct
{
    char *str;
    unsigned int length;
    unsigned int size;
} string_t;

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
get_token(token_t *token, FILE *file, stack_t *stack);

unsigned int
initStack(stack_t *stack);

void
free_stack(stack_t *stack);

unsigned int
push(stack_t *stack, unsigned int item);

void
pull(stack_t *stack);

enum state
{
    STATE_START,
    STATE_LIT,
    STATE_SPACE,
    STATE_INT,
    STATE_RIGHT,
    STATE_LEFT,
    STATE_PLUS,
    STATE_MINUS,
    STATE_DIVISION,
    STATE_FLOR_DIV,
    STATE_MULTI,
    STATE_END,
    STATE_LESS_E,
    STATE_MORE_E,
    STATE_LESS,
    STATE_MORE,
    STATE_IS_EQUAL,
    STATE_ASSIGN,
    STATE_N_EQUAL,
    STATE_NEG,
    STATE_IDENT,
    STATE_COMMENT,
    STATE_BLOCK,
    STATE_BLOCK1,
    STATE_BLOCK2,
    STATE_BLOCK_B,
    STATE_BLOCK_B1,
    STATE_LIT_H1,
    STATE_LIT_H,
    STATE_LIT_B,
    STATE_LIT_FIN,
    STATE_FLOAT_D,
    STATE_FLOAT_E,
    STATE_FLOAT_S,
    STATE_FLOAT_Z,
    STATE_FLOAT,
    STATE_COMMA,
    STATE_COLON,
    STATE_ERROR,
    STATE_EOL,
    STATE_EOL_SP,
    STATE_INDENT,
};

typedef enum token_type
{
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
    TOKEN_EOF,
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
    TOKEN_COLON,
    TOKEN_EOL,
    TOKEN_INDENT,
    TOKEN_DEDENT,
    TOKEN_END,
    TOKEN_SPACE,
    TOKEN_IDENT,
} token_type;
