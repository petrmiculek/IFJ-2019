/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
            xjacko04 (Jacko Daniel)
            xsetin00 (Setinský Jiří)
            xsisma01 (Šišma Vojtěch)
 */
#include "my_string.h"
#include "err.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

unsigned int
init_string(string_t *string)
{
    if ((string->str = (char *) calloc(INITIAL_SIZE, 1)) == NULL)
    {
        return RET_INTERNAL_ERROR;
    }
    string->str[0] = '\0';

    string->size = INITIAL_SIZE;
    string->length = 0;
    return RET_OK;
}



unsigned int
append_char_to_string(string_t *dest, char var)
{
    if (dest == NULL)
    {
        return RET_INTERNAL_ERROR;
    }

    if (dest->length + 2 >= dest->size)
    {

        if ((dest->str = (char *) realloc(dest->str, dest->size + REALLOC_SIZE)) == NULL)
        {
            return RET_INTERNAL_ERROR;
        }
        dest->size += REALLOC_SIZE;
    }

    dest->str[dest->length] = var;
    dest->str[dest->length + 1] = 0;

    dest->length = strlen(dest->str);

    return RET_OK;
}

unsigned int
append_c_string_to_string(string_t *dest, const char *src)
{
    if (src == NULL || dest == NULL)
    {
        return RET_INTERNAL_ERROR;
    }
    size_t src_length = strlen(src);

    size_t size_needed = dest->length + src_length + 1;

    if (dest->size <= size_needed)
    {
        if ((dest->str = (char *) realloc(dest->str, size_needed)) == NULL)
        {
            return RET_INTERNAL_ERROR;
        }
        dest->size = size_needed;
    }

    strncpy(dest->str + dest->length, src, src_length);
    dest->length = strlen(dest->str);
    dest->str[dest->length] = '\0';

    return RET_OK;
}

unsigned int
append_string_to_string(string_t *dest, string_t *src)
{
    return append_c_string_to_string(dest, src->str);
}

int
copy_string(string_t *dest, string_t *src)
{
    if (!dest || !src)
    {
        return RET_INTERNAL_ERROR;
    }
    unsigned int new_length = src->length + 1;

    if (new_length >= dest->size)
    {
        if ((dest->str = (char *) realloc(dest->str, new_length)) == NULL)
        {
            return RET_INTERNAL_ERROR;
        }
        dest->size = new_length;
    }

    strncpy(dest->str, src->str, src->length);
    dest->length = src->length;
    return RET_OK;
}


void
clear_string(string_t *string)
{
    if(string == NULL)
        return;

    memset(string->str, 0, string->size);
    string->length = 0;
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
