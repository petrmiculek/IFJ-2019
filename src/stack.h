//
// Created by petrmiculek on 17.11.19.
//

#ifndef STACK_H
#define STACK_H

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
push(stack_t *stack, unsigned int item);

void
pop(stack_t *stack);

#endif //STACK_H
