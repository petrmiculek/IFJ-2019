#include <stdio.h>
#include <stdlib.h>
#include "psa.h"
#include "my_string.h"

#define MAX_STACK 100

typedef struct sym_stack
{
    unsigned int top;
    sem_t atr[MAX_STACK];
}sym_stack;

typedef struct sem_t
{
    unsigned int type;
    unsigned int d_type;
    string_t sem_data;
}sem_t;

typedef enum d_type
{
    INT,
    FLOAT,
    STRING
}d_type;

unsigned int init(sym_stack *Stack);

void pop(sym_stack *Stack);

unsigned int push(sym_stack *Stack, sem_t *sym);

sem_t get_term(sym_stack *Stack);