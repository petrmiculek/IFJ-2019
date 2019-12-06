
#ifndef HEADER_EXP_STACK
#define HEADER_EXP_STACK
/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
            xjacko04 (Jacko Daniel)
            xsetin00 (Setinský Jiří)
            xsisma01 (Šišma Vojtěch)
 */

#include <stdio.h>
#include <stdlib.h>
// #include "psa.h"
#include "my_string.h"

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
    STRING,
    UNDEFINED
} d_type;

unsigned int
init(sym_stack *Stack);

void
stack_expr_pop(sym_stack *Stack);

unsigned int
stack_expr_push(sym_stack *Stack, sem_t sym);

sem_t
get_term(sym_stack *Stack);

#endif // HEADER_EXP_STACK
