/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
            xjacko04 (Jacko Daniel)
            xsetin00 (Setinský Jiří)
            xsisma01 (Šišma Vojtěch)
 */
#include "scanner.h"
#include "my_string.h"
#include "err.h"
#include <stdio.h>

void
print_token_type(token_t *token);

int
main(int argc, char **argv)
{
    unsigned int res = 0;

    FILE *fp;
    char filename[] = "input";
    char *file = filename;

    if (argc > 1)
    {
        file = argv[1];
    }

    fp = fopen(argv[1], "r");

    if (fp == NULL)
    {
        fprintf(stderr, "Error in opening file \"%s\"\n", file);
        return RET_INTERNAL_ERROR;
    }

    token_t *token;

    if (NULL == (token = malloc(sizeof(token_t))))
    {
        return RET_INTERNAL_ERROR;
    }

    do
    {
        res = get_token(token, fp);

        printf("str: %-40s type: ", token->string.str);
        print_token_type(token);

        if (res != RET_OK)
        {
            printf(" --possibly invalid\n");
        }
        // ^^^ might print invalid data on lexical error, but at least we get some info?

        printf("\n");

        free_string(&token->string);
        if (res != RET_OK)
            return (int) res;
    }
    while (token->type != TOKEN_EOF);

    free(token);

    fclose(fp);

    return RET_OK;
}

void
print_token_type(token_t *token)
{
    if (token->type == TOKEN_INT)
    {
        printf("INT");
    }
    else if (token->type == TOKEN_FLOAT)
    {
        printf("FLOAT");
    }
    else if (token->type == TOKEN_LEFT)
    {
        printf("LEFT");
    }
    else if (token->type == TOKEN_RIGHT)
    {
        printf("RIGHT");
    }
    else if (token->type == TOKEN_LIT)
    {
        printf("LIT");
    }
    else if (token->type == TOKEN_IS_EQUAL)
    {
        printf("IS_EQUAL");
    }
    else if (token->type == TOKEN_N_EQUAL)
    {
        printf("N_EQUAL");
    }
    else if (token->type == TOKEN_MORE)
    {
        printf("MORE");
    }
    else if (token->type == TOKEN_MORE_E)
    {
        printf("MORE_E");
    }
    else if (token->type == TOKEN_LESS)
    {
        printf("LESS");
    }
    else if (token->type == TOKEN_LESS_E)
    {
        printf("LESS_E");
    }
    else if (token->type == TOKEN_MULTI)
    {
        printf("MULTI");
    }
    else if (token->type == TOKEN_DIVISION)
    {
        printf("DIVISION");
    }
    else if (token->type == TOKEN_FLOR_DIV)
    {
        printf("FLOR_DIV");
    }
    else if (token->type == TOKEN_MINUS)
    {
        printf("MINUS");
    }
    else if (token->type == TOKEN_PLUS)
    {
        printf("PLUS");
    }
    else if (token->type == TOKEN_NONE)
    {
        printf("NONE");
    }
    else if (token->type == TOKEN_PASS)
    {
        printf("PASS");
    }
    else if (token->type == TOKEN_RETURN)
    {
        printf("RETURN");
    }
    else if (token->type == TOKEN_COMMA)
    {
        printf("COMMA");
    }
    else if (token->type == TOKEN_COLON)
    {
        printf("COLON");
    }
    else if (token->type == TOKEN_SPACE)
    {
        printf("SPACE");
    }
    else if (token->type == TOKEN_DEF)
    {
        printf("DEF");
    }
    else if (token->type == TOKEN_IF)
    {
        printf("IF");
    }
    else if (token->type == TOKEN_ELSE)
    {
        printf("ELSE");
    }
    else if (token->type == TOKEN_WHILE)
    {
        printf("WHILE");
    }
    else if (token->type == TOKEN_ASSIGN)
    {
        printf("ASSIGN");
    }
    else if (token->type == TOKEN_INDENT)
    {
        printf("INDENT");
    }
    else if (token->type == TOKEN_DEDENT)
    {
        printf("DEDENT");
    }
    else if (token->type == TOKEN_EOL)
    {
        printf("EOL");
    }
    else if (token->type == TOKEN_EOF)
    {
        printf("EOF");
    }
    else if (token->type == TOKEN_IDENTIFIER)
    {
        printf("IDENTIFIER");
    }
    else if (token->type == TOKEN_DOC)
    {
        printf("DOC");
    }
    else
    {
        printf("Unnamed token type(%d)", token->type);
    }
}
