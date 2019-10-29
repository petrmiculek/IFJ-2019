#include "scanner.h"
#include "my_string.h"
#include "err.h"
#include <stdlib.h>
#include <stdio.h>

int main () {
    FILE *fp;
    int c;

    fp = fopen("file.txt","r");
    if(fp == NULL) {
        perror("Error in opening file");
        return(-1);
    }
    token_t token;
    stack_t stack;

    initStack(&stack);
    do{
        if(get_token(&token, fp, &stack)==LEXICAL_ERROR)
        {
            free_string(&token.string);
            fprintf(stderr,"LEXICAL_ERROR\n");
            return 1;
        }

        printf("STRING: %s TYPE: ", token.string);
        if(token.type==TOKEN_FLOAT){
            printf("FLOAT\n");
        }
        else if(token.type==TOKEN_DEDENT){
            printf("DETENT\n");
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