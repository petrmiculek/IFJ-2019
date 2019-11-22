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
// TODO deklarace do .h



int
parse(FILE *file)
{
    int res;

    data_t *data = NULL;

    if ((res = init_data(&data)) != RET_OK)
    {
        return res;
    }

    data->file = file;
    // start syntax analysis with starting nonterminal
    res = statement_global(data);

    printf((res == RET_OK) ? "ok\n" : "err\n");

    free_static_stack();
    clear_data(&data);

    return res;
}

int
statement_list_nonempty(data_t *data)
{
    // STATEMENT_LIST_NONEMPTY -> STATEMENT STATEMENT_LIST

    int res;

    if ((res = statement(data)) != RET_OK) // TMP: token already loaded ?
    {
        return res;
    }

    return statement_list(data);
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

        if ((res = statement(data)) != RET_OK) // TMP: token in queue ( AND loaded )
        {
            return res;
        }

        return statement_list(data);
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

    if (data->token->type != TOKEN_EOL)
        return RET_SYNTAX_ERROR;

    get_next_token(data, &res);
    RETURN_IF_ERR(res)

    if (data->token->type != TOKEN_INDENT)
        return RET_SYNTAX_ERROR;

    return statement_list_nonempty(data); // TMP: token not loaded
}

int
statement(data_t *data)
{
    // STATEMENT -> id = ASSIGN_RHS eol
    // STATEMENT -> ASSIGN_RHS eol
    //
    // STATEMENT -> pass eol
    // STATEMENT -> IF_CLAUSE
    // STATEMENT -> WHILE_CLAUSE
    // STATEMENT -> RETURN_STATEMENT

    int res = 0;

    get_next_token(data, &res);
    RETURN_IF_ERR(res)

    if (data->token->type == TOKEN_PASS)
    {
        get_next_token(data, &res);
        RETURN_IF_ERR(res)

        if (data->token->type == TOKEN_EOL)
        {
            return RET_OK;
        }
        else
        {
            return RET_SYNTAX_ERROR;
        }
    }
    else if (return_statement(data) == RET_OK)
    {
        return RET_OK;
    }
    else if (if_clause(data) == RET_OK)
    {
        return RET_OK;
    }
    else if (while_clause(data) == RET_OK)
    {
        return RET_OK;
    }
    else
    {
        // TODO must check if token is a valid start of expression

        if (data->token->type == TOKEN_IDENTIFIER)
        {
            // non LL1 decision:
            // id = RHS eol
            // RHS eol ( something like: id * id + 4 eol )

            q_enqueue(data->token, data->token_queue);
            data->use_queue_for_read = false; // keep in queue

            get_next_token(data, &res);
            RETURN_IF_ERR(res)

            if (data->token->type == TOKEN_ASSIGN)
            {
                // STATEMENT -> id = ASSIGN_RHS eol

                if ((res = assign_rhs(data)) != RET_OK) // cannot use queue
                    return res;

                get_next_token(data, &res);
                RETURN_IF_ERR(res)

                if (data->token->type == TOKEN_EOL)
                {
                    data->use_queue_for_read = true;

                    get_next_token(data, &res);
                    RETURN_IF_ERR(res)

                    // read info from token == identifier

                    return RET_OK;
                }
                else
                {
                    data->use_queue_for_read = true;

                    get_next_token(data, &res);
                    RETURN_IF_ERR(res)

                    // possibly read info from token == identifier
                    // but here it's a syntax error

                    return RET_SYNTAX_ERROR;
                }
            }
            else
            {
                // STATEMENT -> ASSIGN_RHS eol

                q_enqueue(data->token, data->token_queue); // token past identifier
                data->use_queue_for_read = true;

                if ((res = assign_rhs(data)) != RET_OK)
                    return res;

                get_next_token(data, &res);
                RETURN_IF_ERR(res)

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
        else
        {
            // data->token->type != TOKEN_IDENTIFIER

            q_enqueue(data->token, data->token_queue);
            data->use_queue_for_read = true;

            if ((res = expression(data)) != RET_OK)
                return res;

            get_next_token(data, &res);
            RETURN_IF_ERR(res)

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
}

int
assign_rhs(data_t *data)
{
    // ASSIGN_RHS -> id ( CALL_PARAM_LIST
    // ASSIGN_RHS -> EXPRESSION

    int res = 0;
    get_next_token(data, &res);
    RETURN_IF_ERR(res)
    // TMP: queue should be empty at this point

    token_t local_token = *data->token;

    if (data->token->type == TOKEN_IDENTIFIER)
    {
        get_next_token(data, &res);
        RETURN_IF_ERR(res)

        if (data->token->type == TOKEN_LEFT)
        {
            return call_param_list(data);
        }
        else
        {
            q_enqueue(data->token, data->token_queue);
            data->use_queue_for_read = true;

            return expression(data);
        }
    }
    else
    {

        q_enqueue(&local_token, data->token_queue);
        q_enqueue(data->token, data->token_queue);
        data->use_queue_for_read = true;

        return expression(data);
    }
}

int
statement_global(data_t *data)
{
    // STATEMENT_GLOBAL -> eof
    // STATEMENT_GLOBAL -> def FUNCTION_DEF STATEMENT_GLOBAL
    // STATEMENT_GLOBAL -> STATEMENT STATEMENT_GLOBAL

    int res = 0;
    get_next_token(data, &res);
    RETURN_IF_ERR(res)

    if (data->token->type == TOKEN_EOF
        || data->token->type == TOKEN_EOL) // TODO Not in grammar -> check validity
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
    else
    {
        q_enqueue(data->token, data->token_queue);
        data->use_queue_for_read = true;

        if ((res = statement(data)) == RET_OK) // TMP: token in queue ( AND loaded )
        {
            return (statement_global(data));
        }
        else
        {
            return RET_SYNTAX_ERROR;
        }
    }
}

int
if_clause(data_t *data)
{
    // IF_CLAUSE -> if EXPRESSION : eol indent STATEMENT_LIST_NONEMPTY
    // else : eol indent STATEMENT_LIST_NONEMPTY

    // token already read in by callee

    int res = 0;

    if (data->token->type != TOKEN_IF)
    {
        return RET_SYNTAX_ERROR;
    }

    if ((res = expression(data)) != RET_OK)
    {
        return res;
    }

    get_next_token(data, &res);
    RETURN_IF_ERR(res)

    if (data->token->type != TOKEN_COLON)
    {
        return RET_SYNTAX_ERROR;
    }

    get_next_token(data, &res);
    RETURN_IF_ERR(res)

    if (data->token->type != TOKEN_EOL)
    {
        return RET_SYNTAX_ERROR;
    }

    get_next_token(data, &res);
    RETURN_IF_ERR(res)

    if (data->token->type != TOKEN_INDENT)
    {
        return RET_SYNTAX_ERROR;
    }

    if ((res = statement_list_nonempty(data)) != RET_OK) // TMP: token not loaded
    {
        return res;
    }

    // make sure statement_list is properly terminated

    // eol is read in inside statements

    get_next_token(data, &res);
    RETURN_IF_ERR(res)

    if (data->token->type != TOKEN_ELSE)
    {
        return RET_SYNTAX_ERROR;
    }

    get_next_token(data, &res);
    RETURN_IF_ERR(res)

    if (data->token->type != TOKEN_COLON)
    {
        return RET_SYNTAX_ERROR;
    }

    get_next_token(data, &res);
    RETURN_IF_ERR(res)

    if (data->token->type != TOKEN_EOL)
    {
        return RET_SYNTAX_ERROR;
    }

    get_next_token(data, &res);
    RETURN_IF_ERR(res)

    if (data->token->type != TOKEN_INDENT)
    {
        return RET_SYNTAX_ERROR;
    }

    return statement_list_nonempty(data); // TMP: token not loaded
}

int
while_clause(data_t *data)
{
    // WHILE_CLAUSE -> while EXPRESSION : eol indent STATEMENT_LIST_NONEMPTY

    int res = 0;

    // token already read in by callee

    if (data->token->type != TOKEN_WHILE)
    {
        return RET_SYNTAX_ERROR;
    }
/*
    get_next_token(data, &res);
    RETURN_IF_ERR(res)
*/
    if ((res = expression(data)) != RET_OK)
    {
        return res;
    }

    get_next_token(data, &res);
    RETURN_IF_ERR(res)

    if (data->token->type != TOKEN_COLON)
    {
        return RET_SYNTAX_ERROR;
    }

    get_next_token(data, &res);
    RETURN_IF_ERR(res)

    if (data->token->type != TOKEN_EOL)
    {
        return RET_SYNTAX_ERROR;
    }

    get_next_token(data, &res);
    RETURN_IF_ERR(res)

    if (data->token->type != TOKEN_INDENT)
    {
        return RET_SYNTAX_ERROR;
    }

    return statement_list_nonempty(data); // TMP: token not loaded
}

int
def_param_list_next(data_t *data)
{
    // DEF_PARAM_LIST_NEXT -> , id DEF_PARAM_LIST_NEXT
    // DEF_PARAM_LIST_NEXT -> )

    int res = 0;

    get_next_token(data, &res);
    RETURN_IF_ERR(res)

    if (data->token->type == TOKEN_RIGHT)
    {
        return RET_OK;
    }
    else if (data->token->type == TOKEN_COMMA)
    {
        get_next_token(data, &res);
        RETURN_IF_ERR(res)

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
    RETURN_IF_ERR(res)

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
    RETURN_IF_ERR(res)

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
    RETURN_IF_ERR(res)

    if (data->token->type == TOKEN_RIGHT)
    {
        return RET_OK;
    }
    else if (data->token->type == TOKEN_COMMA)
    {
        get_next_token(data, &res);
        RETURN_IF_ERR(res)

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
    else
    {
        return RET_SYNTAX_ERROR;
    }
}

int
return_statement(data_t *data)
{
    // RETURN -> return RETURN_EXPRESSION

    // token already read in by callee

    if (data->token->type != TOKEN_RETURN)
    {
        return RET_SYNTAX_ERROR;
    }

    return return_expression(data);
}

int
return_expression(data_t *data)
{
    // RETURN_EXPRESSION -> eol
    // RETURN_EXPRESSION -> EXPRESSION eol

    int res = 0;

    get_next_token(data, &res);
    RETURN_IF_ERR(res)

    if (data->token->type == TOKEN_EOL)
    {
        return RET_OK;
    }
    else
    {
        q_enqueue(data->token, data->token_queue);
        data->use_queue_for_read = true;

        // expresssion() starts with no tokens read in

        if ((res = expression(data)) != RET_OK)
        {
            return res;
        }

        get_next_token(data, &res);
        RETURN_IF_ERR(res)

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

int
expression(data_t *data)
{
    // mock rule:
    // EXPRESSION -> expr
    // TODO delete rule when done

    // call PSA here


    // temporary solution:
    // --------- simulate PSA ---------
    int res = 0;

    do
    {
        // skip tokens
        get_next_token(data, &res);
        RETURN_IF_ERR(res)

        // until end of expression is reached
    }
    while (data->token->type != TOKEN_EOL && data->token->type != TOKEN_EOF && data->token->type != TOKEN_COLON);

    // unget eol/eof/: token
    q_enqueue(data->token, data->token_queue);
    data->use_queue_for_read = true;

    return RET_OK;
    // -------------- end --------------
}

void
get_next_token(data_t *data, int *res)
{
    if (data->use_queue_for_read)
    {
        if (data->token_queue->first != NULL)
        {
            data->token = q_pop(data->token_queue);
            *res = RET_OK;
        }
        else
        {
            // queue is empty, stop reading from it
            // and read token like usual
            data->use_queue_for_read = false;
            do
            {
                *res = (int) get_token(data->token, data->file);

            }
            while (*res == RET_OK && data->token->type == TOKEN_SPACE);
        }
    }
    else
    {
        do
        {
            *res = (int) get_token(data->token, data->file);

        }
        while (*res == RET_OK && data->token->type == TOKEN_SPACE);
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

    (*data)->token->type = TOKEN_INVALID;

    (*data)->token->string.length = 0;
    (*data)->token->string.size = 0;
    (*data)->token->string.str = NULL;

    // WARNING: token is not fully initialized until get_token() is run
/*
    // This was replaced by the lines above
    // Scanner initializes the needed string on its own, so this only leaked memory
    if (RET_OK != init_string(&(*data)->token->string))
    {
        init_state = 3;
        goto cleanup;
    }
*/
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

            /*
             free_string(&(*data)->token->string);
             // see block of comments 10 lines up for explanation
             */
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

    // add more init from data_t
    (*data)->use_queue_for_read = false;

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

