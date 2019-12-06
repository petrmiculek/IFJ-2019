/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
            xjacko04 (Jacko Daniel)
            xsetin00 (Setinský Jiří)
            xsisma01 (Šišma Vojtěch)
 */
#include "scanner.h"
#include "err.h"
#include "my_string.h"
#include "stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

/*######################TODO LIST######################
 * Otazky:
 * Co ak pride v literaly '\' a za nim bude nasledovat space
 *
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

// this can't go to stack.c/stack.h, as the variable it interacts with is static
void
free_static_stack()
{
    free(space_stack->array);
    free(space_stack);
}

int
convert_char_to_hexcode(int read, token_t *token)
{
    char c[5];
    sprintf(c, "\\0%.2d", read);

    for (int i = 0; i < 4; ++i)
    {
        read = c[i];
        APPEND(read)
    }
    return RET_OK;
}

int
generate_dedent(long long *spaces_num, token_t *token, int *previous_was_eol)
{
    if (*spaces_num >= 0)
    {
        if (space_stack->array[space_stack->top] < *spaces_num)
        {
            stack_push(space_stack, (unsigned) *spaces_num);
            *spaces_num = -1;
            token->type = TOKEN_INDENT;
            return RET_OK;
        }
        else if (space_stack->array[space_stack->top] == *spaces_num)
        {
            *spaces_num = -1;
        }
        else
        {
            stack_pop(space_stack);
            if (space_stack->array[space_stack->top] < *spaces_num)
                return RET_LEXICAL_ERROR;
            if (space_stack->array[space_stack->top] == *spaces_num)
            {
                *spaces_num = -1;
            }
            token->type = TOKEN_DEDENT;
            return RET_OK;
        }
    }
    else if (*spaces_num == -2)
    {
        if (!*previous_was_eol)
        {
            *previous_was_eol = 2;
            token->type = TOKEN_EOL;
            return RET_OK;
        }
        if (space_stack->array[space_stack->top] != 0)
        {
            stack_pop(space_stack);
            token->type = TOKEN_DEDENT;
            return RET_OK;
        }
        else
        {
            token->type = TOKEN_EOF;
            return RET_OK;
        }

    }
    return 2;
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

    if (token != NULL)
    {
        if (init_string(&token->string) != RET_OK)
        {
            return RET_INTERNAL_ERROR;
        }
    }
    else
    {
        return RET_INTERNAL_ERROR;
    }

    static long long spaces_num = -1;
    static int previous_was_eol = 0;
    unsigned int state = STATE_START;
    int read, flag_block_es = 0;
    char var[2];
    static int first_call = 1;

    int check_dedent = generate_dedent(&spaces_num, token, &previous_was_eol);
    if (check_dedent == 1)
    {
        RETURN_ERR
    }
    else if (check_dedent == 0)
    {

        return RET_OK;
    }

    while (1)
    {
        read = fgetc(file);
        if (previous_was_eol)
        {
            --previous_was_eol;
        }
        switch (state)
        {
            case STATE_START:

                if (first_call == 2)
                {
                    first_call = 0;
                    if (read == ' ')
                    {
                        first_call = 2;
                    }
                    else if (read == '#')
                    {
                        state = STATE_COMMENT;
                    }
                    else if (read == '"')
                    {
                        state = STATE_BLOCK1;
                    }
                    else if (read == EOF)
                    {
                        token->type = TOKEN_EOF;
                        return RET_OK;
                    }
                    else
                    {
                        return RET_SYNTAX_ERROR;
                    }
                    break;
                }
                if (first_call == 1)
                {
                    first_call = 0;
                    if (read == ' ')
                    {
                        first_call = 2;
                        break;
                    }
                }

                if ('1' <= read && read <= '9')
                {
                    state = STATE_INT;
                    APPEND(read)
                    break;
                }
                else if (read == '\r')
                {
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
                    token->type = TOKEN_SPACE;
                    return RET_OK;
                }
                else if (read == '"')
                {
                    state = STATE_BLOCK1;
                    break;
                }
                else if (read == '#')
                {
                    state = STATE_COMMENT;
                    break;
                }
                else if (read == '0')
                {
                    state = STATE_INT_Z;
                    break;
                }
                else if ((read >= 'a' && read <= 'z')
                    || (read >= 'A' && read <= 'Z')
                    || (read == '_'))
                {
                    state = STATE_IDENTIFIER;
                    APPEND(read)

                    break;
                }
                else if (read == '!')
                {
                    state = STATE_NEG;
                    break;
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
                    token->type = TOKEN_COLON;
                    return RET_OK;
                }
                else if (read == '-')
                {
                    token->type = TOKEN_MINUS;
                    return RET_OK;
                }
                else if (read == '+')
                {
                    token->type = TOKEN_PLUS;
                    return RET_OK;
                }
                else if (read == '(')
                {
                    token->type = TOKEN_LEFT;
                    return RET_OK;
                }
                else if (read == ')')
                {
                    token->type = TOKEN_RIGHT;
                    return RET_OK;
                }
                else if (read == '*')
                {
                    token->type = TOKEN_MULTI;
                    return RET_OK;
                }
                else if (read == ',')
                {
                    token->type = TOKEN_COMMA;
                    return RET_OK;
                }
                else if (read == EOF)
                {
                    spaces_num = -2;
                    check_dedent = generate_dedent(&spaces_num, token, &previous_was_eol);
                    if (check_dedent == 1)
                    {
                        RETURN_ERR
                    }
                    else if (check_dedent == 0)
                    {

                        return RET_OK;
                    }
                    break;
                }
                else
                    RETURN_ERR
            case STATE_INT_Z:
                if (read == 'b' || read == 'B')
                {
                    state = STATE_BASE_B;
                }
                else if (read == 'o' || read == 'O')
                {
                    state = STATE_BASE_O;
                }
                else if (read == '.')
                {
                    APPEND('0')
                    APPEND(read)
                    state = STATE_FLOAT;
                }
                else if (read == 'x' || read == 'X')
                {
                    state = STATE_BASE_X;
                }
                else if('0' <= read && read <= '9')
                {
                    RETURN_ERR
                }
                else
                {
                    APPEND('0')
                    token->type = TOKEN_INT;
                    ungetc(read, file);
                    return RET_OK;
                }

                break;
            case STATE_BASE_B:
                if ('0' == read || read == '1')
                {
                    APPEND(read)
                    state = STATE_BASE_B;
                    break;
                }
                else if (read == '_')
                {
                    if(!token->string.length)
                        RETURN_ERR
                    state = STATE_BASE_B_U;
                    break;
                }
                else
                {
                    if(token->string.length == 0)
                        RETURN_ERR
                    sprintf(token->string.str, "%ld", strtol(token->string.str, NULL, 2));
                    append_c_string_to_string(&token->string, token->string.str);
                    ungetc(read, file);
                    token->type = TOKEN_INT;
                    return RET_OK;
                }
            case STATE_BASE_O:
                if ('0' <= read && read <= '7')
                {
                    APPEND(read)
                    state = STATE_BASE_O;
                    break;
                }
                else if (read == '_')
                {
                    if(!token->string.length)
                        RETURN_ERR
                    state = STATE_BASE_O_U;
                    break;
                }
                else
                {
                    if(!token->string.length)
                        RETURN_ERR
                    sprintf(token->string.str, "%ld", strtol(token->string.str, NULL, 8));
                    append_c_string_to_string(&token->string, token->string.str);
                    ungetc(read, file);
                    token->type = TOKEN_INT;
                    return RET_OK;
                }
            case STATE_BASE_X:
                if (('0' <= read && read <= '9')
                    || ('a' <= read && read <= 'f')
                    || ('A' <= read && read <= 'F'))
                {
                    APPEND(read)
                    state = STATE_BASE_X;
                    break;
                }
                else if (read == '_')
                {
                    if(!token->string.length)
                        RETURN_ERR
                    state = STATE_BASE_X_U;
                    break;
                }
                else
                {
                    if(token->string.length == 0)
                        RETURN_ERR
                    sprintf(token->string.str, "%ld", strtol(token->string.str, NULL, 16));
                    append_c_string_to_string(&token->string, token->string.str);
                    ungetc(read, file);
                    token->type = TOKEN_INT;
                    return RET_OK;
                }
            case STATE_BASE_X_U:
                if (('0' <= read && read <= '9')
                    || ('a' <= read && read <= 'f')
                    || ('A' <= read && read <= 'F'))
                {
                    APPEND(read)
                    state = STATE_BASE_X;
                    break;
                }
                else
                {
                    RETURN_ERR
                }
            case STATE_BASE_O_U:
                if ('0' <= read && read <= '7')
                {
                    APPEND(read)
                    state = STATE_BASE_O;
                    break;
                }
                else
                {
                    RETURN_ERR
                }
            case STATE_BASE_B_U:
                if ('0' == read || read == '1')
                {
                    APPEND(read)
                    state = STATE_BASE_B;
                    break;
                }
                else
                {
                    RETURN_ERR
                }
            case STATE_EOL:

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
                else if (read == '"')
                {
                    spaces_num = -1;
                    state = STATE_BLOCK1;
                    break;
                }
                else
                {
                    if (read < 33)
                    {
                        spaces_num = -1;
                    }
                    ungetc(read, file);
                    token->type = TOKEN_EOL;
                    previous_was_eol = 2;
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
                else if (read == '"')
                {
                    spaces_num = -1;
                    state = STATE_BLOCK1;
                    break;
                }
                else
                {
                    if (read < 33)
                    {
                        spaces_num = -1;
                    }
                    ungetc(read, file);
                    token->type = TOKEN_EOL;
                    previous_was_eol = 2;
                    return RET_OK;
                }
            case STATE_FLOAT_S_U:
                if ('0' <= read && read <= '9')
                {
                    APPEND(read)
                    state = STATE_FLOAT_S;
                    break;
                }
                else
                {
                    RETURN_ERR
                }
            case STATE_FLOAT_D_U:
                if ('0' <= read && read <= '9')
                {
                    APPEND(read)
                    state = STATE_FLOAT_D;
                    break;
                }
                else
                {
                    RETURN_ERR
                }
            case STATE_INT_U:
                if ('0' <= read && read <= '9')
                {
                    APPEND(read)
                    state = STATE_INT;
                    break;
                }
                else
                {
                    RETURN_ERR
                }
            case STATE_INT:
                if (read == '.')
                {
                    state = STATE_FLOAT;
                    APPEND(read)
                    break;
                }
                else if (read == '_')
                {
                    state = STATE_INT_U;
                    break;
                }
                else if ('0' <= read && read <= '9')
                {
                    state = STATE_INT;
                    APPEND(read)
                    break;
                }
                else if (read == 'e' || read == 'E')
                {
                    state = STATE_FLOAT_E;
                    APPEND(read)
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
                    APPEND(read)
                    break;
                }
                else
                {
                    RETURN_ERR
                }
            case STATE_FLOAT_D:
                if ('0' <= read && read <= '9')
                {
                    state = STATE_FLOAT_D;
                    APPEND(read)
                    break;
                }
                else if (read == 'e' || read == 'E')
                {
                    state = STATE_FLOAT_E;
                    APPEND(read)
                    break;
                }
                else if (read == '_')
                {
                    state = STATE_FLOAT_S_U;
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
                    APPEND(read)
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
                    APPEND(read)
                    break;
                }
                else
                    RETURN_ERR
            case STATE_FLOAT_Z_U:
                if ('1' <= read && read <= '9')
                {
                    state = STATE_FLOAT_S;
                    APPEND(read)
                    break;
                }
                else if (read == '0')
                {
                    state = STATE_FLOAT_Z;
                    break;
                }
                else
                {
                    RETURN_ERR
                }
            case STATE_FLOAT_Z:
                if ('1' <= read && read <= '9')
                {
                    state = STATE_FLOAT_S;
                    APPEND(read)
                    break;
                }
                else if (read == '0')
                {
                    state = STATE_FLOAT_Z;
                    break;
                }
                else if (read == '_')
                {
                    state = STATE_FLOAT_S_U;
                    break;
                }
                else
                    RETURN_ERR
            case STATE_FLOAT_S:
                if ('0' <= read && read <= '9')
                {
                    state = STATE_FLOAT_S;
                    APPEND(read)
                    break;
                }
                else if (read == '_')
                {
                    state = STATE_FLOAT_S_U;
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
                    break;
                }
                else if (read == '\'')
                {
                    token->type = TOKEN_LIT;
                    return RET_OK;
                }
                else if (31 < read)
                {
                    if (read == ' ' || read == '#')
                    {
                        APPEND_HEXCODE(read)
                    }
                    else
                    {
                        APPEND(read)
                    }
                    state = STATE_LIT;
                    break;
                }
                else
                    RETURN_ERR
            case STATE_LIT_B:
                if (read == 'x')
                {
                    state = STATE_LIT_H;
                    break;
                }
                else
                {
                    state = STATE_LIT;
                    if (read == 'n' || read == 't' || read == '\\' || read == '#')
                    {
                        switch (read)
                        {
                            case 'n': read = '\n';
                                break;
                            case 't': read = '\t';
                                break;
                            case '\\': read = '\\';
                                break;
                            case '#': read = '#';
                            default:break;
                        }
                        APPEND_HEXCODE(read)
                    }
                    else if (read == '\"' || read == '\'')
                    {
                        APPEND(read)
                    }
                    else if (read == ' ')
                    {
                        APPEND_HEXCODE('\\')
                        APPEND_HEXCODE(' ')
                    }
                    else
                    {
                        APPEND_HEXCODE('\\')
                        APPEND(read)
                    }
                    break;
                }
            case STATE_LIT_H:
                if (('0' <= read && read <= '9')
                    || ('a' <= read && read <= 'f')
                    || ('A' <= read && read <= 'F'))
                {
                    state = STATE_LIT_H1;
                    var[0] = (char) read;
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
                    var[1] = (char) read;
                    long dec_num = strtol(var, NULL, 16);
                    APPEND_HEXCODE(dec_num)
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
                    state = STATE_BLOCK3;
                    break;
                }
                else
                    RETURN_ERR
            case STATE_BLOCK3:
                if (read == '"')
                {
                    state = STATE_BLOCK_B1;
                    break;
                }
                else if (read == '\\')
                {
                    flag_block_es = 1;
                    state = STATE_BLOCK_ES1;
                    break;
                }
                else if (read == '\r' || read == '\n' || read == '\t' || read == '#' || read == ' ')
                {
                    APPEND_HEXCODE(read)
                    state = STATE_BLOCK3;
                    break;
                }
                else if (31 < read)
                {
                    APPEND(read)
                    state = STATE_BLOCK3;
                    break;
                }
                else
                    RETURN_ERR
            case STATE_BLOCK_B1:
                if (read == '"')
                {
                    state = STATE_BLOCK_B2;
                    break;
                }
                else if (read == '\\')
                {
                    APPEND('"')
                    flag_block_es = 1;
                    state = STATE_BLOCK_ES1;
                    break;
                }
                else
                {
                    APPEND('"')
                    if (read == ' ' || read == '\r' || read == '\n' || read == '\t' || read == '#')
                    {
                        APPEND_HEXCODE(read)
                    }
                    else
                    {
                        APPEND(read)
                    }
                    state = STATE_BLOCK3;
                    break;
                }
            case STATE_BLOCK_B2:
                if (read == '"')
                {
                    token->type = TOKEN_DOC;
                    return RET_OK;
                }
                else if (read == '\\')
                {
                    APPEND('"')
                    APPEND('"')
                    state = STATE_BLOCK_ES1;
                    flag_block_es = 1;
                    break;
                }
                else
                {
                    APPEND('"')
                    APPEND('"')
                    if (read == ' ' || read == '\r' || read == '\n' || read == '\t' || read == '#')
                    {
                        APPEND_HEXCODE(read)
                    }
                    else
                    {
                        APPEND(read)
                    }
                    state = STATE_BLOCK3;
                    break;
                }

            case STATE_BLOCK_ES1:
                if (read == '\\')
                {
                    if (flag_block_es)
                    {
                        APPEND_HEXCODE('\\')
                    }
                    flag_block_es = 0;
                    APPEND_HEXCODE(read)
                    state = STATE_BLOCK_ES1;
                    break;
                }
                else
                {
                    if (read != '"' && flag_block_es)
                    {
                        APPEND_HEXCODE('\\')
                    }
                    if (read == ' ' || read == '\r' || read == '\n' || read == '\t' || read == '#')
                    {
                        APPEND_HEXCODE(read)
                    }
                    else
                    {
                        APPEND(read)
                    }
                    state = STATE_BLOCK3;
                    break;
                }
            case STATE_COMMENT:
                if (read == '\n')
                {
                    state = STATE_EOL;
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
                    APPEND(read)
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
        }
    }

}
