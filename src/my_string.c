#include "my_string.h"
#include "err.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int
init_string(string_t *string)
{
    if ((string->str = (char *) malloc(sizeof(char) * INITIAL_SIZE)) == NULL)
    {
        return RET_INTERNAL_ERROR;
    }
    string->str[0] = '\0';

    string->size = INITIAL_SIZE;
    string->length = 0;
    return RET_OK;
}

unsigned int
append_string(string_t *string, char var)
{
    if (string->length == string->size)
    {

        if ((string->str = (char *) realloc(string->str, sizeof(char) * string->length + REALLOC_SIZE)) == NULL)
        {
            return RET_INTERNAL_ERROR;
        }
        string->size += REALLOC_SIZE;
    }
    strncat(string->str, &var, 1);
    strncat(string->str, STRING_END, 1);
    /* FIXME second strncat is not necessary, afaik
     * result of strncat is always null-terminated
     */
    string->length++;
    return RET_OK;
}

int
copy_string(string_t *dest, string_t *src)
{
    if (!dest || !src)
    {
        return RET_INTERNAL_ERROR;
    }
    unsigned int new_length = src->length;
    if (new_length >= dest->size)
    {
        if ((dest->str = (char *) realloc(dest->str, new_length + 1)) == NULL)
        {
            return RET_INTERNAL_ERROR;
        }
        dest->size = new_length + 1;
    }
    strcpy(dest->str, src->str);
    dest->length = new_length;
    return RET_OK;
}

void
free_string(string_t *string)
{
    if (string != NULL)
    {
        string->size = 0;
        string->length = 0;
        free(string->str);
    }
}
