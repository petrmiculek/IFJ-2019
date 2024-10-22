/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
            xjacko04 (Jacko Daniel)
 */
#include <string.h>
#include <stdlib.h>
#include "err.h"
#include "scanner.h"
#include "stack.h"

unsigned int
initStack(stack_t *stack)
{
    stack->capacity = STACK_CAPACITY;
    stack->top = 0;
    if ((stack->array = (unsigned int *) calloc(STACK_CAPACITY * sizeof(unsigned int *), 1)) == NULL)
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
stack_push(stack_t *stack, unsigned int item)
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
stack_pop(stack_t *stack)
{
    if (stack->top != 0)
        stack->top--;
}
