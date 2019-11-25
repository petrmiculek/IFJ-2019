#ifndef HEADER_STACK_H
#define HEADER_STACK_H

#include <stdio.h>
#include <stdlib.h>
#include "my_string.h"

typedef struct
{
    unsigned int top;
    unsigned int capacity;
    unsigned int *array;
} stack_t;

unsigned int
initStack(stack_t *stack);

stack_t *
init_stack();

/*
void
free_stack(stack_t *stack);

// replaced by free_static_stack
*/

unsigned int
stack_push(stack_t *stack, unsigned int item);

void
stack_pop(stack_t *stack);

#endif //HEADER_STACK_H
