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

// TODO missing javadocs

int
init_code_string();

void
print_code_string();

int
insert_built_in_functions();

int
generate_var_declare(char *var_id);

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
generate_write(sym_table_item *identifier, bool scope);

int
generate_function_call(string_t *identifier);

int
generate_function_param(int param_number, string_t *identifier, bool scope);

int
generate_operand(string_t operand, int tmp, unsigned int symbol);

int
generate_operation(sem_t op1, sem_t op2, int result, unsigned int rule);

int
generate_result(sem_t result);
#endif // HEADER_CODE_GEN
