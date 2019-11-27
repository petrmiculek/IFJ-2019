/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
            xjacko04 (Jacko Daniel)
            xsetin00 (Setinský Jiří)
            xsisma01 (Šišma Vojtěch)
 */
#include "parser.h"
#include "scanner.h"
#include "err.h"
#include "token_queue.h"
#include "stack.h"
#include "psa.h"
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#define SEMANTICS 897987
// #define PSA 123123

data_t *data = NULL;

/**
    shorter way of expressing: return from function when things go wrong
    typically used after reading from scanner, but can be utilized anywhere
 */
int
symtable_insert(token_t *token, bool is_function);

#define RETURN_IF_ERR(res) do { if ((res) != RET_OK) { return (res);} } while(0);

#define GET_TOKEN() do { get_next_token(); RETURN_IF_ERR(data->get_token_res) } while(0);

int
parse(FILE *file)
{
    int res = RET_OK;

    if ((res = init_data(&data)) != RET_OK)
    {
        return res;
    }

    data->file = file;

    // start syntax analysis with starting nonterminal
    res = statement_global();

    if (res != RET_OK)
    {
        printf("err: %d\n", res);
    }

    free_static_stack();
    // clear_data();

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
            data->get_token_res = RET_OK;
        }
        else
        {
            // queue is empty, stop reading from it
            // and read token like usual
            data->use_queue_for_read = false;
            do
            {
                data->get_token_res = (int) get_token(data->token, data->file);

            }
            while (data->get_token_res == RET_OK && data->token->type == TOKEN_SPACE);
        }
    }
    else
    {
        do
        {
            data->get_token_res = (int) get_token(data->token, data->file);
        }
        while (data->get_token_res == RET_OK && data->token->type == TOKEN_SPACE);
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

int
init_data()
{
    if (NULL == (data = malloc(sizeof(data_t))))
    {

        return RET_INTERNAL_ERROR;
    }

    // token and its contents

    if ((data->token = malloc(sizeof(token_t))) == NULL)
    {
        return RET_INTERNAL_ERROR;
    }

    data->token->type = TOKEN_INVALID;

    data->token->string.length = 0;
    data->token->string.size = 0;
    data->token->string.str = NULL;

    // queue
    if (NULL == (data->token_queue = q_init_queue()))
    {
        return RET_INTERNAL_ERROR;
    }

    //init sym_tables
    if (NULL == (data->global_sym_table = ht_init()))
    {
        return RET_INTERNAL_ERROR;
    }

    if (NULL == (data->local_sym_table = ht_init()))
    {

        return RET_INTERNAL_ERROR;
    }

    if ((data->ID = malloc(sizeof(ht_item_t))) == NULL)
    {
        return RET_INTERNAL_ERROR;
    }

    if ((data->ID->data = malloc(sizeof(sym_table_item))) == NULL)
    {
        return RET_INTERNAL_ERROR;
    }

    // add more init from data_t
    data->use_queue_for_read = false;
    data->get_token_res = RET_OK;

    return RET_OK;
}

int
statement_list_nonempty()
{
    // STATEMENT_LIST_NONEMPTY -> STATEMENT STATEMENT_LIST
    int res;

    if ((res = statement()) != RET_OK)
    {
        return res;
    }

    if ((res = statement_list()) != RET_OK)
    {
        return res;
    }

    return RET_OK;
}

int
statement_list()
{
    // STATEMENT_LIST -> STATEMENT STATEMENT_LIST
    // STATEMENT_LIST -> dedent

    int res = RET_OK;

    GET_TOKEN()

    if (data->token->type == TOKEN_DEDENT)
    {
        return RET_OK;
    }
    else
    {
        q_enqueue(data->token, data->token_queue);
        data->use_queue_for_read = true;

        if ((res = statement()) != RET_OK)
        {
            return res;
        }

        if ((res = statement_list()) != RET_OK)
        {
            return res;
        }

        return RET_OK;
    }
}

int
function_def()
{
    // FUNCTION_DEF -> id ( DEF_PARAM_LIST ) : eol indent STATEMENT_LIST_NONEMPTY

    int res = RET_OK;

    // IMPORTANT: def token was read in by callee

    GET_TOKEN()

    if (data->token->type != TOKEN_IDENTIFIER)
        return RET_SYNTAX_ERROR;

#ifdef SEMANTICS
    // add function id to sym_table
    ht_item_t *search_res = ht_search(data->global_sym_table, data->token->string.str);

    if (search_res == NULL)
    {
        data->ID->data->identifier = data->token->string;
        data->ID->data->is_function = true;

        ht_insert(data->global_sym_table, data->token->string.str, data->ID->data);
    }
    else
    {
        return RET_SEMANTICAL_ERROR;
    }

    // _SEM identifier to symtable and check redefinition
    // check redefinition

    // temporary
    // symtable_insert(NULL, true);

#endif

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

    if ((res = statement_list_nonempty()) != RET_OK)
        return res;

    return RET_OK;
}

int
symtable_insert(token_t *token, bool is_function)
{
    int res = RET_OK;

    if (data->global_sym_table == NULL
        || token->type != TOKEN_IDENTIFIER)
    {
        return RET_INTERNAL_ERROR;
    }

    sym_table_item *new_item;

    if (NULL == (new_item = malloc(sizeof(sym_table_item))))
        return RET_INTERNAL_ERROR;

    copy_string(&new_item->identifier, &token->string); // TODO check if the copy is needed
    RETURN_IF_ERR(res);

    if (is_function)
    {
        // TODO parameters
        new_item->function_params_count = 5;
    }
    else
    { ; // possibly something
    }

    // TODO check that it doesn't exist, yet?

    res = ht_insert(data->global_sym_table, token->string.str, new_item);
    RETURN_IF_ERR(res);

    return RET_OK;
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

    int res = RET_OK;

    table_t *table;
    if (data->local == true)
    {
        table = data->local_sym_table;
    }
    else
    {
        table = data->global_sym_table;
    }

    GET_TOKEN()

    if (data->token->type == TOKEN_PASS)
    {
        if ((res = read_eol(true)) != RET_OK)
            return res;

        return RET_OK;
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
#ifdef SEMANTICS
                // definition of variable
                ht_item_t *local_search_res = ht_search(data->local_sym_table, lhs_identifier.string.str);
                ht_item_t *global_search_res = ht_search(data->global_sym_table, lhs_identifier.string.str);

                if (local_search_res == NULL && global_search_res == NULL)
                {
                    // identifier of that name does not exist
                    data->ID->data->identifier = lhs_identifier.string;
                    data->ID->data->is_function = false;

                    ht_insert(data->global_sym_table, lhs_identifier.string.str, data->ID->data);
                }
                else if (global_search_res != NULL && global_search_res->data->is_function == true)
                {
                    // identifier exists as a function (in global scope)
                    return RET_SEMANTICAL_ERROR;
                }
                else
                {
                    // identifier exists as a variable
                    // this statement just changes its value

                }
#endif // SEMANTICS


                // STATEMENT -> id = ASSIGN_RHS eol
                // _SEM add variable to sym_table
                if ((res = assign_rhs()) != RET_OK)
                    return res;

                if ((res = read_eol(true)) != RET_OK)
                    return res;

                return RET_OK;

            }
            else if (data->token->type == TOKEN_LEFT)
            {
                // STATEMENT -> id ( CALL_PARAM_LIST eol
                // check if function id is defined
#ifdef SEMANTICS
                ht_item_t *search_res = ht_search(data->global_sym_table, lhs_identifier.string.str);

                if (search_res == NULL)
                {
                    return RET_SEMANTICAL_ERROR;
                }
                else if (search_res->data->is_function == false)
                {
                    return RET_SEMANTICAL_ERROR;
                }
#endif // SEMANTICS
                // _SEM check if id is defined
                if ((res = call_param_list()) != RET_OK)
                    return res;

                if ((res = read_eol(true)) != RET_OK)
                    return res;

                return RET_OK;
            }
            else
            {
                // STATEMENT -> EXPRESSION eol

                q_enqueue(&lhs_identifier, data->token_queue); // identifier
                q_enqueue(data->token, data->token_queue); // token past identifier
                data->use_queue_for_read = true;

#ifdef PSA
                if ((res = solve_exp(data)) != RET_OK)
    {
        return res;
    }
#else
                if ((res = expression()) != RET_OK)
                {
                    return res;
                }
#endif

                if ((res = read_eol(true)) != RET_OK)
                    return res;

                return RET_OK;
            }
        }
        else
        {
            // data->token->type != TOKEN_IDENTIFIER
            // but we know that the token is a valid start of expression

            q_enqueue(data->token, data->token_queue);
            data->use_queue_for_read = true;

#ifdef PSA
            if ((res = solve_exp(data)) != RET_OK)
            {
                return res;
            }
#else
            if ((res = expression()) != RET_OK)
            {
                return res;
            }
#endif

            if ((res = read_eol(true)) != RET_OK)
                return res;

            return RET_OK;
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

    int res = RET_OK;

    GET_TOKEN()

    token_t token_tmp = *data->token;

    if (data->token->type == TOKEN_IDENTIFIER)
    {

#ifdef SEMANTICS
        ht_item_t *search_res = ht_search(data->global_sym_table, data->token->string.str);
        if (search_res == NULL)
        {
            return RET_SEMANTICAL_ERROR;
        }
        else if (search_res->data->is_function == false) //ID is not defined function
        {
            return RET_SEMANTICAL_ERROR;
        }

            // _SEM check if ID is defined

#endif // SEMANTICS

        GET_TOKEN()

        if (data->token->type == TOKEN_LEFT)
        {
            res = call_param_list();
#ifdef SEMANTICS
            if (search_res->data->function_params_count != data->par_cnt)
                return RET_SEMANTICAL_PARAMS_ERROR;
#endif // SEMANTICS

            return res;
        }
        else
        {
            q_enqueue(&token_tmp, data->token_queue);
            q_enqueue(data->token, data->token_queue);
            data->use_queue_for_read = true;

#ifdef PSA
            if ((res = solve_exp(data)) != RET_OK)
            {
                return res;
            }
#else
            if ((res = expression()) != RET_OK)
            {
                return res;
            }
#endif

            return RET_OK;
        }
    }
    else
    {
        q_enqueue(&token_tmp, data->token_queue);
        data->use_queue_for_read = true;

#ifdef PSA
        if ((res = solve_exp(data)) != RET_OK)
            {
                return res;
            }
#else
        if ((res = expression()) != RET_OK)
        {
            return res;
        }
#endif

        return RET_OK;
    }
}

int
statement_global()
{
    // STATEMENT_GLOBAL -> eof
    // STATEMENT_GLOBAL -> eol
    // STATEMENT_GLOBAL -> def FUNCTION_DEF STATEMENT_GLOBAL
    // STATEMENT_GLOBAL -> STATEMENT STATEMENT_GLOBAL

    int res = RET_OK;

    GET_TOKEN()

    if (data->token->type == TOKEN_EOF
        || data->token->type == TOKEN_EOL)
    {
        return RET_OK;
    }
    else if (data->token->type == TOKEN_DEF)
    {
        data->local = true;

        if ((res = function_def()) == RET_OK)
        {
            data->local = false;
            if ((res = statement_global()) != RET_OK)
            {
                return res;
            }

            return RET_OK;

        }
        else
        {
            return data->get_token_res;
        }
    }
    else
    {
        q_enqueue(data->token, data->token_queue);
        data->use_queue_for_read = true;

        data->local = false;

        if ((res = statement()) == RET_OK)
        {
            if ((res = statement_global()) != RET_OK)
            {
                return res;
            }

            return RET_OK;
        }
        else
        {
            return res;
        }
    }
}

int
if_clause()
{
    // IF_CLAUSE -> if EXPRESSION : eol indent STATEMENT_LIST_NONEMPTY
    //              else : eol indent STATEMENT_LIST_NONEMPTY

    // STATEMENT_LIST_NONEMPTY includes eol dedent

    // token already read in by callee

    int res = RET_OK;

    if (data->token->type != TOKEN_IF)
    {
        return RET_SYNTAX_ERROR;
    }

#ifdef PSA
    if ((res = solve_exp(data)) != RET_OK)
    {
        return res;
    }
#else
    if ((res = expression()) != RET_OK)
    {
        return res;
    }
#endif

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

    if ((res = statement_list_nonempty()) != RET_OK)
    {
        return res;
    }

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

    if ((res = statement_list_nonempty()) != RET_OK)
    {
        return res;
    }

    return RET_OK;
}

int
while_clause()
{
    // WHILE_CLAUSE -> while EXPRESSION : eol indent STATEMENT_LIST_NONEMPTY

    int res = RET_OK;

    // token already read in by callee

    if (data->token->type != TOKEN_WHILE)
    {
        return RET_SYNTAX_ERROR;
    }
#ifdef PSA
    {
        return res;
    }
#else
    if ((res = expression()) != RET_OK)
    {
        return res;
    }
#endif

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

    if ((res = statement_list_nonempty()) != RET_OK)
    {
        return res;
    }

    return RET_OK;
}

int
def_param_list_next()
{
    // DEF_PARAM_LIST_NEXT -> , id DEF_PARAM_LIST_NEXT
    // DEF_PARAM_LIST_NEXT -> )

    int res = RET_OK;

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

#ifdef SEMANTICS
        data->ID->data->function_params_count++;
#endif // SEMANTICS

        if ((res = def_param_list_next()) != RET_OK)
        {
            return data->get_token_res;
        }
        return RET_OK;
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
    int res = RET_OK;

#ifdef SEMANTICS
    data->ID->data->function_params_count = 0;
#endif // SEMANTICS
    GET_TOKEN()

    if (data->token->type == TOKEN_RIGHT)
    {
        return RET_OK;
    }
    else if (data->token->type == TOKEN_IDENTIFIER)
    {

#ifdef SEMANTICS
        data->ID->data->function_params_count++;
#endif // SEMANTICS

        if ((res = def_param_list_next()) != RET_OK)
        {
            return res;
        }
        return RET_OK;
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

    int res = RET_OK;

#ifdef SEMANTICS
    data->par_cnt = 0;
#endif // SEMANTICS
    GET_TOKEN()

    if (data->token->type == TOKEN_RIGHT)
    {
        return RET_OK;
    }
    else if ((res = call_elem()) == RET_OK)
    {
#ifdef SEMANTICS
        data->par_cnt++;
#endif // SEMANTICS
        if ((res = call_param_list_next()) != RET_OK)
        {
            return res;
        }
        return RET_OK;
    }
    else
    {
        return res;
    }
}

int
call_param_list_next()
{
    // CALL_PARAM_LIST_NEXT -> , CALL_ELEM CALL_PARAM_LIST_NEXT
    // CALL_PARAM_LIST_NEXT -> )

    int res = RET_OK;

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

#ifdef SEMANTICS
        data->par_cnt++;
#endif // SEMANTICS

        if ((res = call_param_list_next()) != RET_OK)
        {
            return res;
        }
        return RET_OK;
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

    int res = RET_OK;

    if (data->token->type != TOKEN_RETURN)
    {
        return RET_SYNTAX_ERROR;
    }

    if ((res = return_expression()) != RET_OK)
    {
        return res;
    }

    return RET_OK;
}

int
return_expression()
{
    // RETURN_EXPRESSION -> eol
    // RETURN_EXPRESSION -> EXPRESSION eol

    int res = RET_OK;

    GET_TOKEN()

    if (data->token->type == TOKEN_EOL)
    {
        if ((res = read_eol(false)) != RET_OK)
        {
            return res;
        }
        return RET_OK;
    }
    else
    {
        q_enqueue(data->token, data->token_queue);
        data->use_queue_for_read = true;

#ifdef PSA
        if ((res = solve_exp(data)) != RET_OK)
    {
        return res;
    }
#else
        if ((res = expression()) != RET_OK)
        {
            return res;
        }
#endif

        if ((res = read_eol(true)) != RET_OK)
        {
            return res;
        }

        return RET_OK;
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
