#include "scanner.h"
#include "my_string.h"
#include "err.h"
#include <stdlib.h>
#include <stdio.h>

void
print_token_type(token_t token);
int
main(int argc, char **argv)
{
    FILE *fp;
    // int c; // unused
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
        return (-1);
    }
    token_t token;
    stack_t stack;

    initStack(&stack);
    do
    {
        if (get_token(&token, fp, &stack) == RET_LEXICAL_ERROR)
        {
            free_string(&token.string);
            fprintf(stdout, "LEXICAL_ERROR\n");
            return RET_LEXICAL_ERROR;
        }

        printf("STRING: %-40s TYPE: ", token.string.str);
        print_token_type(token);

        free_string(&token.string);
    }
    while (token.type != TOKEN_END);

    free_stack(&stack);
    fclose(fp);

    return (0);
}

void
print_token_type(token_t token)
{
    if (token.type == TOKEN_INT)
    {
        printf("INT\n");
    }
    else if (token.type == TOKEN_FLOAT)
    {
        printf("FLOAT\n");
    }
    else if (token.type == TOKEN_UP)
    {
        printf("UP\n");
    }
    else if (token.type == TOKEN_DOWN)
    {
        printf("DOWN\n");
    }
    else if (token.type == TOKEN_LEFT)
    {
        printf("LEFT\n");
    }
    else if (token.type == TOKEN_RIGHT)
    {
        printf("RIGHT\n");
    }
    else if (token.type == TOKEN_LIT)
    {
        printf("LIT\n");
    }
    else if (token.type == TOKEN_NEG)
    {
        printf("NEG\n");
    }
    else if (token.type == TOKEN_IS_EQUAL)
    {
        printf("IS_EQUAL\n");
    }
    else if (token.type == TOKEN_N_EQUAL)
    {
        printf("N_EQUAL\n");
    }
    else if (token.type == TOKEN_MORE)
    {
        printf("MORE\n");
    }
    else if (token.type == TOKEN_MORE_E)
    {
        printf("MORE_E\n");
    }
    else if (token.type == TOKEN_LESS)
    {
        printf("LESS\n");
    }
    else if (token.type == TOKEN_LESS_E)
    {
        printf("LESS_E\n");
    }
    else if (token.type == TOKEN_MULTI)
    {
        printf("MULTI\n");
    }
    else if (token.type == TOKEN_DIVISION)
    {
        printf("DIVISION\n");
    }
    else if (token.type == TOKEN_FLOR_DIV)
    {
        printf("FLOR_DIV\n");
    }
    else if (token.type == TOKEN_MINUS)
    {
        printf("MINUS\n");
    }
    else if (token.type == TOKEN_PLUS)
    {
        printf("PLUS\n");
    }
    else if (token.type == TOKEN_NONE)
    {
        printf("NONE\n");
    }
    else if (token.type == TOKEN_PASS)
    {
        printf("PASS\n");
    }
    else if (token.type == TOKEN_RETURN)
    {
        printf("RETURN\n");
    }
    else if (token.type == TOKEN_COMMA)
    {
        printf("COMMA\n");
    }
    else if (token.type == TOKEN_COLON)
    {
        printf("COLON\n");
    }
    else if (token.type == TOKEN_SPACE)
    {
        printf("SPACE\n");
    }
    else if (token.type == TOKEN_DEF)
    {
        printf("DEF\n");
    }
    else if (token.type == TOKEN_IF)
    {
        printf("IF\n");
    }
    else if (token.type == TOKEN_ELSE)
    {
        printf("ELSE\n");
    }
    else if (token.type == TOKEN_WHILE)
    {
        printf("WHILE\n");
    }
    else if (token.type == TOKEN_ASSIGN)
    {
        printf("ASSIGN\n");
    }
    else if (token.type == TOKEN_INDENT)
    {
        printf("INDENT\n");
    }
    else if (token.type == TOKEN_DEDENT)
    {
        printf("DEDENT\n");
    }
    else if (token.type == TOKEN_END)
    {
        printf("END\n");
    }
    else if (token.type == TOKEN_EOL)
    {
        printf("EOL\n");
    }
    else if (token.type == TOKEN_EOF)
    {
        printf("EOF\n");
    }
    else if (token.type == TOKEN_IDENTIFIER)
    {
        printf("IDENTIFIER\n");
    }
    else
    {
        printf("Unnamed token type(%d)\n", token.type);
    }
}
