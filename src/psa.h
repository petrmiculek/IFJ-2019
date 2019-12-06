#ifndef HEADER_PSA
#define HEADER_PSA
/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
            xjacko04 (Jacko Daniel)
            xsetin00 (Setinský Jiří)
            xsisma01 (Šišma Vojtěch)
 */
#include "parser.h"
#include "scanner.h"
#include "exp_stack.h"
#include "parser.h"
#include "symtable.h"

unsigned int
get_symbol(token_t *token, unsigned int *sym);

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
    OP_DOC,
    OP_ID,
    OP_NONE,
    DOLAR,
    L_BRAC,
    R_BRAC
} table_symbol;

typedef enum rules
{
    R_I,        //E=i
    R_PLUS,     //E=E+E
    R_MIN,      //E=E-E
    R_MUL,      //E=E*E
    R_DIV,      //E=E/E
    R_IDIV,     //E=E/E
    R_A,        //E=E>E
    R_EA,       //E=E=>E
    R_L,        //E=E<E
    R_EL,       //E=E=<E
    R_EQ,       //E=E==E
    R_NE,       //E=E!=E
    R_BRACKETS  //E=(E)
} rules;

unsigned int
check_semantics(rules rule, sem_t *sym1, int result, sem_t *sym3, d_type *final_type, data_t *data, int *frame);

unsigned int
tmp_var();

unsigned int
solve_exp(data_t *data);
#endif // HEADER_PSA
