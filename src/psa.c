
/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
            xjacko04 (Jacko Daniel)
            xsetin00 (Setinský Jiří)
            xsisma01 (Šišma Vojtěch)
 */
#include "token_queue.h"
#include "symtable.h"
#include "exp_stack.h"
#include "psa.h"
#include "parser.h"
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "parser.h"
#include "scanner.h"
#include "err.h"
#include "code_gen.h"

#define TABLE_SIZE 7
#define RETURN_IF_ERR(res) do { if ((res) != RET_OK) {return (res);} } while(0);

sym_stack *Stack;

typedef enum Prio
{
    S,     // <
    E,     // =
    R,     // >
    B,     // /
    F      // Konec
} Prio;

typedef enum table_index
{
    I_Plus,     //0  + -
    I_Mul,      //1  * / //
    I_Rel,      //2 == > >= < <= !=
    I_Op,       //3 i
    I_Dolar,    //4 $
    I_L_Brec,   //5 (
    I_R_Brec    //6 )
} table_index;

int prec_table[TABLE_SIZE][TABLE_SIZE] =
    {
//  | 0 | 1 | 2 | 3 | 4 | 5 | 6 |
        {R, S, R, S, R, S, R}, // 0
        {R, R, R, S, R, S, R}, // 1
        {R, R, E, S, R, S, R}, // 2
        {R, R, R, B, R, B, R}, // 3
        {S, S, S, S, F, S, B}, // 4
        {S, S, S, S, B, S, E}, // 5
        {R, R, R, B, R, B, R}  // 6
    };

static table_index
get_table_index(table_symbol sym)
{
    switch (sym)
    {
        case PLUS:
        case MIN:return I_Plus;
            break;
        case MUL:
        case DIV:
        case IDIV:return I_Mul;
            break;
        case A:
        case EA:
        case L:
        case EL:
        case EQ:
        case NE:

            return I_Rel;
        case OP_FLOAT:
        case OP_INT:
        case OP_STR:
        case OP_NONE:
        case OP_DOC:
        case OP_ID:

            return I_Op;
        case DOLAR:

            return I_Dolar;

        case L_BRAC:

            return I_L_Brec;
        case R_BRAC:

            return I_R_Brec;

        default:

            return RET_INTERNAL_ERROR;
    }
}
unsigned int
get_symbol(token_t *token, unsigned int *sym)
{
    if (token->type == TOKEN_INT)
    {
        *sym = OP_INT;
        return RET_OK;
    }
    else if (token->type == TOKEN_FLOAT)
    {
        *sym = OP_FLOAT;
        return RET_OK;
    }
    else if (token->type == TOKEN_IDENTIFIER)
    {
        *sym = OP_ID;
        return RET_OK;

    }
    else if (token->type == TOKEN_LIT)
    {
        *sym = OP_STR;
        return RET_OK;
    }
    else if (token->type == TOKEN_DOC)
    {
        *sym = OP_DOC;
        return RET_OK;
    }
    else if (token->type == TOKEN_NONE)
    {
        *sym = OP_NONE;
        return RET_OK;
    }
    else if (token->type == TOKEN_PLUS)
    {
        *sym = PLUS;
        return RET_OK;

    }
    else if (token->type == TOKEN_MINUS)
    {
        *sym = MIN;
        return RET_OK;
    }
    else if (token->type == TOKEN_DIVISION)
    {
        *sym = DIV;
        return RET_OK;
    }
    else if (token->type == TOKEN_MULTI)
    {
        *sym = MUL;
        return RET_OK;
    }
    else if (token->type == TOKEN_FLOR_DIV)
    {
        *sym = IDIV;
        return RET_OK;
    }
    else if (token->type == TOKEN_LEFT)
    {
        *sym = L_BRAC;
        return RET_OK;
    }
    else if (token->type == TOKEN_RIGHT)
    {
        *sym = R_BRAC;
        return RET_OK;
    }
    else if (token->type == TOKEN_IS_EQUAL)
    {
        *sym = EQ;
        return RET_OK;
    }
    else if (token->type == TOKEN_N_EQUAL)
    {
        *sym = NE;
        return RET_OK;
    }
    else if (token->type == TOKEN_MORE)
    {
        *sym = A;
        return RET_OK;
    }
    else if (token->type == TOKEN_MORE_E)
    {
        *sym = EA;
        return RET_OK;
    }
    else if (token->type == TOKEN_LESS)
    {
        *sym = L;
        return RET_OK;
    }
    else if (token->type == TOKEN_LESS_E)
    {
        *sym = EL;
        return RET_OK;
    }
    else if (token->type == TOKEN_EOL)
    {
        *sym = DOLAR;
        return RET_OK;
    }
    else if (token->type == TOKEN_COLON)
    {
        *sym = DOLAR;
        return RET_OK;
    }
    else if (token->type == TOKEN_EOF)
    {
        *sym = DOLAR;
        return RET_OK;
    }

    return RET_SYNTAX_ERROR;
}
unsigned int
check_semantics(rules rule, sem_t *sym1, sem_t *sym2, sem_t *sym3, d_type *final_type, data_t *data, int *frame)
{
    bool retype_sym1_to_double = false;
    bool retype_sym3_to_double = false;
    bool retype_sym1_to_integer = false;
    bool retype_sym3_to_integer = false;

    ht_item_t *local_search_res;
    ht_item_t *global_search_res;
    int res = RET_OK;

    if (rule == R_I || rule == R_BRACKETS)
    {
        if (sym1->type == OP_ID)
        {
            local_search_res = ht_search(data->local_sym_table, sym1->sem_data.str);
            global_search_res = ht_search(data->global_sym_table, sym1->sem_data.str);

            if (global_search_res != NULL
                && global_search_res->data->is_function == true) // id is defined function
            {
                return RET_SEMANTICAL_ERROR;
            }

            if (data->parser_in_local_scope == local)
            {
                if (local_search_res == NULL && global_search_res == NULL)
                {
                    // it could be global variable
                    // so we add id to global table as not defined

                    data->ID->is_defined = false;
                    data->ID->is_function = false;

                    res = add_to_symtable(&sym1->sem_data, global);
                    RETURN_IF_ERR((res))

                    data->function_ID->data->global_variables[data->function_ID->data->just_index] = sym1->sem_data.str;
                    data->function_ID->data->just_index++;
                    // we have to add variable to function_ID structures
                }
                else if (local_search_res != NULL)
                {
                    *frame = 1;
                }

            }
            else // we are in global scope
            {
                if (global_search_res == NULL)
                {
                    // no found so its SEM ERR
                    return RET_SEMANTICAL_ERROR;
                }
                else if (global_search_res->data->is_defined == false // exist but not defined
                    || global_search_res->data->is_function == true) // id is functionm
                {
                    return RET_SEMANTICAL_ERROR;
                }

            }
            *final_type = UNDEFINED;
        }
        else if (sym1->type == OP_INT)
        {
            *final_type = INT;
        }
        else if (sym1->type == OP_FLOAT)
        {
            *final_type = FLOAT;
        }
        else if (sym1->type == OP_STR)
        {
            *final_type = STRING;
        }
        else
        {
            *final_type = UNDEFINED;
        }
    }

    if (sym1->d_type != UNDEFINED && sym3->d_type != UNDEFINED)
    {
        switch (rule)
        {
            case R_PLUS:
            case R_MIN:
            case R_MUL:
                if (sym1->d_type == STRING && sym3->d_type == STRING && rule == R_PLUS)
                {
                    *final_type = STRING;
                    break;
                }

                if (sym1->d_type == INT && sym3->d_type == INT)
                {
                    *final_type = INT;
                    break;
                }

                if (sym1->d_type == STRING || sym3->d_type == STRING)
                    return RET_SEMANTICAL_RUNTIME_ERROR;

                *final_type = FLOAT;

                if (sym1->d_type == INT)
                    retype_sym1_to_double = true;

                if (sym3->d_type == INT)
                    retype_sym3_to_double = true;

                break;

            case R_DIV: *final_type = FLOAT;

                if (sym1->d_type == STRING || sym3->d_type == STRING)
                    return RET_SEMANTICAL_RUNTIME_ERROR;

                if (sym1->d_type == INT)
                    retype_sym1_to_double = true;

                if (sym3->d_type == INT)
                    retype_sym3_to_double = true;

                break;

            case R_IDIV: *final_type = INT;

                if (sym1->d_type == STRING || sym3->d_type == STRING)
                    return RET_SEMANTICAL_RUNTIME_ERROR;

                if (sym1->d_type == FLOAT)
                    retype_sym1_to_integer = true;

                if (sym3->d_type == FLOAT)
                    retype_sym3_to_integer = true;

                break;

            case R_EQ:
            case R_NE:
            case R_EL:
            case R_L:
            case R_EA:
            case R_A: *final_type = INT;

                if (sym1->d_type == INT && sym3->d_type == FLOAT)
                    retype_sym1_to_double = true;

                else if (sym1->d_type == FLOAT && sym3->d_type == INT)
                    retype_sym3_to_double = true;

                else if (sym1->d_type != sym3->d_type)
                    return RET_SEMANTICAL_RUNTIME_ERROR;

                break;

            default: break;
        }

        if (retype_sym1_to_double)
        {
            //GENERATE_CODE(generate_stack_sym2_to_douboe);
            res = generate_retype(*sym1, 1);
            if (res != RET_OK)
                return res;
        }

        if (retype_sym3_to_double)
        {
            //GENERATE_CODE(generate_stack_sym1_to_double);
            res = generate_retype(*sym3, 1);
            if (res != RET_OK)
                return res;
        }

        if (retype_sym1_to_integer)
        {
            //GENERATE_CODE(generate_stack_sym2_to_inteoer);
            res = generate_retype(*sym1, 0);
            if (res != RET_OK)
                return res;
        }

        if (retype_sym3_to_integer)
        {
            //GENERATE_CODE(generate_stack_sym1_to_integer);
            res = generate_retype(*sym3, 0);
            if (res != RET_OK)
                return res;
        }

    }
    else
    {
        // we need to generate runtime type check
        if ((res=typecheck(sym1, sym3, rule))!= RET_OK)
        {
            return res;
        }

    }

    return RET_OK;
}

unsigned int
get_rule(sym_stack *Stack, int *count, unsigned int *rule)
{
    int i = Stack->top;
    sem_t tmp = Stack->atr[i];
    while (tmp.type != SHIFT)
    {
        i--;
        *count = *count + 1;
        tmp = Stack->atr[i];
    }
    if (*count == 1)
    {
        i = Stack->top;
        sem_t sym1 = Stack->atr[i];
        if (sym1.type == OP_ID || sym1.type == OP_FLOAT || sym1.type == OP_INT || sym1.type == OP_STR
            || sym1.type == OP_DOC || sym1.type == OP_NONE)
        {
            *rule = R_I;
            return RET_OK;
        }
        else
        {
            return RET_SYNTAX_ERROR;
        }
    }
    else if (*count == 3)
    {
        i = Stack->top;
        sem_t sym1 = Stack->atr[i];
        sem_t sym2 = Stack->atr[i - 1];
        sem_t sym3 = Stack->atr[i - 2];
        if (sym1.type == R_BRAC || sym2.type == EXP || sym3.type == L_BRAC)
        {
            *rule = R_BRACKETS;
            return RET_OK;
        }
        else if (sym1.type == EXP || sym3.type == EXP)
        {
            switch (sym2.type)
            {
                case PLUS:
                {
                    *rule = R_PLUS;
                    return RET_OK;
                }
                case MIN:
                {
                    *rule = R_MIN;
                    return RET_OK;
                }
                case MUL:
                {
                    *rule = R_MUL;
                    return RET_OK;
                }
                case DIV:
                {
                    *rule = R_DIV;
                    return RET_OK;
                }
                case IDIV:
                {
                    *rule = R_IDIV;
                    return RET_OK;
                }
                case A:
                {
                    *rule = R_A;
                    return RET_OK;
                }
                case EA:
                {
                    *rule = R_EA;
                    return RET_OK;
                }
                case L:
                {
                    *rule = R_L;
                    return RET_OK;
                }
                case EL:
                {
                    *rule = R_EL;
                    return RET_OK;
                }
                case EQ:
                {
                    *rule = R_EQ;
                    return RET_OK;
                }
                case NE:
                {
                    *rule = R_NE;
                    return RET_OK;
                }
                default:return RET_SYNTAX_ERROR;
            }
        }
    }
    return RET_SYNTAX_ERROR;
}

unsigned int
tmp_var(string_t *string, int *tmp1, int *tmp2, int *tmp3, int *result)
{
    init_string(string);
    if (*tmp1 == 0 && *tmp2 == 0 && *tmp3 == 0)
    {
        char *c = "tmp_op1";
        if (append_c_string_to_string(string, c) == RET_INTERNAL_ERROR)
        {
            return RET_INTERNAL_ERROR;
        }
        else
        {
            *result = 1;
            *tmp1 = 1;

            return RET_OK;
        }
    }
    else if (*tmp1 == 1 && *tmp2 == 0 && *tmp3 == 0)
    {
        char *c = "tmp_op2";
        if (append_c_string_to_string(string, c) == RET_INTERNAL_ERROR)
        {
            return RET_INTERNAL_ERROR;
        }
        else
        {
            *result = 2;
            *tmp2 = 1;

            return RET_OK;
        }
    }

    else if (*tmp1 == 0 && *tmp2 == 0 && *tmp3 == 1)
    {
        char *c = "tmp_op1";
        if (append_c_string_to_string(string, c) == RET_INTERNAL_ERROR)
        {
            return RET_INTERNAL_ERROR;
        }
        else
        {
            *result = 1;
            *tmp1 = 1;

            return RET_OK;
        }
    }
    else if (*tmp1 == 0 && *tmp2 == 1 && *tmp3 == 0)
    {
        char *c = "tmp_op1";
        if (append_c_string_to_string(string, c) == RET_INTERNAL_ERROR)
        {
            return RET_INTERNAL_ERROR;
        }
        else
        {
            *result = 1;
            *tmp1 = 1;

            return RET_OK;
        }
    }
    else if (*tmp1 == 1 && *tmp2 == 1 && *tmp3 == 0)
    {
        char *c = "tmp_op3";
        if (append_c_string_to_string(string, c) == RET_INTERNAL_ERROR)
        {
            return RET_INTERNAL_ERROR;
        }
        else
        {
            *result = 3;
            *tmp3 = 1;
            *tmp1 = 0;
            *tmp2 = 0;

            return RET_OK;
        }
    }
    else if (*tmp1 == 0 && *tmp2 == 1 && *tmp3 == 1)
    {
        char *c = "tmp_op1";
        if (append_c_string_to_string(string, c) == RET_INTERNAL_ERROR)
        {
            return RET_INTERNAL_ERROR;
        }
        else
        {
            *result = 1;
            *tmp3 = 0;
            *tmp1 = 1;
            *tmp2 = 0;
            return RET_OK;
        }
    }
    else if (*tmp1 == 1 && *tmp2 == 0 && *tmp3 == 1)
    {
        char *c = "tmp_op2";
        if (append_c_string_to_string(string, c) == RET_INTERNAL_ERROR)
        {
            return RET_INTERNAL_ERROR;
        }
        else
        {
            *result = 2;
            *tmp3 = 0;
            *tmp1 = 0;
            *tmp2 = 1;
            return RET_OK;
        }
    }
    else if (*tmp1 == 1 && *tmp2 == 1 && *tmp3 == 1)
    {
        printf("wrong algorithm for asinging tmp_var");
        return RET_INTERNAL_ERROR;
    }
    return RET_INTERNAL_ERROR;
}
unsigned int
init_sym(sem_t *sym)
{
    sym->type = 100;
    sym->d_type = 100;
    return init_string(&(sym->sem_data));
}

unsigned int
solve_exp(data_t *data)
{
    Stack = (sym_stack *) calloc(sizeof(sym_stack), 1);

    int res;
    int result = 0;

    res = 0;

    sem_t new;

    unsigned int finaltype = 0;

    int tmp1_used = 0;
    int tmp2_used = 0;
    int tmp3_used = 0;

    new.type = DOLAR;
    if (init(Stack) == RET_INTERNAL_ERROR)
    {
        return RET_INTERNAL_ERROR;
    }
    if (stack_expr_push(Stack, new) == RET_INTERNAL_ERROR)
    {
        return RET_INTERNAL_ERROR;
    }

    get_next_token();
    RETURN_IF_ERR(data->get_token_res)

    sem_t sym1;
    sem_t sym2;
    sem_t sym3;
    if ((res = init_sym(&sym1)) != RET_OK)
        return res;
    if ((res = init_sym(&sym2)) != RET_OK)
        return res;
    if ((res = init_sym(&sym3)) != RET_OK)
        return res;
    while (1)
    {
        int frame = 0;
        unsigned int sym = 0;
        if (get_symbol(data->token, &sym) == RET_SYNTAX_ERROR)
            return RET_SYNTAX_ERROR;
        sem_t stack_term = get_term(Stack);
        int i = Stack->top;

        if (get_table_index(stack_term.type) == 99 || get_table_index(sym) == 99)
            return RET_SYNTAX_ERROR;

        switch (prec_table[get_table_index(stack_term.type)][get_table_index(sym)])
        {
            case S:
            {

                sem_t tmp = Stack->atr[Stack->top];
                if (tmp.type == EXP)
                {
                    stack_expr_pop(Stack);
                    new.type = SHIFT;
                    stack_expr_push(Stack, new);
                    stack_expr_push(Stack, tmp);
                }
                else
                {
                    new.type = SHIFT;
                    stack_expr_push(Stack, new);

                }
                new.type = sym;
                switch (sym)
                {
                    case OP_INT:
                    case OP_STR:
                    case OP_FLOAT:
                    case OP_DOC:
                    case OP_ID:
                    {
                        new.sem_data = data->token->string;
                        break;
                    }
                    case OP_NONE:
                    {
                        string_t none;
                        init_string(&none);
                        char *c = "none";
                        if (append_c_string_to_string(&none, c) == RET_INTERNAL_ERROR)
                            return RET_INTERNAL_ERROR;
                        new.sem_data = none;
                        break;
                    }
                    default:break;
                }
                stack_expr_push(Stack, new);
                get_next_token();
                RETURN_IF_ERR(data->get_token_res)
                break;

            }
            case E:
            {
                new.type = sym;
                switch (sym)
                {
                    case OP_INT:
                    case OP_STR:
                    case OP_FLOAT:
                    case OP_DOC:
                    case OP_ID:
                    {
                        new.sem_data = data->token->string;
                        break;
                    }
                    case OP_NONE:
                    {
                        string_t none;
                        init_string(&none);
                        char *c = "none";
                        if (append_c_string_to_string(&none, c) == RET_INTERNAL_ERROR)
                            return RET_INTERNAL_ERROR;
                        new.sem_data = none;
                        break;
                    }
                    default:break;
                }

                stack_expr_push(Stack, new);
                get_next_token();
                RETURN_IF_ERR(data->get_token_res)
                break;
            }
            case R:
            {
                int count = 0;
                unsigned int rule = 0;
                if (get_rule(Stack, &count, &rule) != RET_OK)
                    return RET_SYNTAX_ERROR;

                if (rule == R_I)
                {
                    sym1 = Stack->atr[i];

                    if ((res = check_semantics(rule, &sym1, &sym2, &sym3, &finaltype, data, &frame)) != RET_OK)
                        return res;

                    new.type = EXP;
                    new.d_type = finaltype;

                    if (tmp_var(&new.sem_data, &tmp1_used, &tmp2_used, &tmp3_used, &result) == RET_INTERNAL_ERROR)
                        return RET_INTERNAL_ERROR;

                    res = generate_operand(sym1.sem_data, result, sym1.type, frame);
                    if (res != RET_OK)
                        return res;

                    stack_expr_pop(Stack);
                    stack_expr_pop(Stack);
                    stack_expr_push(Stack, new);
                }
                else if (rule == R_PLUS || rule == R_MIN || rule == R_MUL || rule == R_DIV
                    || rule == R_IDIV)
                {
                    sym1 = Stack->atr[i];
                    sym2 = Stack->atr[i - 1];
                    sym3 = Stack->atr[i - 2];
                    if (tmp_var(&new.sem_data, &tmp1_used, &tmp2_used, &tmp3_used, &result) == RET_INTERNAL_ERROR)
                        return RET_INTERNAL_ERROR;
                    
                    if ((res = check_semantics(rule, &sym1, &sym2, &sym3, &finaltype, data, &frame)) != RET_OK)
                        return res;

                    new.type = EXP;
                    new.d_type = finaltype;

                    

                    res = generate_operation(sym3, sym1, result, rule);
                    if (res != RET_OK)
                        return res;

                    stack_expr_pop(Stack);
                    stack_expr_pop(Stack);
                    stack_expr_pop(Stack);
                    stack_expr_pop(Stack);
                    stack_expr_push(Stack, new);
                }
                else if (rule == R_BRACKETS)
                {
                    new = Stack->atr[i - 1];

                    if ((res = check_semantics(rule, &new, &sym2, &sym3, &finaltype, data, &frame)) != RET_OK)
                        return res;

                    stack_expr_pop(Stack);
                    stack_expr_pop(Stack);
                    stack_expr_pop(Stack);
                    stack_expr_pop(Stack);
                    stack_expr_push(Stack, new);
                }
                else if (rule == R_EA || rule == R_A || rule == R_L || rule == R_EL || rule == R_EQ || rule == R_NE)
                {
                    sym1 = Stack->atr[i];
                    sym2 = Stack->atr[i - 1];
                    sym3 = Stack->atr[i - 2];

                    if ((res = check_semantics(rule, &sym1, &sym2, &sym3, &finaltype, data, &frame)) != RET_OK)
                        return res;

                    new.type = EXP;
                    new.d_type = finaltype;

                    if (tmp_var(&new.sem_data, &tmp1_used, &tmp2_used, &tmp3_used, &result) == RET_INTERNAL_ERROR)
                        return RET_INTERNAL_ERROR;

                    generate_relop(sym1, sym3, result, rule);
                    stack_expr_pop(Stack);
                    stack_expr_pop(Stack);
                    stack_expr_pop(Stack);
                    stack_expr_pop(Stack);
                    stack_expr_push(Stack, new);
                }
                break;
            }
            case B:
            {
                return RET_SYNTAX_ERROR;
            }
            case F:
            {
                i = Stack->top;
                sym1 = Stack->atr[i];

                if (sym1.type != EXP)
                    return RET_SYNTAX_ERROR;

                q_enqueue(data->token, data->token_queue);
                data->use_queue_for_read = true;
                res = generate_result(sym1);
                if (res != RET_OK)
                    return res;
                return RET_OK;
            }

            default:break;
        }
    }
    return RET_SYNTAX_ERROR;
}

