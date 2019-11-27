
#ifndef HEADER_EXP_STACK
#define HEADER_EXP_STACK
/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
            xjacko04 (Jacko Daniel)
            xsetin00 (Setinský Jiří)
            xsisma01 (Šišma Vojtěch)
 */

#include <stdio.h>
#include <stdlib.h>
#include "psa.h"
#include "my_string.h"
#include "psa_stack_declarations.h"


unsigned int
init(sym_stack *Stack);

void
stack_expr_pop(sym_stack *Stack);

unsigned int
stack_expr_push(sym_stack *Stack, sem_t sym);

sem_t
get_term(sym_stack *Stack);

#endif // HEADER_EXP_STACK
