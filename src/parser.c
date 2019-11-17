#include "parser.h"
#include "scanner.h"
#include "err.h"
//#include <stdbool.h>
#include <stdlib.h>

#define RETURN_IF_ERR(res) do { if ((res) != RET_OK) {return (res);} } while(0);

/**
 * custom defined bool values
 * so that functions can return RET_INTERNAL_ERROR and RET_LEXICAL_ERROR
 *
 *
 */
#define true 2999
#define false 0

// TODO projit Valgrind

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
function_call(data_t *data);

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

/**
 * Wrapper for get_token
 * @param data
 * @return
 */
void
get_next_token(data_t *data, unsigned int *res);

unsigned int
parse(FILE *file)
{
    unsigned int res;

    data_t *data = NULL;

    if (!init_data(&data))
    {
        return RET_INTERNAL_ERROR;
    }

    data->file = file;
    // start syntax analysis with starting nonterminal
    res = statement_global(data);

    printf((res) ? "ok" : "err");

    free_static_stack();

    return RET_OK;
}

int
init_data(data_t **data)
{
    // idea: could use (data_t** data) and set (*data = NULL) on error

    if (NULL == ((*data) = malloc(sizeof(data_t))))
    {
        return false;
    }

    // token and its contents

    if (NULL == ((*data)->token = malloc(sizeof(token_t))))
    {
        free(*data);
        return false;
    }

    if (RET_OK != init_string(&(*data)->token->string))
    {
        free((*data)->token);
        free(*data);
        return false;
    }

    // symtable

    // flags
    // in function -> allow return
    // in while -> context aware code generation for defvar ?

    return true;
}

int
statement_list_nonempty(data_t *data)
{
    (void) data;
    // STATEMENT_LIST_NONEMPTY -> STATEMENT STATEMENT_LIST



    return 0;
}

int
statement_list(data_t *data)
{
    // STATEMENT_LIST -> STATEMENT STATEMENT_LIST
    // STATEMENT_LIST -> dedent

    unsigned int res = 0;

    get_next_token(data, &res);
    RETURN_IF_ERR(res);

    if (data->token->type == TOKEN_DEDENT)
    {
        return true;
    }
    else if (statement(data))
    {
        return statement_list(data);
    }

    // else
    return false;
}

int
function_def(data_t *data)
{
    // FUNCTION_DEF -> def id ( DEF_PARAM_LIST ) : eol indent STATEMENT_LIST_NONEMPTY
    unsigned int res = 0;

    get_next_token(data, &res);
    RETURN_IF_ERR(res);

    if (data->token->type != TOKEN_DEF)
        return false;

    get_next_token(data, &res);
    RETURN_IF_ERR(res);

    if (data->token->type != TOKEN_IDENTIFIER)
        return false;

    // identifier to symtable
    // check redefinition

    get_next_token(data, &res);
    RETURN_IF_ERR(res);

    if (data->token->type != TOKEN_LEFT)
        return false;

    if (!def_param_list(data))
        return false;

    get_next_token(data, &res);
    RETURN_IF_ERR(res);

    if (data->token->type != TOKEN_RIGHT)
        return false;

    get_next_token(data, &res);
    RETURN_IF_ERR(res);

    if (data->token->type != TOKEN_COLON)
        return false;

    get_next_token(data, &res);
    RETURN_IF_ERR(res);

    if (data->token->type != TOKEN_INDENT)
        return false;

    if (!statement_list_nonempty(data))
        return false;

    return true;
}

int
statement(data_t *data)
{
    // STATEMENT -> id = ASSIGN_RHS eol
    // STATEMENT -> ASSIGN_RHS eol
    //
    //STATEMENT -> IF_CLAUSE
    //STATEMENT -> WHILE_CLAUSE
    //STATEMENT -> pass eol
    //STATEMENT -> RETURN_STATEMENT

    unsigned int res_token_read = 0;

    get_next_token(data, &res_token_read);
    RETURN_IF_ERR(res_token_read);

    if (data->token->type == TOKEN_PASS)
    {

        get_next_token(data, &res_token_read);
        RETURN_IF_ERR(res_token_read);

        if (data->token->type == TOKEN_EOL)
        {
            return true;
        }
        else
        {
            return false;
        }

    }
    else if (return_statement(data))
    {

    }
    else if (if_clause(data))
    {

    }
    else if (while_clause(data))
    {

    }
    else
    {
        // non LL1 decision:
        // id = RHS
        // RHS
    }

    return 0;
}

int
assign_rhs(data_t *data)
{
    (void) data;
    // ASSIGN_RHS -> id ( CALL_PARAM
    // ASSIGN_RHS -> EXPRESSION
    return 0;
}

int
statement_global(data_t *data)
{
    // STATEMENT_GLOBAL -> FUNCTION_DEF STATEMENT_GLOBAL
    // STATEMENT_GLOBAL -> STATEMENT_LIST STATEMENT_GLOBAL
    // STATEMENT_GLOBAL -> eof

    unsigned int res = 0;
    get_next_token(data, &res);

    RETURN_IF_ERR(res);

    if (function_def(data))
    {
        return (statement_global(data));
    }
    else if (statement_list(data))
    {
        // TODO solve repeated branch
        return (statement_global(data));
    }
    else if (data->token->type == TOKEN_EOF)
    {
        // HANDLE END
        return true;
    }
    else
    {
        return false;
    }
}


int
if_clause(data_t *data)
{
    (void) data;
    // IF_CLAUSE -> if EXPRESSION : eol indent STATEMENT_LIST_NONEMPTY
    // else : eol indent STATEMENT_LIST_NONEMPTY
    return 0;
}

int
while_clause(data_t *data)
{
    (void) data;
    // WHILE_CLAUSE -> while EXPRESSION : eol indent STATEMENT_LIST_NONEMPTY
    return 0;
}

int
function_call(data_t *data)
{
    (void) data;
    // FUNCTION_CALL -> function_id ( CALL_PARAM_LIST )
    return 0;
}

int
def_param_list_next(data_t *data)
{
    (void) data;
    // DEF_PARAM_LIST_NEXT -> , id DEF_PARAM_LIST_NEXT
    // DEF_PARAM_LIST_NEXT -> )
    return 0;
}

int
def_param_list(data_t *data)
{
    (void) data;
    // DEF_PARAM_LIST -> id DEF_PARAM_LIST_NEXT
    // DEF_PARAM_LIST -> )
    return 0;
}

int
call_param_list(data_t *data)
{
    (void) data;
    // CALL_PARAM_LIST -> CALL_ELEM CALL_PARAM_LIST_NEXT
    // CALL_PARAM_LIST -> )
    return 0;
}

int
call_param_list_next(data_t *data)
{
    (void) data;
    // CALL_PARAM_LIST_NEXT -> , CALL_ELEM CALL_PARAM_LIST_NEXT
    // CALL_PARAM_LIST_NEXT -> )
    return 0;
}

int
call_elem(data_t *data)
{
    (void) data;
    // CALL_ELEM -> id
    // CALL_ELEM -> literal
    // CALL_ELEM -> none
    return 0;
}

int
return_statement(data_t *data)
{
    (void) data;
    // RETURN -> return RETURN_EXPRESSION
    return 0;
}

int
return_expression(data_t *data)
{
    (void) data;
    // RETURN_EXPRESSION -> eol
    // RETURN_EXPRESSION -> EXPRESSION eol
    return 0;
}

int
expression(data_t *data)
{
    (void) data;
    // mock rule:
    // EXPRESSION -> expr

    // guess we call PSA here
    return 0;
}

void
get_next_token(data_t *data, unsigned int *res)
{
    *res = get_token(data->token, data->file);

}

