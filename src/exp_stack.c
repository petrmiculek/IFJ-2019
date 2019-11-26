/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
            xjacko04 (Jacko Daniel)
            xsetin00 (Setinský Jiří)
            xsisma01 (Šišma Vojtěch)
 */
#include <stdio.h>
#include <stdlib.h>
#include "err.h"
#include "psa.h"
#include "exp_stack.h"


unsigned int init_val(sym_stack *Stack)
{
    if(Stack == NULL)
    {
        return RET_INTERNAL_ERROR;
    }
    Stack->top = -1;
    return RET_OK;
}

void pop_val(sym_stack *Stack)
{
    if(Stack->top == -1)
    {
        return;
    }
    else
    {
        Stack->top--;
    }
}

unsigned int push_val(sym_stack *Stack, sem_t atr)
{
    if(Stack->top == MAX_STACK-1)
    {
        return RET_INTERNAL_ERROR;
    }
    else
    {
        Stack->atr[Stack->top+1] = atr;
        Stack->top++;

    }
    return RET_OK;
}

unsigned int init(sym_stack *Stack)
{
    if(Stack == NULL)
    {
        return RET_INTERNAL_ERROR;
    }
    Stack->top = -1;
    return RET_OK;
}

void
stack_expr_pop(sym_stack *Stack)
{
    if (Stack->top == -1)
    {
        return;
    }
    else
    {
        Stack->top--;
    }
}

unsigned int
stack_expr_push(sym_stack *Stack, sem_t *sym)
{
    if (Stack->top == MAX_STACK - 1)
    {
        return RET_INTERNAL_ERROR;
    }
    else
    {
        Stack->atr[Stack->top + 1] = *sym;
        Stack->top++;

    }
    return RET_OK;
}

sem_t get_term(sym_stack *Stack)
{
    int i = Stack->top+1;
    sem_t tmp = Stack->atr[i];
    while (tmp.type == EXP || tmp.type == SHIFT)
    {
        i--;
        tmp = Stack->atr[i];
    }
    return Stack->atr[i];

}
