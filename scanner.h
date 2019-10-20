#include <stdio.h>
#include <stdlib.h>

#define INITIAL_SIZE 20
#define REALLOC_SIZE 5
#define STACK_CAPACITY 100
#define STACK_REALLOC 20

#define STRING_END "\0"

#define RET_ERR { state = ERROR; return LEXICAL_ERROR; }
#define APPEND if(append_string(&(token->string), read)){ return INTERNAL_ERROR;}

typedef struct
{ 
    unsigned int top; 
    unsigned int capacity; 
    unsigned int* array; 
}stack_t; 

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



/**
 * @brief alloc memory for string
 * 
 * @param string pointer
 * @return unsigned int INTERNAL_ERROR if malloc fails, otherwise OK
 */
unsigned int init_string(string_t *string);

/**
 * @brief add one character at end of string
 * 
 * @param string string to which will be appended
 * @param var Char which is to be appended
 * @return unsigned int INTERNAL_ERROR if realloc fails, otherwise OK
 */
unsigned int append_string(string_t *string, char var);
/**
 * @brief free
 * 
 * @param string 
 * @return unsigned int 
 */
unsigned int free_string(string_t *string);
/**
 * @brief Get the token object
 * 
 * @param token token struct to be filled 
 * @param file file to read from
 * @return unsigned int INTERNAL_ERROR if one of used functions fails, LEXICAL_ERROR if lexical sructure of 
 * text is wrong, otherwise OK
 */
unsigned int get_token(token_t *token, FILE *file);

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
    FLOAT_Z,
    FLOAT_S,
    FLOAT_D,
    FLOAT,
    COMMA,
    COLON,
    ERROR,
    EOL,
    EOL_SP,
    INDENT
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
    TOKEN_COLON,
    TOKEN_EOL
};
