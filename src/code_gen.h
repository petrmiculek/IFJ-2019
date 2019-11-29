#ifndef HEADER_CODE_GEN
#define HEADER_CODE_GEN
/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
            xsetin00 (Setinský Jiří)

 */

#include <stdbool.h>
#include "symtable.h"

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
generate_function_param(string_t *identifier, int param_number);
#endif // HEADER_CODE_GEN
