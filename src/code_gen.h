#ifndef HEADER_CODE_GEN
#define HEADER_CODE_GEN
/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
            xjacko04 (Jacko Daniel)
            xsetin00 (Setinský Jiří)
            xsisma01 (Šišma Vojtěch)
 */

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

// TODO missing javadocs
int
generate_unique_number();

string_t *
generate_unique_identifier(const char *prefix_scope, const char *prefix_type);

#endif // HEADER_CODE_GEN
