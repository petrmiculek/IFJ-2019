//
// Created by petrmiculek on 15.11.19.
//

#ifndef IFJ_2019__PARSER_H
#define IFJ_2019__PARSER_H
#include <stdio.h>
#include "scanner.h"

unsigned int
parse(FILE *file);

typedef struct
{
    FILE *file;
    token_t *token;
    // stack_t stack;
    // symtable
    // flags (in fun, if, while)

} data_t;

#endif //IFJ_2019__PARSER_H
