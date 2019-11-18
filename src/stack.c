//
// Created by petrmiculek on 17.11.19.
//

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "my_string.h"
#include "err.h"
#include "scanner.h"
#include "stack.h"
unsigned int
initStack(stack_t *stack)
{
    stack->capacity = STACK_CAPACITY;
    stack->top = 0;
    if ((stack->array = (unsigned int *) malloc(STACK_CAPACITY * sizeof(unsigned int *))) == NULL)
    {
        return RET_INTERNAL_ERROR;
    }
    stack->array[stack->top] = 0;
    return RET_OK;
}
stack_t *
init_stack()
{
    stack_t *stack;
    if (NULL != (stack = malloc(sizeof(stack_t))))
    {
        if (initStack(stack) != RET_OK)
        {
            free(stack);
            stack = NULL;
        }
    }
    return stack;
}

unsigned int
push(stack_t *stack, unsigned int item)
{
    if (stack->capacity - 1 == stack->top)
    {
        if ((stack->array = (unsigned int *) realloc(stack->array, sizeof(unsigned int) * STACK_REALLOC)) == NULL)
        {
            return RET_INTERNAL_ERROR;
        }
    }

    stack->array[++stack->top] = item;
    return RET_OK;
}
void
pop(stack_t *stack)
{
    if (stack->top != 0)
        stack->top--;
}
