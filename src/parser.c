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
        printf("%u: %s\n", token.type, token.string.str);
    }
    while (token.type != TOKEN_END && res != RET_LEXICAL_ERROR);

    printf((token.type == TOKEN_END) ? "ok" : "err");
}
