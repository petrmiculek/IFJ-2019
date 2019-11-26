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


int
main(int argc, char **argv)
{
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

    return res;
}

