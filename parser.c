//
// Created by petrmiculek on 15.11.19.
//

#include "parser.h"
#include "scanner.h"
#include "err.h"

void
parse(FILE *file)
{
    token_t token;
    stack_t stack;
    initStack(&stack);

    unsigned int res;
    do
    {
        res = get_token(&token, file, &stack);
    }
    while (token.type != TOKEN_END && res != RET_LEXICAL_ERROR);

    printf((token.type == TOKEN_END) ? "ok" : "err");
}
