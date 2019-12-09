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
#include "psa.h"
#include "code_gen.h"
#include "symtable.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

data_t *data = NULL;

extern string_t code;

/**
    shorter way of expressing: return from function when things go wrong
    typically used after reading from scanner, but can be utilized anywhere
 */


#define GET_TOKEN()                        \
    do                                     \
    {                                      \
        get_next_token();                  \
        RETURN_IF_ERR(data->get_token_res) \
    } while (0);

int parse(FILE *file)
{
    int res;

    if ((res = init_data(&data)) != RET_OK)
        return res;

    data->file = file;

    if ((res = symtable_insert_predefined()) != RET_OK)
        return res;

    init_code_string();

    res = generate_file_header();
    RETURN_IF_ERR(res)
    insert_built_in_functions();
    RETURN_IF_ERR(res)
    res = insert_convert_to_bool_function();
    RETURN_IF_ERR(res)
    generate_main_scope_start();
    RETURN_IF_ERR(res)

    // start syntax analysis with starting nonterminal
    res = statement_global();
    RETURN_IF_ERR(res)

    res = check_all_functions_defined(data);

    free_static_stack();
    // clear_data();

    compiler_ret_value_comment(res); // retvalue ignored on purpose

    if (res == RET_OK)
    {
        generate_main_scope_end();
        RETURN_IF_ERR(res)
        print_code_string();
    }
    else
    {
        fprintf(stderr, "# err");
    }
    fprintf(stdout, "\n");

    return res;
}

int symtable_insert_predefined()
{
    int res;

    // print
    if (RET_OK != (res = symtable_insert_function("print", -1)))
        return res;

    // len
    if (RET_OK != (res = symtable_insert_function("len", 1)))
        return res;

    // substr
    if (RET_OK != (res = symtable_insert_function("substr", 3)))
        return res;

    // ord
    if (RET_OK != (res = symtable_insert_function("ord", 2)))
        return res;

    // chr
    if (RET_OK != (res = symtable_insert_function("chr", 1)))
        return res;

    // inputs
    if (RET_OK != (res = symtable_insert_function("inputs", 0)))
        return res;

    // inputi
    if (RET_OK != (res = symtable_insert_function("inputi", 0)))
        return res;

    // inputf
    if (RET_OK != (res = symtable_insert_function("inputf", 0)))
        return res;

    return RET_OK;
}

int symtable_insert_function(const char *identifier_arr, int param_count)
{
    int res;
    string_t *identifier_string;

    if (NULL == (identifier_string = malloc(sizeof(string_t))))
    {
        return RET_INTERNAL_ERROR;
    }
    init_string(identifier_string);
    append_c_string_to_string(identifier_string, identifier_arr);

    sym_table_item *data_symtable;
    if (NULL == (data_symtable = calloc(sizeof(sym_table_item), 1)))
        return RET_INTERNAL_ERROR;

    data_symtable->function_params_count = param_count;
    data_symtable->is_function = true;
    data_symtable->is_defined = true;
    data_symtable->just_index = 0;
    copy_string(&data_symtable->identifier, identifier_string);

    res = ht_insert(data->global_sym_table, identifier_string->str, data_symtable);
    if (res != RET_OK)
    {
        free_string(identifier_string);
        return res;
    }

    clear_string(identifier_string);

    return RET_OK;
}

int add_to_symtable(string_t *identifier, bool use_local_symtable)
{
    int res;
    string_t *uniq_identifier;
    table_t *table;
    char *prefix;
    if (use_local_symtable)
    {
        table = data->local_sym_table;
        prefix = data->function_ID->key;
        // TODO        ^^^ ->key or ->data->identifier? (unique name)
    }
    else
    {
        table = data->global_sym_table;
        if (data->ID->is_function)
        {
            prefix = "";
        }
        else
        {
            prefix = "global";
        }
    }

    uniq_identifier = generate_unique_identifier(prefix, identifier->str);

    if (uniq_identifier == NULL)
        return RET_INTERNAL_ERROR;

    res = copy_string(&data->ID->identifier, uniq_identifier);
    RETURN_IF_ERR(res)

    res = ht_insert(table, identifier->str, data->ID);

    return res;
}

void get_next_token()
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
                data->get_token_res = (int)get_token(data->token, data->file);
            } while (data->get_token_res == RET_OK && data->token->type == TOKEN_SPACE);
        }
    }
    else
    {
        do
        {
            data->get_token_res = (int)get_token(data->token, data->file);
        } while (data->get_token_res == RET_OK && data->token->type == TOKEN_SPACE);
    }
}

int read_eol(bool check_for_first_eol)
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
    } while (data->token->type == TOKEN_EOL);

    q_enqueue(data->token, data->token_queue);
    data->use_queue_for_read = true;
    return RET_OK;
}

int is_expression_start()
{
    if (data->token->type == TOKEN_IDENTIFIER || data->token->type == TOKEN_INT || data->token->type == TOKEN_FLOAT || data->token->type == TOKEN_LIT || data->token->type == TOKEN_DOC || data->token->type == TOKEN_LEFT || data->token->type == TOKEN_NONE)
    {
        return RET_OK;
    }
    else
    {
        return RET_SYNTAX_ERROR;
    }
}

int init_data()
{
    if (NULL == (data = calloc(sizeof(data_t), 1)))
    {

        return RET_INTERNAL_ERROR;
    }

    // token and its contents

    if ((data->token = calloc(sizeof(token_t), 1)) == NULL)
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
    /*
    // not needed since I swapped out ht_item_t for sym_table_item
    if ((data->ID = calloc(sizeof(ht_item_t), 1)) == NULL)
    {
        return RET_INTERNAL_ERROR;
    }
*/
    if ((data->function_ID = calloc(sizeof(ht_item_t), 1)) == NULL)
    {
        return RET_INTERNAL_ERROR;
    }

    if ((data->ID = calloc(sizeof(sym_table_item), 1)) == NULL)
    {
        return RET_INTERNAL_ERROR;
    }

    if ((data->call_params = q_init_queue()) == NULL)
    {
        return RET_INTERNAL_ERROR;
    }

    // add more init from data_t
    data->use_queue_for_read = false;
    data->get_token_res = RET_OK;

    return RET_OK;
}

int statement_list_nonempty()
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

int statement_list()
{
    // STATEMENT_LIST -> STATEMENT STATEMENT_LIST
    // STATEMENT_LIST -> dedent

    int res;

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

int function_def()
{
    // FUNCTION_DEF -> id ( DEF_PARAM_LIST ) : eol indent STATEMENT_LIST_NONEMPTY

    int res;
    int lp = 0;

    // IMPORTANT: def token was read in by callee

    GET_TOKEN()

    if (data->token->type != TOKEN_IDENTIFIER)
        return RET_SYNTAX_ERROR;

    // add function id to sym_table
    ht_item_t *global_search_res = ht_search(data->global_sym_table, data->token->string.str);

    if (global_search_res == NULL)
    {
        data->ID->is_function = true;
        data->ID->is_defined = true;
        data->ID->just_index = 0;

        res = add_to_symtable(&data->token->string, global);
        RETURN_IF_ERR(res)
        // _SEM function identifier to symtable
    }
    else
    {
        if (global_search_res->data->is_function == true && global_search_res->data->is_defined == false)
        {
            global_search_res->data->is_defined = true;
            lp = global_search_res->data->function_params_count;
        }
        else
        {
            return RET_SEMANTICAL_ERROR;
        }
    }

    data->function_ID = ht_search(data->global_sym_table, data->token->string.str);
    // for later definitions of variables in functions

    GET_TOKEN()

    if (data->token->type != TOKEN_LEFT)
        return RET_SYNTAX_ERROR;

    if ((res = def_param_list()) != RET_OK)
        return res;

    if (lp != 0) //check earlier local function call
    {
        if (lp != data->function_ID->data->function_params_count)
            return RET_SEMANTICAL_PARAMS_ERROR;
    }
    // IMPORTANT: right brace read in inside def_param_list

    GET_TOKEN()

    if (data->token->type != TOKEN_COLON)
        return RET_SYNTAX_ERROR;

    if ((res = read_eol(true)) != RET_OK)
        return res;

    GET_TOKEN()

    if (data->token->type != TOKEN_INDENT)
        return RET_SYNTAX_ERROR;
    
    res=generate_function_start(data->function_ID->data->identifier.str);
    RETURN_IF_ERR(res);
    
    if ((res = statement_list_nonempty()) != RET_OK)
        return res;
    
    res=generate_function_end(data->function_ID->data->identifier.str);
    RETURN_IF_ERR(res);
    
    // back to global scope
    data->parser_in_local_scope = global;

    return RET_OK;
}

int statement()
{
    // STATEMENT -> id = ASSIGN_RHS eol
    // STATEMENT -> ASSIGN_RHS eol
    //
    // STATEMENT -> pass eol
    // STATEMENT -> IF_CLAUSE
    // STATEMENT -> WHILE_CLAUSE
    // STATEMENT -> RETURN_STATEMENT

    int res;

    GET_TOKEN()

    if (data->token->type == TOKEN_PASS)
    {
        if ((res = read_eol(true)) != RET_OK)
            return res;

        return RET_OK;
    }
    else if (data->token->type == TOKEN_RETURN)
    {
        if (data->parser_in_local_scope == global)
        {
            return RET_SEMANTICAL_ERROR; // return statement outside function
        }

        if ((res = return_statement()) != RET_OK)
        {
            return res;
        }

        return RET_OK;
    }
    else if (data->token->type == TOKEN_IF)
    {
        if ((res = if_clause()) != RET_OK)
        {
            return res;
        }

        return RET_OK;
    }
    else if (data->token->type == TOKEN_WHILE)
    {
        if ((res = while_clause()) != RET_OK)
        {
            return res;
        }

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

                // definition of variable
                ht_item_t *local_search_res = ht_search(data->local_sym_table, lhs_identifier.string.str);
                ht_item_t *global_search_res = ht_search(data->global_sym_table, lhs_identifier.string.str);

                if (global_search_res != NULL && global_search_res->data->is_function == true)
                {
                    // identifier exists as a function (in global scope)
                    return RET_SEMANTICAL_ERROR;
                }

                if ((res = assign_rhs()) != RET_OK)
                    return res;

                // definition of variable
                if (data->parser_in_local_scope == local)
                {

                    // TODO we have to check if in function was early same variable as global
                    // find all variables in some structure in function_id if in global table are defined
                    // if variable is found RET SEM ERROR
                    if ((res = global_variables(lhs_identifier.string.str, 0)) != RET_OK)
                        return res;

                    if (local_search_res == NULL)
                    {
                        // identifier does not exist

                        data->ID->is_function = false;
                        data->ID->is_defined = true;

                        res = add_to_symtable(&lhs_identifier.string, local);
                        RETURN_IF_ERR((res))

                        generate_var_declare(data->ID->identifier.str, data->parser_in_local_scope);
                        RETURN_IF_ERR((res))
                    }
                }
                else
                {
                    if (global_search_res == NULL)
                    {
                        // identifier does not exist

                        data->ID->is_function = false;
                        data->ID->is_defined = true;

                        res = add_to_symtable(&lhs_identifier.string, global);
                        RETURN_IF_ERR((res))

                        generate_var_declare(data->ID->identifier.str, data->parser_in_local_scope);
                        RETURN_IF_ERR((res))
                    }
                    else
                    {
                        // identifier exists (in global scope)
                        global_search_res->data->is_defined = true; // before used as not defined in function
                    }
                }

                if ((res = read_eol(true)) != RET_OK)
                    return res;

                generate_move_exp_result_to_variable(&lhs_identifier, data);

                return RET_OK;
            }
            else if (data->token->type == TOKEN_LEFT)
            {
                // STATEMENT -> id ( CALL_PARAM_LIST eol
                // check if function id is defined

                ht_item_t *global_search_res = ht_search(data->global_sym_table, lhs_identifier.string.str);

                if (global_search_res == NULL)
                {
                    if (data->parser_in_local_scope == local)
                    {
                        // function without definition can only be called from another function
                        // not from global scope
                        // (function must be defined later)

                        // SEM: ADD TO SYMTABLE undefined
                        data->ID->is_function = true;
                        data->ID->is_defined = false;
                        
                        if ((res = call_param_list()) != RET_OK)
                            return res;
                        data->ID->function_params_count = data->function_call_param_count;
                        res = add_to_symtable(&lhs_identifier.string, global);
                        RETURN_IF_ERR(res)

                        global_search_res = ht_search(data->global_sym_table, lhs_identifier.string.str);
                    }
                    else
                    {
                        // cannot call undefined function from global scope
                        return RET_SEMANTICAL_ERROR;
                    }
                }
                else if (global_search_res->data->is_function == false)
                {
                    //ID exists but it is NOT a defined function
                    return RET_SEMANTICAL_ERROR;
                }
                else if (data->parser_in_local_scope == global)
                {

                    ht_item_t *swap = data->function_ID;
                    data->function_ID = global_search_res;

                    // check if all variables in function are defined
                    if ((res = global_variables(lhs_identifier.string.str, 1)) != RET_OK)
                    {
                        return RET_SEMANTICAL_ERROR;
                    }
                    // for later check of params variable

                    data->function_ID = swap;

                    if ((res = call_param_list()) != RET_OK)
                        return res;
                }
                else if (data->parser_in_local_scope == local)
                {
                    if ((res = call_param_list()) != RET_OK)
                    {
                        return res;
                    }
                }
                // end of if-else chain, no errors -> carry on

                if (global_search_res != NULL && global_search_res->data->function_params_count != -1 && global_search_res->data->function_params_count != data->function_call_param_count)
                {
                    return RET_SEMANTICAL_PARAMS_ERROR;
                }

                //call_predefined_function(&lhs_identifier);
                if (strcmp(lhs_identifier.string.str, "print") == 0)
                {
                    while (data->call_params->first != NULL)
                    {
                        token_t *param = q_pop(data->call_params);
                        generate_write(param, data);

                        if(data->call_params->first != NULL)
                            res = generate_print_space_or_newline(' ');
                        RETURN_IF_ERR(res);
                    }
                    res = generate_print_space_or_newline('\n');
                    RETURN_IF_ERR(res);
                }

                else
                {
                    //we can push params in queue
                    res = generate_function_param(data);
                    RETURN_IF_ERR(res);

                    res = generate_function_call(&global_search_res->data->identifier);
                    RETURN_IF_ERR(res);
                }

                if ((res = read_eol(true)) != RET_OK)
                    return res;

                return RET_OK;
            }
            else
            {
                // STATEMENT -> EXPRESSION eol

                q_enqueue(&lhs_identifier, data->token_queue); // identifier
                q_enqueue(data->token, data->token_queue);     // token past identifier
                data->use_queue_for_read = true;

                if ((res = (int)solve_exp(data)) != RET_OK)
                {
                    return res;
                }

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

            if ((res = (int)solve_exp(data)) != RET_OK)
            {
                return res;
            }

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
int call_predefined_function(token_t *identifier)
{
    // TODO don't forget about this
    return RET_SEMANTICAL_ERROR;
}

bool is_predefined_function(token_t *identifier)
{
    if (strcmp(identifier->string.str, "print") == 0)
    {
        return true;
    }

    // TODO add other functions (see symtable_insert_predefined)

    return false;
}

int assign_rhs()
{
    // ASSIGN_RHS -> id ( CALL_PARAM_LIST
    // ASSIGN_RHS -> EXPRESSION

    int res = RET_OK;

    GET_TOKEN()

    token_t token_tmp = *data->token;

    if (data->token->type == TOKEN_IDENTIFIER)
    {
        GET_TOKEN()

        if (data->token->type == TOKEN_LEFT)
        {

            ht_item_t *global_search_res = ht_search(data->global_sym_table, token_tmp.string.str);

            if (global_search_res == NULL)
            {
                if (data->parser_in_local_scope == local)
                {
                    // function without definition can only be called from another function
                    // not from global scope
                    // (function must be defined later)

                    // SEM: ADD TO SYMTABLE undefined
                    data->ID->is_function = true;
                    data->ID->is_defined = false;
                    if ((res = call_param_list()) != RET_OK)
                        return res;
                    data->ID->function_params_count = data->function_call_param_count;
                    res = add_to_symtable(&token_tmp.string, global);
                    RETURN_IF_ERR(res)
                    global_search_res = ht_search(data->global_sym_table, data->token->string.str);
                }
                else
                {
                    // cannot call undefined function from global scope
                    return RET_SEMANTICAL_ERROR;
                }
            }
            else if (global_search_res->data->is_function == false)
            {
                //ID exists but it is NOT a defined function
                return RET_SEMANTICAL_ERROR;
            }
            else if (data->parser_in_local_scope == global)
            {

                ht_item_t *swap = data->function_ID;
                data->function_ID = global_search_res;

                // check if all variables in function are defined
                if ((res = global_variables(token_tmp.string.str, 1)) != RET_OK)
                {
                    return RET_SEMANTICAL_ERROR;
                }
                // for later check of params variable

                data->function_ID = swap;
                if ((res = call_param_list()) != RET_OK)
                {
                    return res;
                }
            }
            else if (data->parser_in_local_scope == local)
            {
                if ((res = call_param_list()) != RET_OK)
                {
                    return res;
                }
            }

            if (global_search_res != NULL && global_search_res->data->function_params_count != -1 && global_search_res->data->function_params_count != data->function_call_param_count)
            {
                return RET_SEMANTICAL_PARAMS_ERROR;
            }

            //call_predefined_function(&token_tmp);
            if (strcmp(token_tmp.string.str, "print") == 0)
            {
                while (data->call_params->first != NULL)
                {
                    token_t *param = q_pop(data->call_params);
                    generate_write(param, data);

                    if(data->call_params->first != NULL)
                        res = generate_print_space_or_newline(' ');

                    RETURN_IF_ERR(res);
                }
                res = generate_print_space_or_newline('\n');
                RETURN_IF_ERR(res);
            }

            else
            {
                //we can push params in queue
                res = generate_function_param(data);
                RETURN_IF_ERR(res);

                res = generate_function_call(&global_search_res->data->identifier);
                RETURN_IF_ERR(res);
            }

            return res;
        }
        else
        {
            q_enqueue(&token_tmp, data->token_queue);
            q_enqueue(data->token, data->token_queue);
            data->use_queue_for_read = true;

            if ((res = (int)solve_exp(data)) != RET_OK)
            {
                return res;
            }

            return RET_OK;
        }
    }
    else
    {
        q_enqueue(&token_tmp, data->token_queue);
        data->use_queue_for_read = true;

        if ((res = (int)solve_exp(data)) != RET_OK)
        {
            return res;
        }

        return RET_OK;
    }
}

int statement_global()
{
    // STATEMENT_GLOBAL -> eof
    // STATEMENT_GLOBAL -> eol
    // STATEMENT_GLOBAL -> def FUNCTION_DEF STATEMENT_GLOBAL
    // STATEMENT_GLOBAL -> STATEMENT STATEMENT_GLOBAL

    int res;

    GET_TOKEN()

    if (data->token->type == TOKEN_EOF)
    {
        return RET_OK;
    }
    else if (data->token->type == TOKEN_EOL)
    {
        read_eol(false);
        return statement_global();
    }
    else if (data->token->type == TOKEN_DEF)
    {
        data->parser_in_local_scope = local;

        if ((res = function_def()) == RET_OK)
        {
            data->parser_in_local_scope = global;
            ht_clear_all(data->local_sym_table);

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
    else
    {
        q_enqueue(data->token, data->token_queue);
        data->use_queue_for_read = true;

        data->parser_in_local_scope = global;

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

int if_clause()
{
    // IF_CLAUSE -> if EXPRESSION : eol indent STATEMENT_LIST_NONEMPTY
    //              else : eol indent STATEMENT_LIST_NONEMPTY

    // STATEMENT_LIST_NONEMPTY includes eol dedent

    // 'if' token checked already

    int res;

    if ((res = (int)solve_exp(data)) != RET_OK)
    {
        return res;
    }

    string_t *uniq_identifier_if = generate_unique_identifier("local%label", "if");

    generate_if_begin(uniq_identifier_if->str);
    GET_TOKEN()

    if (data->token->type != TOKEN_COLON)
    {
        return RET_SYNTAX_ERROR;
    }

    if ((res = read_eol(true)) != RET_OK)
        return res;

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
    generate_if_else(uniq_identifier_if->str);

    GET_TOKEN()

    if (data->token->type != TOKEN_COLON)
    {
        return RET_SYNTAX_ERROR;
    }

    read_eol(true);

    GET_TOKEN()

    if (data->token->type != TOKEN_INDENT)
    {
        return RET_SYNTAX_ERROR;
    }

    if ((res = statement_list_nonempty()) != RET_OK)
    {
        return res;
    }
    generate_if_end(uniq_identifier_if->str);
    return RET_OK;
}

int while_clause()
{
    // WHILE_CLAUSE -> while EXPRESSION : eol indent STATEMENT_LIST_NONEMPTY

    int res;

    string_t *uniq_identifier_while = generate_unique_identifier("local%label", "while");
    generate_while_label(uniq_identifier_while->str);
    int counter = 0;

    // 'while' token checked already

    if ((res = (int)solve_exp(data)) != RET_OK)
    {
        return res;
    }

    GET_TOKEN()

    if (data->token->type != TOKEN_COLON)
    {
        return RET_SYNTAX_ERROR;
    }

    if ((res = read_eol(true)) != RET_OK)
        return res;

    generate_while_begin(uniq_identifier_while->str);

    GET_TOKEN()

    if (data->token->type != TOKEN_INDENT)
    {
        return RET_SYNTAX_ERROR;
    }

    if ((res = statement_list_nonempty()) != RET_OK)
    {
        return res;
    }

    generate_while_end(uniq_identifier_while->str);

    return RET_OK;
}

int def_param_list_next()
{
    // DEF_PARAM_LIST_NEXT -> , id DEF_PARAM_LIST_NEXT
    // DEF_PARAM_LIST_NEXT -> )

    int res;

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

        data->function_ID->data->function_params_count++;

        ht_item_t *local_search_res = ht_search(data->local_sym_table, data->token->string.str);

        if (local_search_res != NULL)
        {
            // identifier already exists, cannot redefine
            return RET_SEMANTICAL_ERROR;
        }
        else
        {
            // identifier does not exist
            // -> add param to symtable

            data->ID->is_function = false;
            data->ID->is_defined = true;

            res = add_to_symtable(&data->token->string, local);

            RETURN_IF_ERR((res))
        }

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

int def_param_list()
{
    // DEF_PARAM_LIST -> )
    // DEF_PARAM_LIST -> id DEF_PARAM_LIST_NEXT
    int res;

    data->function_ID->data->function_params_count = 0;

    GET_TOKEN()

    if (data->token->type == TOKEN_RIGHT)
    {
        return RET_OK;
    }
    else if (data->token->type == TOKEN_IDENTIFIER)
    {

        data->function_ID->data->function_params_count++;

        ht_item_t *local_search_res = ht_search(data->local_sym_table, data->token->string.str);

        if (local_search_res != NULL)
        {
            // identifier already exists, cannot redefine
            return RET_SEMANTICAL_ERROR;
        }
        else
        {
            // identifier does not exist
            // -> add param to symtable

            data->ID->is_function = false;
            data->ID->is_defined = true;

            res = add_to_symtable(&data->token->string, local);
            if (res != RET_OK)
            {
                return res;
            }
        }

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

int call_param_list()
{
    // CALL_PARAM_LIST -> )
    // CALL_PARAM_LIST -> CALL_ELEM CALL_PARAM_LIST_NEXT

    int res;

    ht_item_t *local_search_res;
    ht_item_t *global_search_res;
    data->function_call_param_count = 0;

    GET_TOKEN()

    if (data->token->type == TOKEN_RIGHT)
    {
        return RET_OK;
    }
    else if ((res = call_elem()) == RET_OK)
    {

        data->function_call_param_count++;

        // first param check if defined
        if (data->token->type == TOKEN_IDENTIFIER)
        {

            local_search_res = ht_search(data->local_sym_table, data->token->string.str);
            global_search_res = ht_search(data->global_sym_table, data->token->string.str);

            if (data->parser_in_local_scope == local)
            {
                if (local_search_res == NULL)
                {
                    // it could be global variable
                    // so we add id to global table as not defined

                    data->ID->is_defined = false;
                    data->ID->is_function = false;

                    res = add_to_symtable(&data->token->string, global);
                    RETURN_IF_ERR(res)

                    // save param for generating code

                    data->function_ID->data->global_variables[data->function_ID->data->just_index] =
                        data->token->string.str;
                    data->function_ID->data->just_index++;
                    // we have to add variable to function_ID structures
                }
            }
            else // we are in global scope
            {
                if (global_search_res == NULL)
                {
                    // not found so its SEM ERR
                    return RET_SEMANTICAL_ERROR;
                }
                else if (global_search_res->data->is_defined == false     // exist but not defined
                         || global_search_res->data->is_function == true) // id is function
                {
                    return RET_SEMANTICAL_ERROR;
                }
            }
        }
        q_enqueue(data->token, data->call_params);

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

int call_param_list_next()
{
    // CALL_PARAM_LIST_NEXT -> , CALL_ELEM CALL_PARAM_LIST_NEXT
    // CALL_PARAM_LIST_NEXT -> )

    ht_item_t *local_search_res;
    ht_item_t *global_search_res;

    int res;

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

        data->function_call_param_count++;

        // next param check if defined
        if (data->token->type == TOKEN_IDENTIFIER)
        {

            local_search_res = ht_search(data->local_sym_table, data->token->string.str);
            global_search_res = ht_search(data->global_sym_table, data->token->string.str);

            if (data->parser_in_local_scope == local)
            {
                if (local_search_res == NULL)
                {
                    // it could be global variable
                    // so we add id to global table as not defined

                    data->ID->is_defined = false;
                    data->ID->is_function = false;

                    res = add_to_symtable(&data->token->string, global);
                    RETURN_IF_ERR(res)

                    // save param for generating code

                    data->function_ID->data->global_variables[data->function_ID->data->just_index] =
                        data->token->string.str;

                    data->function_ID->data->just_index++;
                    // we have to add variable to function_ID structures
                }
            }
            else // we are in global scope
            {
                if (global_search_res == NULL)
                {
                    // no found so its SEM ERR
                    return RET_SEMANTICAL_ERROR;
                }
                else if (global_search_res->data->is_defined == false     // exist but not defined
                         || global_search_res->data->is_function == true) // id is function
                {
                    return RET_SEMANTICAL_ERROR;
                }
            }
        }
        // everything is ok
        q_enqueue(data->token, data->call_params);

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

int call_elem()
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

int return_statement()
{
    // RETURN -> return RETURN_EXPRESSION

    // 'return' token already checked by caller

    int res;

    if ((res = return_expression()) != RET_OK)
    {
        return res;
    }

    return RET_OK;
}

int return_expression()
{
    // RETURN_EXPRESSION -> eol
    // RETURN_EXPRESSION -> EXPRESSION eol

    int res;

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

        if ((res = (int)solve_exp(data)) != RET_OK)
        {
            return res;
        }

        if ((res = read_eol(true)) != RET_OK)
        {
            return res;
        }

        return RET_OK;
    }
}

int global_variables(char *str, int a)
{
    int i = data->function_ID->data->just_index - 1;
    ht_item_t *search_res;
    while (i >= 0)
    {
        if (a == 1)
        {
            search_res = ht_search(data->global_sym_table, data->function_ID->data->global_variables[i]);
            if (search_res->data->is_defined == false)
            {
                return RET_SEMANTICAL_ERROR;
            }
        }
        else if (strcmp(data->function_ID->data->global_variables[i], str) == 0)
        {
            return RET_SEMANTICAL_ERROR;
        }
        i--;
    }
    return RET_OK;
}
