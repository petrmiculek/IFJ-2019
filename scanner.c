#include "scanner.h"
#include "err.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*######################TODO LIST######################
-0 v celociselnej casti ako chyba


*/

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

unsigned int
free_string(string_t *string)
{
    if (string != NULL)
    {
        string->size = 0;
        string->length = 0;
        free(string->str);
    }
    return OK;
}

unsigned int
get_token(token_t *token, FILE *file)
{
    if (init_string(&token->string))
    {
        return INTERNAL_ERROR;
    }
    unsigned int state = STATE_START;
    int read;

    while (1)
    {

        read = fgetc(file);
        if (read == EOF)
        {
            // handle
        }

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
                    state = STATE_START;
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
                    break;
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
                else if ((read >= 'a' && read <= 'z') || (read >= 'A' && read <= 'Z') || (read == '-'))
                {

                    state = STATE_INDENT;
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
                    break;
                }
                else if (read == '<')
                {
                    state = STATE_LESS;
                    break;
                }
                else if (read == '/')
                {
                    state = STATE_DIVISION;
                    token->type = TOKEN_DIVISION;
                    return OK;
                }
                else if (read == ':')
                {
                    state = STATE_COLON;
                    token->type = TOKEN_COLON;
                    return OK;
                }
                else if (read == '-')
                {
                    /* FIXME    this condition can never be true
                                because there is '-' included in
                                STATE_INDENT case
                     */
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
                    state = STATE_END;
                    token->type = TOKEN_END;
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
                else if ((39 < read && read < 48) || (59 < read && read < 63) || read == ' ' || read == '"' ||
                    read == '\n')
                { //   <> () *+=-/ " "  # " ,

                    token->type = TOKEN_INT;
                    ungetc(read, file);

                    return OK;
                }
                else
                {
                    state = STATE_ERROR;
                    RET_ERR
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
                else if ((39 < read && read < 48) || (59 < read && read < 63) || read == ' ' || read == '"' ||
                    read == '\n')
                { //   <> () *+=-/ " "  # " ,
                    token->type = TOKEN_FLOAT;
                    ungetc(read, file);
                    return OK;
                }
                else
                {
                    state = STATE_ERROR;
                    RET_ERR
                }
            case STATE_FLOAT_E:
                if ('1' <= read && read <= '9')
                {
                    state = STATE_FLOAT_N;
                    APPEND
                    break;
                }
                else if (read == '0')
                {
                    state = STATE_FLOAT_E;
                    break;
                }
                else if (read == '+' || read == '-')
                {
                    state = STATE_FLOAT_S;
                    APPEND
                    break;
                }
                else
                    RET_ERR
            case STATE_FLOAT_N:
                if ('0' <= read && read <= '9')
                {
                    state = STATE_FLOAT_N;
                    APPEND
                    break;
                }
                else if ((39 < read && read < 48) || (59 < read && read < 63) || read == ' ' || read == '"' ||
                    read == '\n')
                {
                    token->type = TOKEN_FLOAT;
                    ungetc(read, file);
                    return OK;
                }
                else
                    RET_ERR

            case STATE_FLOAT_S:
                if ('0' <= read && read <= '9')
                {
                    state = STATE_FLOAT_N;
                    APPEND
                    break;
                }
                else if ((39 < read && read < 48) || (59 < read && read < 63) || read == ' ' || read == '"' ||
                    read == '\n')
                {
                    token->type = TOKEN_FLOAT;
                    ungetc(read, file);
                    return OK;
                }
                else
                    RET_ERR

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
            case STATE_INDENT:
                if ((read >= 'a' && read <= 'z') || (read >= 'A' && read <= 'Z') || (read == '-') ||
                    (read >= '0' && read <= '9'))
                {
                    state = STATE_INDENT;
                    APPEND
                    break;
                }
                else
                    RET_ERR

            default: state = STATE_ERROR;
                break;
        }
    }

}
