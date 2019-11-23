#include "parser.h"
#include "scanner.h"
#include "err.h"
#include "token_queue.h"
#include "stack.h"
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

data_t *data = NULL;

/**
    shorter way of expressing: return from function when things go wrong
    typically used after reading from scanner, but can be utilized anywhere
 */
#define RETURN_IF_ERR(res) do { if ((res) != RET_OK) { return (res);} } while(0);

#define GET_TOKEN() do { get_next_token(); RETURN_IF_ERR(data->res) } while(0);

int
parse(FILE *file)
{
    int res;

    if ((res = init_data(&data)) != RET_OK)
    {
        return res;
    }

    data->file = file;

    // start syntax analysis with starting nonterminal
    res = statement_global();

    printf((res == RET_OK) ? "ok\n" : "err\n");

    free_static_stack();
    clear_data();

    return res;
}

void
get_next_token()
{
    // for explanation, see function declaration

    if (data->use_queue_for_read)
    {
        if (data->token_queue->first != NULL)
        {
            data->token = q_pop(data->token_queue);
            data->res = RET_OK;
        }
        else
        {
            // queue is empty, stop reading from it
            // and read token like usual
            data->use_queue_for_read = false;
            do
            {
                data->res = (int) get_token(data->token, data->file);

            }
            while (data->res == RET_OK && data->token->type == TOKEN_SPACE);
        }
    }
    else
    {
        do
        {
            data->res = (int) get_token(data->token, data->file);

        }
        while (data->res == RET_OK && data->token->type == TOKEN_SPACE);
    }
}

int
read_eol(bool check_for_first_eol)
{
    // check for first eol token
    // skip further eol tokens

    if (check_for_first_eol)
    {
        GET_TOKEN()

        if (data->token->type != TOKEN_EOL)
        {
            return RET_SYNTAX_ERROR;
        }
    }

    do
    {
        GET_TOKEN()
    }
    while (data->token->type == TOKEN_EOL);

    q_enqueue(data->token, data->token_queue);
    data->use_queue_for_read = true;

    return RET_OK;
}

int
is_expression_start()
{
    if (data->token->type == TOKEN_IDENTIFIER
        || data->token->type == TOKEN_INT
        || data->token->type == TOKEN_FLOAT
        || data->token->type == TOKEN_LIT
        || data->token->type == TOKEN_DOC
        || data->token->type == TOKEN_LEFT
        || data->token->type == TOKEN_NONE)
    {
        return RET_OK;
    }
    else
    {
        return RET_SYNTAX_ERROR;
    }
}

void
clear_data()
{
    q_free_queue(data->token_queue);
    free_string(&data->token->string);
    free(data->token);
    free(data);
    data = NULL;
}

int
statement_list_nonempty()
{
    // STATEMENT_LIST_NONEMPTY -> STATEMENT STATEMENT_LIST

    int res;

    if ((res = statement()) != RET_OK) // TMP: token already loaded ?
    {
        return res;
    }

    return statement_list();
}

int
statement_list()
{
    // STATEMENT_LIST -> STATEMENT STATEMENT_LIST
    // STATEMENT_LIST -> dedent

    int res = 0;

    GET_TOKEN()

    if (data->token->type == TOKEN_DEDENT)
    {
        return RET_OK;
    }
    else
    {
        q_enqueue(data->token, data->token_queue);
        data->use_queue_for_read = true;

        if ((res = statement()) != RET_OK) // TMP: token in queue ( AND loaded )
        {
            return res;
        }

        return statement_list();
    }
}

int
function_def()
{
    // FUNCTION_DEF -> id ( DEF_PARAM_LIST ) : eol indent STATEMENT_LIST_NONEMPTY

    int res = 0;

    // IMPORTANT: def token was read in by callee

    GET_TOKEN()

    if (data->token->type != TOKEN_IDENTIFIER)
        return RET_SYNTAX_ERROR;

    // identifier to symtable
    // check redefinition

    GET_TOKEN()

    if (data->token->type != TOKEN_LEFT)
        return RET_SYNTAX_ERROR;

    if ((res = def_param_list()) != RET_OK)
        return res;

    // IMPORTANT: right brace read in inside def_param_list

    GET_TOKEN()

    if (data->token->type != TOKEN_COLON)
        return RET_SYNTAX_ERROR;

    GET_TOKEN()

    if (data->token->type != TOKEN_EOL)
        return RET_SYNTAX_ERROR;

    GET_TOKEN()

    if (data->token->type != TOKEN_INDENT)
        return RET_SYNTAX_ERROR;

    return statement_list_nonempty(); // TMP: token not loaded
}

int
statement()
{
    // STATEMENT -> id = ASSIGN_RHS eol
    // STATEMENT -> ASSIGN_RHS eol
    //
    // STATEMENT -> pass eol
    // STATEMENT -> IF_CLAUSE
    // STATEMENT -> WHILE_CLAUSE
    // STATEMENT -> RETURN_STATEMENT

    int res = 0;

    GET_TOKEN()

    if (data->token->type == TOKEN_PASS)
    {
        return read_eol(true);
    }
    else if (return_statement() == RET_OK)
    {
        return RET_OK;
    }
    else if (if_clause() == RET_OK)
    {
        return RET_OK;
    }
    else if (while_clause() == RET_OK)
    {
        return RET_OK;
    }
    else if (is_expression_start() == RET_OK)
    {
        if (data->token->type == TOKEN_IDENTIFIER)
        {
            // non LL1 decision:
            // id = RHS eol
            // id ( CALL_PARAM_LIST eol
            // expression eol ( something like: id * id + 4 eol )

            token_t lhs_identifier = *data->token;

            GET_TOKEN()

            if (data->token->type == TOKEN_ASSIGN)
            {
                // STATEMENT -> id = ASSIGN_RHS eol

                if ((res = assign_rhs()) != RET_OK)
                    return res;

                GET_TOKEN()

                return read_eol(true);

            }
            else if (data->token->type == TOKEN_LEFT)
            {
                // STATEMENT -> id ( CALL_PARAM_LIST eol

                if ((res = call_param_list()) != RET_OK)
                    return res;

                return read_eol(true);
            }
            else
            {
                // STATEMENT -> EXPRESSION eol

                q_enqueue(&lhs_identifier, data->token_queue); // token past identifier
                q_enqueue(data->token, data->token_queue); // token past identifier
                data->use_queue_for_read = true;

                if ((res = expression()) != RET_OK)
                    return res;

                return read_eol(true);
            }
        }
        else
        {
            // data->token->type != TOKEN_IDENTIFIER
            // but we know that the token is a valid start of expression

            q_enqueue(data->token, data->token_queue);
            data->use_queue_for_read = true;

            if ((res = expression()) != RET_OK)
                return res;

            return read_eol(true);
        }
    }
    else
    {
        // unexpected token type
        return RET_SYNTAX_ERROR;
    }
}


int
assign_rhs()
{
    // ASSIGN_RHS -> id ( CALL_PARAM_LIST
    // ASSIGN_RHS -> EXPRESSION

    GET_TOKEN()

    token_t token_tmp = *data->token;

    if (data->token->type == TOKEN_IDENTIFIER)
    {
        GET_TOKEN()

        if (data->token->type == TOKEN_LEFT)
        {
            return call_param_list();
        }
        else
        {
            q_enqueue(&token_tmp, data->token_queue);
            q_enqueue(data->token, data->token_queue);
            data->use_queue_for_read = true;

            return expression();
        }
    }
    else
    {
        q_enqueue(&token_tmp, data->token_queue);
        data->use_queue_for_read = true;

        return expression();
    }
}

int
statement_global()
{
    // STATEMENT_GLOBAL -> eof
    // STATEMENT_GLOBAL -> def FUNCTION_DEF STATEMENT_GLOBAL
    // STATEMENT_GLOBAL -> STATEMENT STATEMENT_GLOBAL

    int res = 0;
    GET_TOKEN()

    if (data->token->type == TOKEN_EOF
        || data->token->type == TOKEN_EOL) // TODO Not in grammar -> check validity
    {
        return RET_OK;
    }
    else if (data->token->type == TOKEN_DEF)
    {
        if ((data->res = function_def()) == RET_OK)
        {
            return (statement_global());
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

        if ((data->res = statement()) == RET_OK) // TMP: token in queue ( AND loaded )
        {
            return (statement_global());
        }
        else
        {
            return RET_SYNTAX_ERROR;
        }
    }
}

int
if_clause()
{
    // IF_CLAUSE -> if EXPRESSION : eol indent STATEMENT_LIST_NONEMPTY
    // else : eol indent STATEMENT_LIST_NONEMPTY

    // token already read in by callee

    int res = 0;

    if (data->token->type != TOKEN_IF)
    {
        return RET_SYNTAX_ERROR;
    }

    if ((res = expression()) != RET_OK)
    {
        return res;
    }

    GET_TOKEN()

    if (data->token->type != TOKEN_COLON)
    {
        return RET_SYNTAX_ERROR;
    }

    GET_TOKEN()

    if (data->token->type != TOKEN_EOL)
    {
        return RET_SYNTAX_ERROR;
    }

    GET_TOKEN()

    if (data->token->type != TOKEN_INDENT)
    {
        return RET_SYNTAX_ERROR;
    }

    if ((res = statement_list_nonempty()) != RET_OK) // TMP: token not loaded
    {
        return res;
    }

    // make sure statement_list is properly terminated

    // eol is read in inside statements

    GET_TOKEN()

    if (data->token->type != TOKEN_ELSE)
    {
        return RET_SYNTAX_ERROR;
    }

    GET_TOKEN()

    if (data->token->type != TOKEN_COLON)
    {
        return RET_SYNTAX_ERROR;
    }

    GET_TOKEN()

    if (data->token->type != TOKEN_EOL)
    {
        return RET_SYNTAX_ERROR;
    }

    GET_TOKEN()

    if (data->token->type != TOKEN_INDENT)
    {
        return RET_SYNTAX_ERROR;
    }

    return statement_list_nonempty(); // TMP: token not loaded
}

int
while_clause()
{
    // WHILE_CLAUSE -> while EXPRESSION : eol indent STATEMENT_LIST_NONEMPTY

    int res = 0;

    // token already read in by callee

    if (data->token->type != TOKEN_WHILE)
    {
        return RET_SYNTAX_ERROR;
    }

    if ((res = expression()) != RET_OK)
    {
        return res;
    }

    GET_TOKEN()

    if (data->token->type != TOKEN_COLON)
    {
        return RET_SYNTAX_ERROR;
    }

    GET_TOKEN()

    if (data->token->type != TOKEN_EOL)
    {
        return RET_SYNTAX_ERROR;
    }

    GET_TOKEN()

    if (data->token->type != TOKEN_INDENT)
    {
        return RET_SYNTAX_ERROR;
    }

    return statement_list_nonempty(); // TMP: token not loaded
}

int
def_param_list_next()
{
    // DEF_PARAM_LIST_NEXT -> , id DEF_PARAM_LIST_NEXT
    // DEF_PARAM_LIST_NEXT -> )

    GET_TOKEN()

    if (data->token->type == TOKEN_RIGHT)
    {
        return RET_OK;
    }
    else if (data->token->type == TOKEN_COMMA)
    {
        GET_TOKEN()

        if (data->token->type != TOKEN_IDENTIFIER)
            return (RET_SYNTAX_ERROR);

        return (def_param_list_next());
    }
    else
    {
        return RET_SYNTAX_ERROR;
    }

}

int
def_param_list()
{
    // DEF_PARAM_LIST -> )
    // DEF_PARAM_LIST -> id DEF_PARAM_LIST_NEXT
    int res = 0;

    GET_TOKEN()

    if (data->token->type == TOKEN_RIGHT)
    {
        return RET_OK;
    }
    else if (data->token->type == TOKEN_IDENTIFIER)
    {
        return (def_param_list_next());
    }
    else
    {
        return RET_SYNTAX_ERROR;
    }
}

int
call_param_list()
{
    // CALL_PARAM_LIST -> )
    // CALL_PARAM_LIST -> CALL_ELEM CALL_PARAM_LIST_NEXT

    GET_TOKEN()

    if (data->token->type == TOKEN_RIGHT)
    {
        return RET_OK;
    }
    else if ((data->res = call_elem()) == RET_OK)
    {
        return (call_param_list_next());
    }
    else
    {
        return data->res;
    }
}

int
call_param_list_next()
{
    // CALL_PARAM_LIST_NEXT -> , CALL_ELEM CALL_PARAM_LIST_NEXT
    // CALL_PARAM_LIST_NEXT -> )

    int res = 0;

    GET_TOKEN()

    if (data->token->type == TOKEN_RIGHT)
    {
        return RET_OK;
    }
    else if (data->token->type == TOKEN_COMMA)
    {
        GET_TOKEN()

        if ((res = call_elem()) != RET_OK)
            return res;

        return (call_param_list_next());
    }
    else
    {
        return RET_SYNTAX_ERROR;
    }
}

int
call_elem()
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
return_statement()
{
    // RETURN -> return RETURN_EXPRESSION

    // token already read in by callee

    if (data->token->type != TOKEN_RETURN)
    {
        return RET_SYNTAX_ERROR;
    }

    return return_expression();
}

int
return_expression()
{
    // RETURN_EXPRESSION -> eol
    // RETURN_EXPRESSION -> EXPRESSION eol

    int res = 0;

    GET_TOKEN()

    if (data->token->type == TOKEN_EOL)
    {
        return RET_OK;
    }
    else
    {
        q_enqueue(data->token, data->token_queue);
        data->use_queue_for_read = true;

        // expression() starts with no tokens read in

        if ((res = expression()) != RET_OK)
        {
            return res;
        }

        GET_TOKEN()

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
expression()
{
    // call PSA here


    // temporary solution:
    // --------- simulate PSA ---------
    do
    {
        GET_TOKEN()
    }
    while (data->token->type != TOKEN_EOL && data->token->type != TOKEN_EOF && data->token->type != TOKEN_COLON);

    q_enqueue(data->token, data->token_queue);
    data->use_queue_for_read = true;

    return RET_OK;
    // -------------- end --------------
}



int
init_data()
{
    int init_state = 0;

    if (NULL == (data = malloc(sizeof(data_t))))
    {
        init_state = 1;
        goto cleanup;
    }

    // token and its contents

    if ((data->token = malloc(sizeof(token_t))) == NULL)
    {
        init_state = 2;
        goto cleanup;
    }

    data->token->type = TOKEN_INVALID;

    data->token->string.length = 0;
    data->token->string.size = 0;
    data->token->string.str = NULL;

    // WARNING: token is not fully initialized until get_token() is run
/*
    // This was replaced by the lines above
    // Scanner initializes the needed string on its own, so this only leaked memory
    if (RET_OK != init_string(&data->token->string))
    {
        init_state = 3;
        goto cleanup;
    }
*/
    // queue
    if (NULL == (data->token_queue = q_init_queue()))
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
            q_free_queue(data->token_queue);
            // falls through
        case 4:

            /*
             free_string(&data->token->string);
             // see block of comments 10 lines up for explanation
             */
            // falls through
        case 3:

            free(data->token);
            // falls through
        case 2:

            free(data);
            data = NULL;
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
    data->use_queue_for_read = false;

    return RET_OK;
}
