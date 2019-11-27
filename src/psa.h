#ifndef HEADER_PSA
#define HEADER_PSA
/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
            xjacko04 (Jacko Daniel)
            xsetin00 (Setinský Jiří)
            xsisma01 (Šišma Vojtěch)
 */

#include "scanner.h"
#include "exp_stack.h"
#include "psa_stack_declarations.h"

unsigned int
get_symbol(token_t *token);

unsigned int
check_semantics(rules rule, sem_t *sym1, sem_t *sym2, sem_t *sym3, d_type *final_type);

#define EXP 42
#define SHIFT 21



unsigned int tmp_var();
#endif // HEADER_PSA
