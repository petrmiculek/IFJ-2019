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

typedef enum rules
{
    R_I,
    R_PLUS,
    R_MIN,
    R_MUL,
    R_DIV,
    R_IDIV,
    R_A,
    R_EA,
    R_L,
    R_EL,
    R_EQ,
    R_NE,
    R_BRACKETS
}rules;