#include "parser.h"
#include "scanner.h"
#include "err.h"
#include "token_queue.h"
#include "exp_stack.h"
#include "psa.h"
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#define TABLE_SIZE 7
#define RETURN_IF_ERR(res) do { if ((res) != RET_OK) {return (res);} } while(0);

typedef enum Prio
{
        S,     // <
        E,     // =
        R,     // >
        B,     // /
        F      // Konec
}Prio;

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
    { R , S , R , S , R , S , R }, // 0
    { R , R , R , S , R , S , R }, // 1
    { R , R , E , S , R , S , R }, // 2
    { R , R , R , B , R , B , R }, // 3
    { S , S , S , S , F , S , B }, // 4
    { S , S , S , S , B , S , R }, // 5
    { R , R , R , B , R , B , R }  // 6
};

static table_index get_table_index(table_symbol sym)
{
    switch (sym)
    {
        case PLUS:
        case MIN:
            return I_Plus;
        case MUL:
        case DIV:
        case IDIV:
            return I_Mul;
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
        case OP_ID:return I_Op;
        case DOLAR:return I_Dolar;
        case L_BRAC:return I_L_Brec;
        case R_BRAC:return I_R_Brec;
        default:break;
    }
    return WARNING_NOT_IMPLEMENTED; // FIXME
}
unsigned int get_symbol(token_t *token)
{
    if(token->type == TOKEN_INT)
    {
        return OP_INT;
    }
    else if(token->type == TOKEN_FLOAT)
    {
        return OP_FLOAT;
    }
    else if(token->type == TOKEN_LIT)
    {
        return OP_STR;
    }
    else if(token->type == TOKEN_PLUS)
    {
        return PLUS;
    }
    else if(token->type == TOKEN_MINUS)
    {
        return MIN;
    }
    else if(token->type == TOKEN_DIVISION)
    {
        return DIV;
    }
    else if(token->type == TOKEN_MULTI)
    {
        return MUL;
    }
    else if(token->type == TOKEN_FLOR_DIV)
    {
        return IDIV;
    }
    else if(token->type == TOKEN_LEFT)
    {
        return L_BRAC;
    }
    else if(token->type == TOKEN_RIGHT)
    {
        return R_BRAC;
    }
    else if(token->type == TOKEN_IS_EQUAL)
    {
        return EQ;
    }
    else if(token->type == TOKEN_N_EQUAL)
    {
        return NE;
    }
    else if(token->type == TOKEN_MORE)
    {
        return A;
    }
    else if(token->type == TOKEN_MORE_E)
    {
        return EA;
    }
    else if(token->type == TOKEN_LESS)
    {
        return L;
    }
    else if(token->type == TOKEN_LESS_E)
    {
        return EL;
    }
    else if(token->type == TOKEN_EOL)
    {
        return DOLAR;
    }
    else if (token->type == TOKEN_COLON)
    {
        return DOLAR;
    }
    else if (token->type == TOKEN_EOF)
    {
        return DOLAR;
    }

    return WARNING_NOT_IMPLEMENTED; // FIXME
}
unsigned int check_semantics(rules rule, sem_t *sym1, sem_t *sym2, sem_t *sym3, d_type* final_type)
{
    bool retype_sym1_to_double = false;
	bool retype_sym3_to_double = false;
	bool retype_sym1_to_integer = false;
	bool retype_sym3_to_integer = false;

	if (rule == R_I)
	{
		//check if the operand is defined probably sym table
        //return RET_SEMANTICAL_ERROR

	}

	if (rule == R_BRACKETS)
	{
		// check sym_table
	}

	if (rule != R_I && rule != R_BRACKETS)
	{
		//check sym_table
	}

	switch (rule)
	{
	case R_I:
        *final_type =sym1->d_type;
		break;

	case R_BRACKETS:
        *final_type = sym2->d_type;
		break;

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

	case R_DIV:
		*final_type = FLOAT;

		if (sym1->d_type == STRING || sym3->d_type == STRING)
			return RET_SEMANTICAL_RUNTIME_ERROR;

		if (sym1->d_type == INT)
			retype_sym1_to_double = true;

		if (sym3->d_type == INT)
			retype_sym3_to_double = true;

		break;

	case R_IDIV:
		*final_type = INT;

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
	case R_A:
		*final_type = INT;

		if (sym1->d_type == INT && sym3->d_type == FLOAT)
			retype_sym1_to_double = true;

		else if (sym1->d_type == FLOAT && sym3->d_type == INT)
			retype_sym3_to_double = true;

		else if (sym1->d_type != sym3->d_type)
			return RET_SEMANTICAL_RUNTIME_ERROR;

		break;

	default:
		break;
	}

	if (retype_sym1_to_double)
	{
		//GENERATE_CODE(generate_stack_sym2_to_douboe);
	}

	if (retype_sym3_to_double)
	{
		//GENERATE_CODE(generate_stack_sym1_to_double);
	}

	if (retype_sym1_to_integer)
	{
		//GENERATE_CODE(generate_stack_sym2_to_inteoer);
	}

	if (retype_sym3_to_integer)
	{
		//GENERATE_CODE(generate_stack_sym1_to_integer);
	}

	return RET_OK;
}

unsigned int get_rule(sym_stack *Stack,int *count, unsigned int *rule)
{
    int i = Stack->top+1;
    sem_t tmp = Stack->atr[i];
    while (tmp.type != SHIFT)
    {
        i--;
        count++;
        tmp = Stack->atr[i];
    }
    if(*count == 1)
    {
        sem_t sym1 = Stack->atr[i];
        if (sym1.type == EXP) // FIXME assign, not comparison; is this intended?
        {
            *rule = R_I;
            return RET_OK;
        }
        else
        {
            return RET_SYNTAX_ERROR;
        }
    }
    else if(*count == 3)
    {
        i = Stack->top+1;
        sem_t sym1 = Stack->atr[i];
        sem_t sym2 = Stack->atr[i-1];
        sem_t sym3 = Stack->atr[i-2];
        if(sym1.type == R_BRAC || sym2.type == EXP || sym3.type == L_BRAC)
        {
            *rule = R_BRACKETS;
            return RET_OK;
        }
        else if(sym1.type == EXP || sym3.type == EXP)
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
    else
    {
        return RET_SYNTAX_ERROR;
    }
    return WARNING_NOT_IMPLEMENTED; // FIXME
}


unsigned int tmp_var(string_t *string, int *tmp1, int *tmp2, int *tmp3)
{
    string_t string;
    init_string(&string);
    if(*tmp1 == 0 && *tmp2 == 0 && *tmp3 == 0)
    {
        char *c = "tmp1";
        if(append_c_string_to_string(&string, c) == RET_INTERNAL_ERROR)
        {
            return RET_INTERNAL_ERROR;
        }
        else
        {
            *tmp1 = 1;
            return RET_OK;
        }
    }
    else if (*tmp1 == 1 && *tmp2 == 0 && *tmp3 == 0)
    {
        char *c = "tmp2";
        if(append_c_string_to_string(&string, c) == RET_INTERNAL_ERROR)
        {
            return RET_INTERNAL_ERROR;
        }
        else
        {
            *tmp2 = 1;
            return RET_OK;
        }     
    }

    else if (*tmp1 ==0  && *tmp2 ==0  && *tmp3 == 1)
    {
        char *c = "tmp1";
        if(append_c_string_to_string(&string, c) == RET_INTERNAL_ERROR)
        {
            return RET_INTERNAL_ERROR;
        }
        else
        {
            *tmp1 = 1;
            return RET_OK;
        }     
    }
    else if (*tmp1 ==0  && *tmp2 ==1  && *tmp3 == 0)
    {
        char *c = "tmp1";
        if(append_c_string_to_string(&string, c) == RET_INTERNAL_ERROR)
        {
            return RET_INTERNAL_ERROR;
        }
        else
        {
            *tmp1 = 1;
            return RET_OK;
        }     
    }
    else if (*tmp1 == 1 && *tmp2 == 1 && *tmp3 == 0)
    {
        char *c = "tmp3";
        if(append_c_string_to_string(&string, c) == RET_INTERNAL_ERROR)
        {
            return RET_INTERNAL_ERROR;
        }
        else
        {
            *tmp3 = 1;
            *tmp1 = 0;
            *tmp2 = 0;
            return RET_OK;
        }     
    }
    else if (*tmp1 == 0 && *tmp2 == 1 && *tmp3 == 1)
    {
        char *c = "tmp1";
        if(append_c_string_to_string(&string, c) == RET_INTERNAL_ERROR)
        {
            return RET_INTERNAL_ERROR;
        }
        else
        {
            *tmp3 = 0;
            *tmp1 = 1;
            *tmp2 = 0;
            return RET_OK;
        }     
    }
    else if (*tmp1 == 1 && *tmp2 == 0 && *tmp3 == 1)
    {
        char *c = "tmp2";
        if(append_c_string_to_string(&string, c) == RET_INTERNAL_ERROR)
        {
            return RET_INTERNAL_ERROR;
        }
        else
        {
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
}
sym_stack* Stack;



unsigned int solve_exp(data_t *data)

{
    Stack = (sym_stack*) malloc(sizeof(sym_stack));

    int res;

    res = 0;

    sem_t new;

    unsigned int finaltype;

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
    while(res != 1)
    {
        get_next_token(data, &res);
        RETURN_IF_ERR(res)

        unsigned int sym = get_symbol(data->token);
        sem_t stack_term = get_term(Stack);
        sem_t sym1;
        sem_t sym2;
        sem_t sym3;
        int i = Stack->top;


        switch(prec_table[get_table_index(stack_term.type)][get_table_index(sym)])
        {
            case S:
            {

                sem_t tmp = Stack->atr[Stack->top];
                if(tmp.type == EXP)
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
                switch(sym)
                {
                    case OP_INT:
                    case OP_STR:
                    case OP_FLOAT:
                    case OP_ID:
                        new.sem_data = data->token->string;
                        break;
                    default:
                        break;
                }
                stack_expr_push(Stack, new);
                get_next_token(data, &res);
                RETURN_IF_ERR(res)

            }
            case E:
            {
                new.type = sym;
                switch(sym)
                {
                    case OP_INT:
                    case OP_STR:
                    case OP_FLOAT:
                    case OP_ID:
                        new.sem_data = data->token->string;
                        break;
                    default:
                        break;
                }
                stack_expr_push(Stack, new);
                get_next_token(data, &res);
                RETURN_IF_ERR(res)
            }
            case R:
            {
                int count;
                unsigned int rule = 0;
                get_rule(Stack, &count, &rule);
                if (rule == R_I)
                {
                    if(check_semantics(rule, &sym1, &sym2, &sym3, &finaltype) != RET_OK)
                        return RET_SEMANTICAL_RUNTIME_ERROR;
                    new.type = EXP;
                    new.d_type = finaltype;
                    if(tmp_var( &new.sem_data, &tmp1_used, &tmp2_used, &tmp3_used) == RET_INTERNAL_ERROR)
                        return RET_INTERNAL_ERROR;
                    stack_expr_pop(Stack);
                    stack_expr_pop(Stack);
                    stack_expr_push(Stack, new);
                }
                else if (rule == R_PLUS || rule == R_MIN || rule == R_MUL || rule == R_DIV || rule == R_IDIV) // FIXME just wanted to pass build
                {
                    sym1 = Stack->atr[i];
                    sym2 = Stack->atr[i-1];
                    sym3 = Stack->atr[i-2];
                    if(check_semantics(rule, &sym1, &sym2, &sym3, &finaltype) != RET_OK)
                        return RET_SEMANTICAL_RUNTIME_ERROR;
                    new.type = EXP;
                    new.d_type = finaltype;
                    if(tmp_var( &new.sem_data, &tmp1_used, &tmp2_used, &tmp3_used) == RET_INTERNAL_ERROR)
                        return RET_INTERNAL_ERROR;
                    stack_expr_pop(Stack);
                    stack_expr_pop(Stack);
                    stack_expr_pop(Stack);
                    stack_expr_pop(Stack);
                    stack_expr_push(Stack, new);
                }
                else if(rule = R_BRACKETS)
                {
                    sem_t new = Stack->atr[i-1];
                    stack_expr_pop(Stack);
                    stack_expr_pop(Stack);
                    stack_expr_pop(Stack);
                    stack_expr_pop(Stack);
                    stack_expr_push(Stack, new);
                }
                else if(rule == R_EA || rule == R_A || rule == R_L || rule == R_EL || rule == R_EQ || rule == R_NE)
                {
                    stack_expr_pop(Stack);
                    stack_expr_pop(Stack);
                    stack_expr_pop(Stack);
                    stack_expr_pop(Stack);
                    stack_expr_push(Stack, new);
                }
            }
            case B:
            {
                return RET_SYNTAX_ERROR;
            }
            case F:
            {
                return RET_OK;
            }

            default:break;
        }
    }
    return WARNING_NOT_IMPLEMENTED; // FIXME
}

