#include "scanner.h"
#include "my_string.h"
#include "err.h"
#include <stdlib.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
    FILE *fp;
    // int c; // unused

    if (argc < 2)
    {
        fp = fopen("input", "r");
    }
    else
    {
        fp = fopen(argv[1], "r");
    }

    if (fp == NULL)
    {
        perror("Error in opening file");
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
            fprintf(stderr, "LEXICAL_ERROR\n");
            return 1;
        }

        printf("STRING: %s TYPE: ", token.string.str);
        if (token.type == TOKEN_FLOAT)
        {
            printf("FLOAT\n");
        }
        else if(token.type==TOKEN_DEDENT){
            printf("DEDENT\n");
        }
        else if(token.type==TOKEN_INDENT){
            printf("INDENT\n");
        }
        else if(token.type==TOKEN_END){
            printf("END\n");
        }
        else if(token.type==TOKEN_EOF){
            printf("EOF\n");
        }
        else if(token.type==TOKEN_SPACE){
            printf("SPACE\n");
        }
        else if(token.type==TOKEN_EOL){
            printf("EOL\n");
        }
        else if(token.type==TOKEN_IDENTIFIER){
            printf("IDENT\n");
        }
        else{
            printf("%d\n",token.type);
        }

        free_string(&token.string);
    } while(token.type!=TOKEN_END);

    free_stack(&stack);
    fclose(fp);

    return(0);
}
