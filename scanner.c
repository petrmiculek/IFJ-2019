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
	unsigned int state = START;
	char read;

	while (1)
	{

		read = fgetc(file);
		switch (state)
		{
			case START:

				if ('1' <= read && read <= '9')
				{
					state = INT;
					APPEND
					break;
				} else if (read == '\n')
				{
					state = START;
					break;
				} else if (read == '\'')
				{
					state = LIT;
					break;
				} else if (read == ' ')
				{
					state = SPACE;
					break;
				} else if (read == '"')
				{
					state = BLOCK;
					break;
				} else if (read == '#')
				{
					state = COMMENT;
					break;
				} else if ((read >= 'a' && read <= 'z') || (read >= 'A' && read <= 'Z') || (read == '-'))
				{

					state = IDENT;
					APPEND

					break;
				} else if (read == '!')
				{
					state = NEG;
					break;
				} else if (read == '0')
				{
					RET_ERR;
				} else if (read == '=')
				{
					state = ASSIG;
					break;
				} else if (read == '>')
				{
					state = MORE;
					break;
				} else if (read == '<')
				{
					state = LESS;
					break;
				} else if (read == '/')
				{
					state = DIVISION;
					token->type = TOKEN_DIVISION;
					return OK;
				} else if (read == ':')
				{
					state = COLON;
					token->type = TOKEN_COLON;
					return OK;
				} else if (read == '-')
				{
					state = MINUS;
					token->type = TOKEN_MINUS;
					return OK;
				} else if (read == '+')
				{
					state = PLUS;
					token->type = TOKEN_PLUS;
					return OK;
				} else if (read == '(')
				{
					state = LEFT;
					token->type = TOKEN_LEFT;
					return OK;
				} else if (read == ')')
				{
					state = RIGHT;
					token->type = TOKEN_RIGHT;
					return OK;
				} else if (read == EOF)
				{
					state = END;
					token->type = TOKEN_END;
					return OK;
				} else if (read == '*')
				{
					state = MULTI;
					token->type = TOKEN_MULTI;
					return OK;
				} else if (read == ',')
				{
					state = COMMA;
					token->type = TOKEN_COMMA;
					return OK;
				} else
				{
					state = ERROR;
					RET_ERR
				}

			case INT:
				if (read == '.')
				{
					state = FLOAT;
					APPEND
					break;
				} else if ('0' <= read && read <= '9')
				{
					state = INT;
					APPEND
					break;
				} else if (read == 'e' || read == 'E')
				{
					state = FLOAT_E;
					APPEND
					break;
				} else if ((39 < read && read < 48) || (59 < read && read < 63) || read == ' ' || read == '"' ||
						   read == '\n')
				{ //   <> () *+=-/ " "  # " ,

					token->type = TOKEN_INT;
					ungetc(read, file);

					return OK;
				} else
				{
					state = ERROR;
					RET_ERR
				}
			case FLOAT:
				if ('0' <= read && read <= '9')
				{
					state = FLOAT_D;
					APPEND
					break;
				} else
				{
					state = ERROR;
					RET_ERR
				}
			case FLOAT_D:
				if ('0' <= read && read <= '9')
				{
					state = FLOAT_D;
					APPEND
					break;
				} else if (read == 'e' || read == 'E')
				{
					state = FLOAT_E;
					APPEND
					break;
				} else if ((39 < read && read < 48) || (59 < read && read < 63) || read == ' ' || read == '"' ||
						   read == '\n')
				{ //   <> () *+=-/ " "  # " ,
					token->type = TOKEN_FLOAT;
					ungetc(read, file);
					return OK;
				} else
				{
					state = ERROR;
					RET_ERR
				}
			case FLOAT_E:
				if ('1' <= read && read <= '9')
				{
					state = FLOAT_N;
					APPEND
					break;
				} else if (read == '0')
				{
					state = FLOAT_E;
					break;
				} else if (read == '+' || read == '-')
				{
					state = FLOAT_S;
					APPEND
					break;
				} else
				RET_ERR
			case FLOAT_N:
				if ('0' <= read && read <= '9')
				{
					state = FLOAT_N;
					APPEND
					break;
				} else if ((39 < read && read < 48) || (59 < read && read < 63) || read == ' ' || read == '"' ||
						   read == '\n')
				{
					token->type = TOKEN_FLOAT;
					ungetc(read, file);
					return OK;
				} else
				RET_ERR

			case FLOAT_S:
				if ('0' <= read && read <= '9')
				{
					state = FLOAT_N;
					APPEND
					break;
				} else if ((39 < read && read < 48) || (59 < read && read < 63) || read == ' ' || read == '"' ||
						   read == '\n')
				{
					token->type = TOKEN_FLOAT;
					ungetc(read, file);
					return OK;
				} else
				RET_ERR

			case LIT:
				if (read == '\\')
				{
					state = LIT_B;
					APPEND
					break;
				} else if (read == '\'')
				{
					token->type = TOKEN_LIT;
					return OK;
				} else if (31 < read)
				{
					state = LIT;
					APPEND
					break;
				} else
				RET_ERR
			case LIT_B:
				if (read == 'x')
				{
					state = LIT_H;
					APPEND
					break;
				} else
				{
					state = LIT;
					APPEND
					break;
				}
			case LIT_H:
				if (('0' <= read && read <= '9') || ('a' <= read && read <= 'f') || ('A' <= read && read <= 'F'))
				{
					state = LIT_H1;
					APPEND
					break;
				} else
				RET_ERR
			case LIT_H1:
				if (('0' <= read && read <= '9') || ('a' <= read && read <= 'f') || ('A' <= read && read <= 'F'))
				{
					state = LIT;
					APPEND
					break;
				} else
				RET_ERR
			case BLOCK:
				if (read == '"')
				{
					state = BLOCK1;
					break;
				} else
				RET_ERR
			case BLOCK1:
				if (read == '"')
				{
					state = BLOCK2;
					break;
				} else
				RET_ERR
			case BLOCK2:
				if (read == '"')
				{
					state = BLOCK_B;
					break;
				} else if (31 < read)
				{
					state = BLOCK2;
				} else
				RET_ERR
			case BLOCK_B:
				if (read == '"')
				{
					state = BLOCK1;
					break;
				} else
				RET_ERR
			case BLOCK_B1:
				if (read == '"')
				{
					state = START;
					break;
				} else
				RET_ERR
			case COMMENT:
				if (read == '\n')
				{
					state = START;
					break;
				} else
				{
					state = COMMENT;
					break;
				}
			case IDENT:
				if ((read >= 'a' && read <= 'z') || (read >= 'A' && read <= 'Z') || (read == '-') ||
					(read >= '0' && read <= '9'))
				{
					state = IDENT;
					APPEND
					break;
				} else
				RET_ERR


			default:
				state = ERROR;
				break;
		}
	}


}
