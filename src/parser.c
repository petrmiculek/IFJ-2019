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
get_next_token(data_t *data, int *res);

int
is_expression(token_t *token);

int
parse(FILE *file)
{
    int res;

    data_t *data = NULL;

    if (init_data(&data))
    {
        return RET_INTERNAL_ERROR;
    }

    data->file = file;
    // start syntax analysis with starting nonterminal
    res = statement_global(data);

    printf((res == RET_OK) ? "ok" : "err");

    free_static_stack();
    clear_data(&data);

    return res;
}

int
statement_list_nonempty(data_t *data)
{
    (void) data;
    // STATEMENT_LIST_NONEMPTY -> STATEMENT STATEMENT_LIST



    return WARNING_NOT_IMPLEMENTED;
}

int
statement_list(data_t *data)
{
    // STATEMENT_LIST -> STATEMENT STATEMENT_LIST
    // STATEMENT_LIST -> dedent

    int res = 0;

    get_next_token(data, &res);
    RETURN_IF_ERR(res)

    if (data->token->type == TOKEN_DEDENT)
    {
        return RET_OK;
    }
    else
    {
        q_enqueue(data->token, data->token_queue);
        data->use_queue_for_read = true;

        if ((res = statement(data)) != RET_OK)
        {
            return res;
        }

        if ((res = statement_list(data)) != RET_OK)
        {
            return res;

        }

        return RET_OK; // nothing failed
    }
}

int
function_def(data_t *data)
{
    // FUNCTION_DEF -> id ( DEF_PARAM_LIST ) : eol indent STATEMENT_LIST_NONEMPTY

    int res = 0;

    // IMPORTANT: def token was read in by callee

    get_next_token(data, &res);
    RETURN_IF_ERR(res)

    if (data->token->type != TOKEN_IDENTIFIER)
        return RET_SYNTAX_ERROR;

    // identifier to symtable
    // check redefinition

    get_next_token(data, &res);
    RETURN_IF_ERR(res)

    if (data->token->type != TOKEN_LEFT)
        return RET_SYNTAX_ERROR;

    if ((res = def_param_list(data)) != RET_OK)
        return res;

    // IMPORTANT: right brace read in inside def_param_list

    get_next_token(data, &res);
    RETURN_IF_ERR(res)

    if (data->token->type != TOKEN_COLON)
        return RET_SYNTAX_ERROR;

    get_next_token(data, &res);
    RETURN_IF_ERR(res)

    if (data->token->type != TOKEN_INDENT)
        return RET_SYNTAX_ERROR;

    if ((res = statement_list_nonempty(data)) != RET_OK)
        return res;

    return RET_OK;
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

    int res = 0;

    get_next_token(data, &res);
    RETURN_IF_ERR(res);

    if (data->token->type == TOKEN_PASS)
    {
        get_next_token(data, &res);
        RETURN_IF_ERR(res);

        if (data->token->type == TOKEN_EOL)
        {
            return RET_OK;
        }
        else
        {
            return RET_SYNTAX_ERROR;
        }
    }
    else if (return_statement(data))
    {
        return RET_SYNTAX_ERROR;
    }
    else if (if_clause(data))
    {
        return RET_SYNTAX_ERROR;
    }
    else if (while_clause(data))
    {
        return RET_SYNTAX_ERROR;
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

            get_next_token(data, &res);
            RETURN_IF_ERR(res);

            if (data->token->type == TOKEN_ASSIGN)
            {
                // STATEMENT -> id = ASSIGN_RHS eol

                if ((res = assign_rhs(data)) != RET_OK)
                    return res;

                get_next_token(data, &res);
                RETURN_IF_ERR(res);

                if (data->token->type == TOKEN_EOL)
                {
                    // assign-statement complete, save info from tokens
                    return RET_OK;
                }
                else
                {
                    return RET_SYNTAX_ERROR;
                }
            }
            else
            {
                // STATEMENT -> ASSIGN_RHS eol

                q_enqueue(data->token, data->token_queue); // token past identifier is set aside
                data->use_queue_for_read = true;
                // data->use_queue_for_read turns back to false after queue is emptied

                if ((res = assign_rhs(data)) != RET_OK)
                    return res;

                get_next_token(data, &res);
                RETURN_IF_ERR(res);

                if (data->token->type == TOKEN_EOL)
                {
                    return RET_OK;
                }
                else
                {
                    return RET_SYNTAX_ERROR;
                }

            }
        }
        else // data->token->type != TOKEN_IDENTIFIER
        {

            if ((res = assign_rhs(data)) != RET_OK)
                return res;
        }
    }
    return RET_SYNTAX_ERROR; // TODO dead code, remove when done
}

int
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
    // ASSIGN_RHS -> id ( CALL_PARAM_LIST
    // ASSIGN_RHS -> EXPRESSION

    int res = 0;
    get_next_token(data, &res);
    RETURN_IF_ERR(res);

    if (data->token->type == TOKEN_IDENTIFIER)
    {
        q_enqueue(data->token, data->token_queue);

        get_next_token(data, &res);
        RETURN_IF_ERR(res);

        if (data->token->type == TOKEN_LEFT)
        {
            if ((res = call_param_list(data)) != RET_OK)
                return res;

            // TODO finish
            return WARNING_NOT_IMPLEMENTED;

        }
    }

    return RET_SYNTAX_ERROR;
}

int
statement_global(data_t *data)
{
    // STATEMENT_GLOBAL -> eof
    // STATEMENT_GLOBAL -> def FUNCTION_DEF STATEMENT_GLOBAL
    // STATEMENT_GLOBAL -> STATEMENT_LIST STATEMENT_GLOBAL

    int res = 0;
    get_next_token(data, &res);

    RETURN_IF_ERR(res);

    if (data->token->type == TOKEN_EOF)
    {
        return RET_OK;
    }
    else if (data->token->type == TOKEN_DEF)
    {
        if ((res = function_def(data)) == RET_OK)
        {
            return (statement_global(data));
        }
        else
        {
            return RET_SYNTAX_ERROR;
        }
    }
    else if ((res = statement_list(data)) == RET_OK)
    {
        return (statement_global(data));
    }
    else
    {
        return RET_SYNTAX_ERROR;
    }

}

int
if_clause(data_t *data)
{
    (void) data;
    // IF_CLAUSE -> if EXPRESSION : eol indent STATEMENT_LIST_NONEMPTY
    // else : eol indent STATEMENT_LIST_NONEMPTY
    return WARNING_NOT_IMPLEMENTED;
}

int
while_clause(data_t *data)
{
    (void) data;
    // WHILE_CLAUSE -> while EXPRESSION : eol indent STATEMENT_LIST_NONEMPTY
    return WARNING_NOT_IMPLEMENTED;
}

int
function_call(data_t *data)
{
    (void) data;
    // FUNCTION_CALL -> function_id ( CALL_PARAM_LIST )
    return WARNING_NOT_IMPLEMENTED;
}

int
def_param_list_next(data_t *data)
{
    // DEF_PARAM_LIST_NEXT -> , id DEF_PARAM_LIST_NEXT
    // DEF_PARAM_LIST_NEXT -> )

    int res = 0;

    get_next_token(data, &res);
    RETURN_IF_ERR(res);

    if (data->token->type == TOKEN_RIGHT)
    {
        return RET_OK;
    }
    else if (data->token->type == TOKEN_COMMA)
    {
        get_next_token(data, &res);
        RETURN_IF_ERR(res);

        if (data->token->type != TOKEN_IDENTIFIER)
            return (RET_SYNTAX_ERROR);

        return (def_param_list_next(data));
    }
    else
    {
        return RET_SYNTAX_ERROR;
    }

}

int
def_param_list(data_t *data)
{
    // DEF_PARAM_LIST -> )
    // DEF_PARAM_LIST -> id DEF_PARAM_LIST_NEXT
    int res = 0;

    get_next_token(data, &res);
    RETURN_IF_ERR(res);

    if (data->token->type == TOKEN_RIGHT)
    {
        return RET_OK;
    }
    else if (data->token->type == TOKEN_IDENTIFIER)
    {
        return (def_param_list_next(data));
    }
    else
    {
        return RET_SYNTAX_ERROR;
    }
}

int
call_param_list(data_t *data)
{
    // CALL_PARAM_LIST -> )
    // CALL_PARAM_LIST -> CALL_ELEM CALL_PARAM_LIST_NEXT
    int res = 0;

    get_next_token(data, &res);
    RETURN_IF_ERR(res);

    if (data->token->type == TOKEN_RIGHT)
    {
        return RET_OK;
    }
    else if ((res = call_elem(data)) == RET_OK)
    {
        return (call_param_list_next(data));
    }
    else
    {
        return res;
    }
}

int
call_param_list_next(data_t *data)
{
    // CALL_PARAM_LIST_NEXT -> , CALL_ELEM CALL_PARAM_LIST_NEXT
    // CALL_PARAM_LIST_NEXT -> )

    int res = 0;

    get_next_token(data, &res);
    RETURN_IF_ERR(res);

    if (data->token->type == TOKEN_RIGHT)
    {
        return RET_OK;
    }
    else if (data->token->type == TOKEN_COMMA)
    {
        if ((res = call_elem(data)) != RET_OK)
            return res;

        return (call_param_list_next(data));
    }
    else
    {
        return RET_SYNTAX_ERROR;
    }
}

int
call_elem(data_t *data)
{
    // CALL_ELEM -> id
    // CALL_ELEM -> literal
    // CALL_ELEM -> none

    if (data->token->type == TOKEN_NONE)
    {
        return RET_OK;
    }
    else if (data->token->type == TOKEN_IDENTIFIER)
    {
        return RET_OK;
    }
    else if (data->token->type == TOKEN_INT)
    {
        return RET_OK;
    }
    else if (data->token->type == TOKEN_FLOAT)
    {
        return RET_OK;
    }
    else if (data->token->type == TOKEN_DOC)
    {
        return RET_OK;
    }
    else if (data->token->type == TOKEN_LIT)
    {
        return RET_OK;
    }
}

int
return_statement(data_t *data)
{
    (void) data;
    // RETURN -> return RETURN_EXPRESSION
    return WARNING_NOT_IMPLEMENTED;
}

int
return_expression(data_t *data)
{
    (void) data;
    // RETURN_EXPRESSION -> eol
    // RETURN_EXPRESSION -> EXPRESSION eol
    return WARNING_NOT_IMPLEMENTED;
}

int
expression(data_t *data)
{
    (void) data;
    // mock rule:
    // EXPRESSION -> expr

    // guess we call PSA here
    return WARNING_NOT_IMPLEMENTED;
}

void
get_next_token(data_t *data, int *res)
{
    if (data->use_queue_for_read)
    {
        if (data->token_queue->first)
        {
            data->token = q_pop(data->token_queue);
            *res = RET_OK;
        }
        else
        {
            // queue is empty, stop reading from it
            // and read token like usual
            data->use_queue_for_read = false;
            *res = (int) get_token(data->token, data->file);
        }
    }
    else
    {
        *res = (int) get_token(data->token, data->file);
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

            // unreachable code, it's here for future extending of inititialization
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
            return RET_INTERNAL_ERROR;
        case 0:
            /* nothing failed -> OK */
            break;
        default:

            printf("%s:%u:init_state: invalid value (%d)\n", __func__, __LINE__, init_state);
            break;
    }

    // Feature ideas:

    //      symtable
    //      flags
    //      inside function -> allow return
    //      inside while -> context aware code generation for defvar ?

    return RET_OK;
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

