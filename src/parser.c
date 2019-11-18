#include "parser.h"
#include "scanner.h"
#include "err.h"
#include "token_queue.h"
#include "stack.h"
//#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#define RETURN_IF_ERR(res) do { if ((res) != RET_OK) {return (res);} } while(0);

/**
 * custom defined bool values
 *
 * so that functions can return RET_INTERNAL_ERROR and RET_LEXICAL_ERROR
 * - why can't you just use the stdbool ones?
 * true is defined as 1, which is RET_LEXICAL_ERROR, meaning they collide
 */
#ifdef true
#undef true
#endif // true

#define true 2999

#ifndef false
#define false 0
#endif // false

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

void
clear_data(data_t **data);

/**
 * Wrapper for get_token
 * @param data
 * @return
 */
void
get_next_token(data_t *data, unsigned int *res);

bool
is_expression(token_t *token);
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
    clear_data(&data);

    return RET_OK;
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
/*
    // right brace should be read from DEF_PARAM_LIST
    get_next_token(data, &res);
    RETURN_IF_ERR(res);

    if (data->token->type != TOKEN_RIGHT)
        return false;

 */

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
    //STATEMENT -> pass eol
    //STATEMENT -> IF_CLAUSE
    //STATEMENT -> WHILE_CLAUSE
    //STATEMENT -> RETURN_STATEMENT

    unsigned int read_result = 0;

    get_next_token(data, &read_result);
    RETURN_IF_ERR(read_result);

    if (data->token->type == TOKEN_PASS)
    {
        get_next_token(data, &read_result);
        RETURN_IF_ERR(read_result);

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
        return true;
    }
    else if (if_clause(data))
    {
        return true;
    }
    else if (while_clause(data))
    {
        return true;
    }
    else
    {
        if (data->token->type == TOKEN_IDENTIFIER)
        {
            // RHS
            // non LL1 decision:
            // id = RHS eol
            // RHS eol ( something like: id * id + 4 eol )

            q_enqueue(data->token, data->token_queue); // identifier set aside
            data->use_queue_for_read = false;

            get_next_token(data, &read_result);
            RETURN_IF_ERR(read_result);

            if (data->token->type == TOKEN_ASSIGN)
            {
                // STATEMENT -> id = ASSIGN_RHS eol

                if (assign_rhs(data))
                {
                    get_next_token(data, &read_result);
                    RETURN_IF_ERR(read_result);

                    if (data->token->type == TOKEN_EOL)
                    {
                        // assign-statement complete, save info from tokens
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }

            }
            else
            {
                // STATEMENT -> ASSIGN_RHS eol

                q_enqueue(data->token, data->token_queue); // token past identifier is set aside
                data->use_queue_for_read = true;

                if (assign_rhs(data))
                {

                    get_next_token(data, &read_result);
                    RETURN_IF_ERR(read_result);

                    if (data->token->type == TOKEN_EOL)
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
        }
        else // data->token->type != TOKEN_IDENTIFIER
        {
            // TODO Don't forget about this
        }
    }
    return false; // TODO dead code, remove when done
}

bool
is_expression(token_t *token)
{
    return (token->type == TOKEN_PLUS
        || token->type == TOKEN_MINUS
        || token->type == TOKEN_LEFT
    );
    // TODO actually, this is not going to work
}

int
assign_rhs(data_t *data)
{
    (void) data;
    // ASSIGN_RHS -> id ( CALL_PARAM
    // ASSIGN_RHS -> EXPRESSION

    unsigned int res = 0;
    get_next_token(data, &res);
    RETURN_IF_ERR(res);

    if (data->token->type == TOKEN_IDENTIFIER)
    {
        //token_t token_tmp = *data->token;
        q_enqueue(data->token, data->token_queue);

        // call PSA
    }

    return false;
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
        // TODO how to handle eof?
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
    if (data->token_queue->first)
    {
        data->token = q_pop(data->token_queue);
        *res = RET_OK;
    }
    else
    {
        *res = get_token(data->token, data->file);
    }
}

int
init_data(data_t **data)
{
    int init_state = 0;

    if (NULL == ((*data) = malloc(sizeof(data_t))))
    {
        init_state = 1;
        goto cleanup;
    }

    // token and its contents

    if (NULL == ((*data)->token = malloc(sizeof(token_t))))
    {
        init_state = 2;
        goto cleanup;
    }

    if (RET_OK != init_string(&(*data)->token->string))
    {
        init_state = 3;
        goto cleanup;
    }

    // queue
    if (NULL == ((*data)->token_queue = q_init_queue()))
    {
        init_state = 4;
        goto cleanup;
    }


    // switch cases don't contain break statement on purpose
    // fall-through makes sure all the neccessary objects are cleaned
    cleanup:
    switch (init_state)
    {
        case 5:

            q_free_queue((*data)->token_queue);
            // falls through
        case 4:

            free_string(&(*data)->token->string);
            // falls through
        case 3:

            free((*data)->token);
            // falls through
        case 2:

            free(*data);
            *data = NULL;
            // falls through
        case 1:
            /* first alloc failed, nothing to free */
            return false;
        case 0:
            /* nothing failed -> OK */
        default:

            printf("%s:%u:init_state: invalid value\n", __func__, __LINE__);
            break;
    }

    // Feature ideas:

    //      symtable
    //      flags
    //      inside function -> allow return
    //      inside while -> context aware code generation for defvar ?

    return true;
}

void
clear_data(data_t **data)
{
    q_free_queue((*data)->token_queue);
    free_string(&(*data)->token->string);
    free((*data)->token);
    free(*data);
    *data = NULL;
}

