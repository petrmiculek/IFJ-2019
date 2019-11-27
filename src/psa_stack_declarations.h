#ifndef HEADER_PSA_STACK_DECLARATIONS
#define HEADER_PSA_STACK_DECLARATIONS

#include "my_string.h"
#include "exp_stack.h"

#define MAX_STACK 100

typedef struct sem_t
{
    unsigned int type;
    unsigned int d_type;
    string_t sem_data;
} sem_t;

typedef struct sym_stack
{
    int top;
    sem_t atr[MAX_STACK];
} sym_stack;

typedef enum d_type
{
    INT,
    FLOAT,
    STRING
} d_type;

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
} table_symbol;

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
} rules;
#endif // HEADER_PSA_STACK_DECLARATIONS
