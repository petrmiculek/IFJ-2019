/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
            xjacko04 (Jacko Daniel)
            xsetin00 (Setinský Jiří)
            xsisma01 (Šišma Vojtěch)
 */
#include "scanner.h"
#include "my_string.h"
#include "parser.h"
#include "err.h"
#include <stdlib.h>
#include <stdio.h>

//#define file_input 1234098
#define output
int
main(int argc, char **argv)
{

#ifdef file_input
    FILE *file;
    if (argc == 1)
    {
        perror("No input file given\n");
        return RET_INTERNAL_ERROR;
    }
    if ((file = fopen(argv[1], "r")) == NULL)
    {
        fprintf(stderr, "Error in opening file %s", argv[1]);
        return RET_INTERNAL_ERROR;
    }
    int res = parse(file);
#else
    int res = 0;
    #ifdef output
        string_t code;
        if((res = (init_string(&code)) != RET_OK))
            return res;
    #endif
    res = parse(stdin);
#endif

#ifdef output
    if(res == RET_OK)
        printf("%s", code.str);
#endif
    return res;
}

