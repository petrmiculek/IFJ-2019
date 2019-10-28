#include "scanner.h"
#include "err.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*######################TODO LIST######################
-zadanie 4.2 posledny odsek o viacriadkovom koment.
-scanner.h wtf do
-otzka moze byt hned prvy prikaz s odsadenim?
-na konci sa dogeneruju vsetky zvysne tokeny dedent, takze 
-state space
-string by s nemusel stále alokovať a uvoľnovať, ale používal by sa stálen ten istý s tým,
že by sa po každom použití priradila '/0' na pozíciu 0 
#######################################################
*/
char *keywords[] = {"def", "else", "if", "None", "pass", "return", "while"};

token_type
chech_keyword(char *str)
{
    if (!strcmp(str, keywords[0]))
        return TOKEN_DEF;
    else if (!strcmp(str, keywords[1]))
        return TOKEN_ELSE;
    else if (!strcmp(str, keywords[2]))
        return TOKEN_IF;
    else if (!strcmp(str, keywords[3]))
        return TOKEN_NONE;
    else if (!strcmp(str, keywords[4]))
        return TOKEN_PASS;
    else if (!strcmp(str, keywords[5]))
        return TOKEN_RETURN;
    else if (!strcmp(str, keywords[6]))
        return TOKEN_WHILE;
    else
        return TOKEN_IDENT;
}

unsigned int
initStack(stack_t *stack)
{
    stack->capacity = STACK_CAPACITY;
    stack->top = 0;
    if ((stack->array = (unsigned int *) malloc(STACK_CAPACITY * sizeof(unsigned int *))) == NULL)
    {
        return INTERNAL_ERROR;
    }
    stack->array[stack->top] == 0;
    return OK;
}

void
free_stack(stack_t *stack)
{
    free(stack->array);
}
unsigned int
push(stack_t *stack, unsigned int item)
{
    if (stack->capacity - 1 == stack->top)
    {
        if ((stack->array = (int *) realloc(stack->array, sizeof(unsigned int) * STACK_REALLOC)) == NULL)
        {
            return INTERNAL_ERROR;
        }
    }
    stack->array[++stack->top] = item;
    return OK;
}

void
pull(stack_t *stack)
{
    if (stack->top != 0)
        stack->top--;
}

unsigned int
init_string(string_t *string)
{
    if ((string->str = (char *) malloc(sizeof(char) * INITIAL_SIZE)) == NULL)
    {
        return INTERNAL_ERROR;
    }
    string->size = INITIAL_SIZE;
    string->length = 0;
    return OK;
}

unsigned int
append_string(string_t *string, char var)
{
    if (string->length == string->size)
    {

        if ((string->str = (char *) realloc(string->str, sizeof(char) * string->length + REALLOC_SIZE)) == NULL)
        {
            return INTERNAL_ERROR;
        }
        string->size += REALLOC_SIZE;
    }
    strncat(string->str, &var, 1);
    strncat(string->str, STRING_END, 1);
    string->length++;
    //string->str[string->length]= STRING_END;
    return OK;

}

void
free_string(string_t *string)
{
    if (string != NULL)
    {
        string->size = 0;
        string->length = 0;
        free(string->str);
    }
}

unsigned int
get_token(token_t *token, FILE *file, stack_t *stack)
{
    if (init_string(&token->string))
    {
        return INTERNAL_ERROR;
    }
    static int spaces_num = -1;
    unsigned int state = STATE_START;
    int read;

    //printf("getTOken\n");
    if (spaces_num >= 0)
    {
        //printf("%d\n",spaces_num);
        if (stack->array[stack->top] < spaces_num)
        {
            //printf("---1---: %d \n",stack->array[stack->top]);
            push(stack, (unsigned) spaces_num);
            spaces_num = -1;
            token->type = TOKEN_INDENT;
            return OK;
        }
        else if (stack->array[stack->top] == spaces_num)
        {
            //printf("---2---: %d \n",stack->array[stack->top]);
            spaces_num = -1;
        }
        else
        {
            //printf("---3---: %d \n",stack->array[stack->top]);
            pull(stack);
            if (stack->array[stack->top] < spaces_num)
                RET_ERR
            if (stack->array[stack->top] == spaces_num)
            {
                spaces_num = -1;
            }
            token->type = TOKEN_DEDENT;
            return OK;
        }
    }
    else if (spaces_num == -2)
    {
        //printf("%d\n",stack->array[stack->top]);
        //printf("%d\n",spaces_num);
        if (stack->array[stack->top] != 0)
        {
            pull(stack);
            token->type = TOKEN_DEDENT;
            return OK;
        }
        else
        {
            token->type = TOKEN_END;
            return OK;
        }

    }

    while (1)
    {

        read = fgetc(file);

        switch (state)
        {
            case STATE_START:

                if ('1' <= read && read <= '9')
                {
                    state = STATE_INT;
                    APPEND
                    break;
                }
                else if (read == '\n')
                {
                    state = STATE_EOL;
                    break;
                }
                else if (read == '\'')
                {
                    state = STATE_LIT;
                    break;
                }
                else if (read == ' ')
                {
                    state = STATE_SPACE;
                    token->type = TOKEN_SPACE;
                    return OK;
                }
                else if (read == '"')
                {
                    state = STATE_BLOCK;
                    break;
                }
                else if (read == '#')
                {
                    state = STATE_COMMENT;
                    break;
                }
                else if ((read >= 'a' && read <= 'z') || (read >= 'A' && read <= 'Z'))
                {

                    state = STATE_IDENT;
                    APPEND

                    break;
                }
                else if (read == '!')
                {
                    state = STATE_NEG;
                    break;
                }
                else if (read == '0')
                {
                    RET_ERR;
                }
                else if (read == '=')
                {
                    state = STATE_ASSIGN;
                    break;
                }
                else if (read == '>')
                {
                    state = STATE_MORE;
                    token->type = TOKEN_LESS;
                    break;
                }
                else if (read == '<')
                {
                    state = STATE_LESS;
                    token->type = TOKEN_LESS;
                    break;
                }
                else if (read == '/')
                {
                    state = STATE_DIVISION;
                    break;
                }
                else if (read == ':')
                {
                    state = STATE_COLON;
                    token->type = TOKEN_COLON;
                    return OK;
                }
                else if (read == '-')
                {
                    state = STATE_MINUS;
                    token->type = TOKEN_MINUS;
                    return OK;
                }
                else if (read == '+')
                {
                    state = STATE_PLUS;
                    token->type = TOKEN_PLUS;
                    return OK;
                }
                else if (read == '(')
                {
                    state = STATE_LEFT;
                    token->type = TOKEN_LEFT;
                    return OK;
                }
                else if (read == ')')
                {
                    state = STATE_RIGHT;
                    token->type = TOKEN_RIGHT;
                    return OK;
                }
                else if (read == EOF)
                {
                    spaces_num = -2;
                    state = STATE_END;
                    token->type = TOKEN_EOF;
                    return OK;
                }
                else if (read == '*')
                {
                    state = STATE_MULTI;
                    token->type = TOKEN_MULTI;
                    return OK;
                }
                else if (read == ',')
                {
                    state = STATE_COMMA;
                    token->type = TOKEN_COMMA;
                    return OK;
                }
                else
                {
                    state = STATE_ERROR;
                    RET_ERR
                }

            case STATE_EOL:spaces_num++;
                if (read == ' ')
                {
                    state = STATE_EOL_SP;
                    break;
                }
                else
                {
                    ungetc(read, file);
                    token->type = TOKEN_EOL;
                    return OK;
                }
            case STATE_EOL_SP:spaces_num++;
                if (read == ' ')
                {
                    state = STATE_EOL_SP;
                    break;
                }
                else if (read == '#')
                {
                    spaces_num == -1;
                    state = STATE_COMMENT;
                    break;
                }
                else
                {
                    ungetc(read, file);
                    token->type = TOKEN_EOL;
                    return OK;
                }

            case STATE_INT:
                if (read == '.')
                {
                    state = STATE_FLOAT;
                    APPEND
                    break;
                }
                else if ('0' <= read && read <= '9')
                {
                    state = STATE_INT;
                    APPEND
                    break;
                }
                else if (read == 'e' || read == 'E')
                {
                    state = STATE_FLOAT_E;
                    APPEND
                    break;
                }
                else
                {
                    token->type = TOKEN_INT;
                    ungetc(read, file);
                    return OK;
                }

            case STATE_FLOAT:
                if ('0' <= read && read <= '9')
                {
                    state = STATE_FLOAT_D;
                    APPEND
                    break;
                }
                else
                {
                    state = STATE_ERROR;
                    RET_ERR
                }
            case STATE_FLOAT_D:
                if ('0' <= read && read <= '9')
                {
                    state = STATE_FLOAT_D;
                    APPEND
                    break;
                }
                else if (read == 'e' || read == 'E')
                {
                    state = STATE_FLOAT_E;
                    APPEND
                    break;
                }
                else
                {
                    token->type = TOKEN_FLOAT;
                    ungetc(read, file);
                    return OK;
                }
            case STATE_FLOAT_E:
                if ('1' <= read && read <= '9')
                {
                    state = STATE_FLOAT_S;
                    APPEND
                    break;
                }
                else if (read == '0')
                {
                    state = STATE_FLOAT_Z;
                    break;
                }
                else if (read == '+' || read == '-')
                {
                    state = STATE_FLOAT_Z;
                    APPEND
                    break;
                }
                else
                    RET_ERR

            case STATE_FLOAT_Z:
                if ('1' <= read && read <= '9')
                {
                    state = STATE_FLOAT_S;
                    APPEND
                    break;
                }
                else if (read == '0')
                {
                    state = STATE_FLOAT_Z;
                    break;
                }

            case STATE_FLOAT_S:
                if ('0' <= read && read <= '9')
                {
                    state = STATE_FLOAT_S;
                    APPEND
                    break;
                }
                else
                {
                    token->type = TOKEN_FLOAT;
                    ungetc(read, file);
                    return OK;
                }

            case STATE_LIT:
                if (read == '\\')
                {
                    state = STATE_LIT_B;
                    APPEND
                    break;
                }
                else if (read == '\'')
                {
                    token->type = TOKEN_LIT;
                    return OK;
                }
                else if (31 < read)
                {
                    state = STATE_LIT;
                    APPEND
                    break;
                }
                else
                    RET_ERR
            case STATE_LIT_B:
                if (read == 'x')
                {
                    state = STATE_LIT_H;
                    APPEND
                    break;
                }
                else
                {
                    state = STATE_LIT;
                    APPEND
                    break;
                }
            case STATE_LIT_H:
                if (('0' <= read && read <= '9') || ('a' <= read && read <= 'f') || ('A' <= read && read <= 'F'))
                {
                    state = STATE_LIT_H1;
                    APPEND
                    break;
                }
                else
                    RET_ERR
            case STATE_LIT_H1:
                if (('0' <= read && read <= '9') || ('a' <= read && read <= 'f') || ('A' <= read && read <= 'F'))
                {
                    state = STATE_LIT;
                    APPEND
                    break;
                }
                else
                    RET_ERR
            case STATE_BLOCK:
                if (read == '"')
                {
                    state = STATE_BLOCK1;
                    break;
                }
                else
                    RET_ERR
            case STATE_BLOCK1:
                if (read == '"')
                {
                    state = STATE_BLOCK2;
                    break;
                }
                else
                    RET_ERR
            case STATE_BLOCK2:
                if (read == '"')
                {
                    state = STATE_BLOCK_B;
                    break;
                }
                else if (31 < read)
                {
                    state = STATE_BLOCK2;
                }
                else
                    RET_ERR
            case STATE_BLOCK_B:
                if (read == '"')
                {
                    state = STATE_BLOCK1;
                    break;
                }
                else
                    RET_ERR
            case STATE_BLOCK_B1:
                if (read == '"')
                {
                    state = STATE_START;
                    break;
                }
                else
                    RET_ERR
            case STATE_COMMENT:
                if (read == '\n')
                {
                    state = STATE_START;
                    break;
                }
                else
                {
                    state = STATE_COMMENT;
                    break;
                }
            case STATE_IDENT:
                if ((read >= 'a' && read <= 'z') || (read >= 'A' && read <= 'Z') || (read == '-') ||
                    (read >= '0' && read <= '9'))
                {
                    state = STATE_IDENT;
                    APPEND
                    break;
                }
                else
                {
                    ungetc(read, file);
                    token->type = chech_keyword(token->string.str);
                    return OK;
                }
            case STATE_NEG:
                if (read == '=')
                {
                    token->type = TOKEN_N_EQUAL;
                    return OK;
                }
                else
                    RET_ERR
            case STATE_ASSIGN:
                if (read == '=')
                {
                    token->type = TOKEN_IS_EQUAL;
                    return OK;
                }
                else
                {
                    ungetc(read, file);
                    token->type = TOKEN_ASSIG;
                    return OK;
                }
            case STATE_MORE:
                if (read == '=')
                {
                    token->type = TOKEN_MORE_E;
                    return OK;
                }
                else
                {
                    ungetc(read, file);
                    token->type = TOKEN_MORE;
                    return OK;
                }
            case STATE_LESS:
                if (read == '=')
                {
                    token->type = TOKEN_LESS_E;
                    return OK;
                }
                else
                {
                    ungetc(read, file);
                    token->type = TOKEN_LESS;
                    return OK;
                }
            case STATE_DIVISION:
                if (read == '/')
                {
                    token->type = TOKEN_FLOR_DIV;
                    return OK;
                }
                else
                {
                    ungetc(read, file);
                    token->type = TOKEN_DIVISION;
                    return OK;
                }
            default: state = STATE_ERROR;
                break;
        }
    }

}