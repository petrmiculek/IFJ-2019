#ifndef HEADER_CODE_GEN
#define HEADER_CODE_GEN
/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
            xsetin00 (Setinský Jiří)

 */

#include <stdbool.h>
#include "symtable.h"
#include "exp_stack.h"
#include "parser.h"

// TODO missing javadocs

int
init_code_string();

void
print_code_string();

int
insert_built_in_functions();

int
insert_convert_to_bool_function();

int
generate_var_declare(char *var_id, bool is_scope_local);

int
generate_file_header();

int
generate_main_scope_start();

int
generate_main_scope_end();

int
generate_function_start(char *function_id);

int
generate_function_end(char *function_id);

int
generate_unique_number();

string_t *
generate_unique_identifier(const char *prefix_scope, const char *prefix_type);

int
generate_write(token_t *token, data_t *data);

int
generate_function_call(string_t *identifier);

int
generate_function_param(int param_number, string_t *identifier, bool scope);

int
generate_operand(string_t operand, int tmp, unsigned int symbol, data_t *data);

int
generate_result(sem_t result);

int
generate_retype(sem_t op, int to);

int
typecheck(sem_t *op1, sem_t *op2, unsigned int rule, int result);

int
defvar_type(sem_t *op);

int
compiler_ret_value_comment(int retval);

int
generate_move_exp_result_to_variable(token_t *token, data_t *data);

int
generate_if_begin(char *label);

int
generate_if_else(char *label);

int
generate_if_end(char *label);

int
generate_while_label(char *label);

int
generate_while_begin(char *label);

int
generate_while_end(char *label);

int
generate_var_declare_while(char *var_id, char *label, int counter);

#endif // HEADER_CODE_GEN

