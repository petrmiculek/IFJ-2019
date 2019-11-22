#ifndef IFJ_2019__PARSER_H
#define IFJ_2019__PARSER_H
#include <stdio.h>
#include "scanner.h"
#include "token_queue.h"

int
parse(FILE *file);

struct _struct_data_t
{
    FILE *file;
    token_queue_t *token_queue;
    token_t *token;
    int use_queue_for_read;

    // Feature ideas:

    //      symtable
    //      flags
    //      inside function -> allow return
    //      inside while -> context aware code generation for defvar ?


};
typedef struct _struct_data_t data_t;

int
function_def(data_t *data);

int
statement(data_t *data);

int
assign_rhs(data_t *data);

int
statement_list_nonempty(data_t *data);

int
statement_global(data_t *data);

int
statement_list(data_t *data);

int
if_clause(data_t *data);

int
while_clause(data_t *data);

int
def_param_list_next(data_t *data);

int
def_param_list(data_t *data);

int
call_param_list(data_t *data);

int
call_elem(data_t *data);

int
call_param_list_next(data_t *data);

int
return_statement(data_t *data);

int
return_expression(data_t *data);

int
expression(data_t *data);

int
init_data(data_t **data);

void
clear_data(data_t **data);

/**
 * Wrapper for get_token
 * @param data
 * @return
 */
void
get_next_token(data_t *data, int *res);

#endif //IFJ_2019__PARSER_H
