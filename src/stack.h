#ifndef HEADER_STACK_H
#define HEADER_STACK_H
/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
            xjacko04 (Jacko Daniel)
 */

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

unsigned int
stack_push(stack_t *stack, unsigned int item);

void
stack_pop(stack_t *stack);

#endif //HEADER_STACK_H
