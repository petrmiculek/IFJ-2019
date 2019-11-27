#ifndef IFJ_2019__PARSER_H
#define IFJ_2019__PARSER_H
/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
            xjacko04 (Jacko Daniel)
            xsetin00 (Setinský Jiří)
            xsisma01 (Šišma Vojtěch)
 */
#include <stdio.h>
#include <stdbool.h>
#include "scanner.h"
#include "token_queue.h"
#include "symtable.h"

struct _struct_data_t
{
    FILE *file;
    token_queue_t *token_queue;
    token_t *token;
    int use_queue_for_read;

    table_t *sym_table;

    bool in_function; // TODO only allow return statement inside f-definition

    int res; // result of most-recent operation

    // Feature ideas:

    //      flags
    //      inside function -> allow return
    //      inside while -> context aware code generation for defvar ?


};
typedef struct _struct_data_t data_t;

/**
 * @brief initialize and start syntax analysis
 * @param file
 * @return
 */
int
parse(FILE *file);

/**
 * @brief Wrapper for get_token
 *
 * tl;dr Reads a token into data->token
 *
 * spaces are ignored
 * when use_queue_for_read, calls to this function
 * don't use scanner but read from queue as long as
 * the queue is not empty
 *
 *
 */
void
get_next_token();

/**
 * @pre eol token was already read
 * @brief read further eol tokens, until non-eol token is reached
 *
 *        Why use it?
 *
 *        It's a way to skip a sequence of eol tokens in situations
 *        when we don't care about them
 *
 * @return predefined return value, see err.h
 */
int
read_eol(bool check_for_first_eol);

/**
 * check that token is a valid start of expression
 * @return
 */
int
is_expression_start();

/**
 * @brief
 *
 * @return predefined return value, see err.h
 */
int
init_data();

/**
 * @brief cleanup values used by data_t
 */
/*
void
clear_data();
*/

// TODO missing javadoc
int
symtable_insert(token_t *token, bool is_function);

// ---------vvv--------- GRAMMAR RULES FUNCTIONS ---------vvv---------

/**
 * @brief
 *
 * @return predefined return value, see err.h
 */
int
function_def();

/**
 * @brief
 *
 * @return predefined return value, see err.h
 */
int
statement();

/**
 * @brief
 *
 * @return predefined return value, see err.h
 */
int
assign_rhs();

/**
 * @brief
 *
 * @return predefined return value, see err.h
 */
int
statement_list_nonempty();

/**
 * @brief
 *
 * @return predefined return value, see err.h
 */
int
statement_global();

/**
 * @brief
 *
 * @return predefined return value, see err.h
 */
int
statement_list();

/**
 * @brief
 *
 * @return predefined return value, see err.h
 */
int
if_clause();

/**
 * @brief
 *
 * @return predefined return value, see err.h
 */
int
while_clause();

/**
 * @brief
 *
 * @return predefined return value, see err.h
 */
int
def_param_list_next();

/**
 * @brief
 *
 * @return predefined return value, see err.h
 */
int
def_param_list();

/**
 * @brief
 *
 * @return predefined return value, see err.h
 */
int
call_param_list();

/**
 * @brief
 *
 * @return predefined return value, see err.h
 */
int
call_elem();

/**
 * @brief
 *
 * @return predefined return value, see err.h
 */
int
call_param_list_next();

/**
 * @brief
 *
 * @return predefined return value, see err.h
 */
int
return_statement();

/**
 * @brief
 *
 * @return predefined return value, see err.h
 */
int
return_expression();

/**
 * @brief
 *
 * @return predefined return value, see err.h
 */
int
expression();

#endif //IFJ_2019__PARSER_H
