#include "scanner.h"

unsigned int get_symbol(token_t *token);

#define EXP 42
#define SHIFT 21


typedef enum
{
    PLUS,
    MIN,
    MUL,
    DIV,
    IDIV,
    A,
    EA,
    L,
    EL,
    EQ,
    NE,
    OP_INT,
    OP_FLOAT,
    OP_STR,
    OP_ID,
    DOLAR,
    L_BRAC,
    R_BRAC
}table_symbol;