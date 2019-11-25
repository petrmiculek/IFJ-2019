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
        case OP_ID:
            return I_Op;
        case DOLAR:
            return I_Dolar;
        case L_BRAC:
            return I_L_Brec;
        case R_BRAC:
            return I_R_Brec;
    }
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
    else if(token->type == TOKEN_COLON)
    {
        return DOLAR;
    }
    else if(token->type == TOKEN_EOF)
    {
        return DOLAR;
    }

    
}

sym_stack* Stack;

unsigned int solve_exp(data_t *data)

{
    Stack = (sym_stack*) malloc(sizeof(sym_stack));
    int res; 
    res = 0;
    sem_t *new = malloc(sizeof(sem_t));
    new->type = DOLAR;
    if(init(Stack) == RET_INTERNAL_ERROR)
    {
        return RET_INTERNAL_ERROR;
    }
    if (push(Stack, new) == RET_INTERNAL_ERROR)
    {
        return RET_INTERNAL_ERROR;
    }
    while(res != 1)
    {
        get_next_token(data, &res);
        RETURN_IF_ERR(res)

        unsigned int sym = get_symbol(data->token);
        sem_t stack_term = get_term(Stack);


        switch(prec_table[get_table_index(stack_term.type)][get_table_index(sym)])
        {   
            case S:
            {
                new->type = SHIFT;
                push(Stack, new);
                new->type = sym;
                switch(sym)
                {
                    case OP_INT:
                    case OP_STR:
                    case OP_FLOAT:
                        new->sem_data = data->token->string;
                        break;
                    default:
                        break;
                }
                push(Stack, new);
                get_next_token(data, &res);
                RETURN_IF_ERR(res)

            }
            case E:
            {
                new->type = sym;
                switch(sym)
                {
                    case OP_INT:
                    case OP_STR:
                    case OP_FLOAT:
                        new->sem_data = data->token->string;
                        break;
                    default:
                        break;
                }
                push(Stack, new);
                get_next_token(data, &res);
                RETURN_IF_ERR(res)
            }
            case R:
            {
                
            }
            case B:
            {

            }
            case F:
            {

            } 
        }
    }
}

