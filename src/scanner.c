#include "scanner.h"
#include "err.h"
#include "my_string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*######################TODO LIST######################

#######################################################
*/
char *keywords[] = {"def", "else", "if", "None", "pass", "return", "while"};

static stack_t *space_stack = NULL;

token_type
check_keyword(char *str)
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
        return TOKEN_IDENTIFIER;
}

unsigned int
initStack(stack_t *stack)
{
    stack->capacity = STACK_CAPACITY;
    stack->top = 0;
    if ((stack->array = (unsigned int *) malloc(STACK_CAPACITY * sizeof(unsigned int *))) == NULL)
    {
        return RET_INTERNAL_ERROR;
    }
    stack->array[stack->top] = 0;
    return RET_OK;
}


stack_t *
init_stack()
{
    stack_t *stack;
    if (NULL != (stack = malloc(sizeof(stack_t))))
    {
        if (initStack(stack) != RET_OK)
        {
            free(stack);
            stack = NULL;
        }
    }
    return stack;
}


void
free_static_stack()
{
    free(space_stack);
}

unsigned int
push(stack_t *stack, unsigned int item)
{
    if (stack->capacity - 1 == stack->top)
    {
        if ((stack->array = (unsigned int *) realloc(stack->array, sizeof(unsigned int) * STACK_REALLOC)) == NULL)
        {
            return RET_INTERNAL_ERROR;
        }
    }

    stack->array[++stack->top] = item;
    return RET_OK;
}

void
pop(stack_t *stack)
{
    if (stack->top != 0)
        stack->top--;
}

unsigned int
get_token(token_t *token, FILE *file)
{
    if (space_stack == NULL)
    {
        if (NULL == (space_stack = init_stack()))
        {
            return RET_INTERNAL_ERROR;
        }
    }

    if (init_string(&token->string))
    {
        return RET_INTERNAL_ERROR;
    }
    static long long spaces_num = -1;
    unsigned int state = STATE_START;
    int read;

    if (spaces_num >= 0)
    {
        if (space_stack->array[space_stack->top] < spaces_num)
        {
            push(space_stack, (unsigned) spaces_num);
            spaces_num = -1;
            token->type = TOKEN_INDENT;
            return RET_OK;
        }
        else if (space_stack->array[space_stack->top] == spaces_num)
        {
            spaces_num = -1;
        }
        else
        {
            pop(space_stack);
            if (space_stack->array[space_stack->top] < spaces_num)
                RETURN_ERR
            if (space_stack->array[space_stack->top] == spaces_num)
            {
                spaces_num = -1;
            }
            token->type = TOKEN_DEDENT;
            return RET_OK;
        }
    }
    else if (spaces_num == -2)
    {
        if (space_stack->array[space_stack->top] != 0)
        {
            pop(space_stack);
            token->type = TOKEN_DEDENT;
            return RET_OK;
        }
        else
        {
            token->type = TOKEN_END;
            return RET_OK;
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
                else if (read == '\r')
                {
                    break;
                    // TODO gotta handle windows eol \r\n
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
                    return RET_OK;
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
                else if ((read >= 'a' && read <= 'z')
                    || (read >= 'A' && read <= 'Z')
                    || (read == '_'))
                {
                    state = STATE_IDENTIFIER;
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
                    RETURN_ERR;
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
                    return RET_OK;
                }
                else if (read == '-')
                {
                    state = STATE_MINUS;
                    token->type = TOKEN_MINUS;
                    return RET_OK;
                }
                else if (read == '+')
                {
                    state = STATE_PLUS;
                    token->type = TOKEN_PLUS;
                    return RET_OK;
                }
                else if (read == '(')
                {
                    state = STATE_LEFT;
                    token->type = TOKEN_LEFT;
                    return RET_OK;
                }
                else if (read == ')')
                {
                    state = STATE_RIGHT;
                    token->type = TOKEN_RIGHT;
                    return RET_OK;
                }
                else if (read == '*')
                {
                    state = STATE_MULTI;
                    token->type = TOKEN_MULTI;
                    return RET_OK;
                }
                else if (read == ',')
                {
                    state = STATE_COMMA;
                    token->type = TOKEN_COMMA;
                    return RET_OK;
                }
                else if (read == EOF)
                {
                    spaces_num = -2;
                    state = STATE_END;
                    token->type = TOKEN_EOF;
                    return RET_OK;
                }
                else
                {
                    state = STATE_ERROR;
                    RETURN_ERR
                }

            case STATE_EOL:

                spaces_num++;
                if (read == ' ')
                {
                    state = STATE_EOL_SP;
                    break;
                }
                else
                {
                    ungetc(read, file);
                    token->type = TOKEN_EOL;
                    return RET_OK;
                }

            case STATE_EOL_SP:

                spaces_num++;
                if (read == ' ')
                {
                    state = STATE_EOL_SP;
                    break;
                }
                else if (read == '#')
                {
                    spaces_num = -1;
                    state = STATE_COMMENT;
                    break;
                }
                else
                {
                    ungetc(read, file);
                    token->type = TOKEN_EOL;
                    return RET_OK;
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
                    return RET_OK;
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
                    RETURN_ERR
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
                    return RET_OK;
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
                    RETURN_ERR

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
                else
                    RETURN_ERR

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
                    return RET_OK;
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
                    return RET_OK;
                }
                else if (31 < read)
                {
                    state = STATE_LIT;
                    APPEND
                    break;
                }
                else
                    RETURN_ERR

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
                if (('0' <= read && read <= '9')
                    || ('a' <= read && read <= 'f')
                    || ('A' <= read && read <= 'F'))
                {
                    state = STATE_LIT_H1;
                    APPEND
                    break;
                }
                else
                    RETURN_ERR

            case STATE_LIT_H1:
                if (('0' <= read && read <= '9')
                    || ('a' <= read && read <= 'f')
                    || ('A' <= read && read <= 'F'))
                {
                    state = STATE_LIT;
                    APPEND
                    break;
                }
                else
                    RETURN_ERR

            case STATE_BLOCK:
                if (read == '"')
                {
                    state = STATE_BLOCK1;
                    break;
                }
                else
                    RETURN_ERR

            case STATE_BLOCK1:
                if (read == '"')
                {
                    state = STATE_BLOCK2;
                    break;
                }
                else
                    RETURN_ERR

            case STATE_BLOCK2:
                if (read == '"')
                {
                    state = STATE_BLOCK_B;
                    break;
                }
                else if (31 < read
                    || read == '\r'
                    || read == '\n'
                    || read == '\t')
                {
                    state = STATE_BLOCK2;
                    break;
                }
                else
                    RETURN_ERR

            case STATE_BLOCK_B:
                if (read == '"')
                {
                    state = STATE_BLOCK_B1;
                    break;
                }
                else
                    RETURN_ERR
            case STATE_BLOCK_B1:
                if (read == '"')
                {
                    state = STATE_START;
                    break;
                }
                else
                    RETURN_ERR

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

            case STATE_IDENTIFIER:
                if ((read >= 'a' && read <= 'z')
                    || (read >= 'A' && read <= 'Z')
                    || (read == '_')
                    || (read >= '0' && read <= '9'))
                {
                    state = STATE_IDENTIFIER;
                    APPEND
                    break;
                }
                else
                {
                    ungetc(read, file);
                    token->type = check_keyword(token->string.str);
                    return RET_OK;
                }

            case STATE_NEG:
                if (read == '=')
                {
                    token->type = TOKEN_N_EQUAL;
                    return RET_OK;
                }
                else
                    RETURN_ERR

            case STATE_ASSIGN:
                if (read == '=')
                {
                    token->type = TOKEN_IS_EQUAL;
                    return RET_OK;
                }
                else
                {
                    ungetc(read, file);
                    token->type = TOKEN_ASSIGN;
                    return RET_OK;
                }

            case STATE_MORE:
                if (read == '=')
                {
                    token->type = TOKEN_MORE_E;
                    return RET_OK;
                }
                else
                {
                    ungetc(read, file);
                    token->type = TOKEN_MORE;
                    return RET_OK;
                }

            case STATE_LESS:
                if (read == '=')
                {
                    token->type = TOKEN_LESS_E;
                    return RET_OK;
                }
                else
                {
                    ungetc(read, file);
                    token->type = TOKEN_LESS;
                    return RET_OK;
                }

            case STATE_DIVISION:
                if (read == '/')
                {
                    token->type = TOKEN_FLOR_DIV;
                    return RET_OK;
                }
                else
                {
                    ungetc(read, file);
                    token->type = TOKEN_DIVISION;
                    return RET_OK;
                }

            default:

                state = STATE_ERROR;
                fprintf(stderr, "%s, %u: reached default state", __func__, __LINE__);
                break;
                // FIXME default case unreachable code?
        }
    }

}
