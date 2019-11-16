#include "scanner.h"
#include "my_string.h"
#include "parser.h"
#include "err.h"
#include <stdlib.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
    FILE *f;
    if (argc == 1)
    {
        perror("No input file given\n");
        return RET_INTERNAL_ERROR; // TODO idk which ret_code to use
    }
    if ((f = fopen(argv[1], "r")) == NULL)
    {
        fprintf(stderr, "Error in opening file %s", argv[1]);
        return RET_INTERNAL_ERROR;
    }

    parse(f);

    return 0;
}

