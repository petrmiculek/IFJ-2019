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
#include "parser.h"

unsigned int
get_symbol(token_t *token);


#define EXP 42
#define SHIFT 21

typedef enum
{
    PLUS,
    MIN,
    MUL,
    DIV,
    IDIV,
    A,
    EA,
    L,
    EL,
    EQ,
    NE,
    OP_INT,
    OP_FLOAT,
    OP_STR,
    OP_ID,
    DOLAR,
    L_BRAC,
    R_BRAC
}table_symbol;

typedef enum rules
{
    R_I,
    R_PLUS,
    R_MIN,
    R_MUL,
    R_DIV,
    R_IDIV,
    R_A,
    R_EA,
    R_L,
    R_EL,
    R_EQ,
    R_NE,
    R_BRACKETS
} rules;

unsigned int
check_semantics(rules rule, sem_t *sym1, sem_t *sym2, sem_t *sym3, d_type *final_type);

unsigned int
tmp_var();

unsigned int
solve_exp(data_t *data);
#endif // HEADER_PSA
