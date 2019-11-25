#include "my_string.h"
#include "err.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
append_char_to_string(string_t *string, char var)
{
    if (string == NULL)
    {
        return RET_INTERNAL_ERROR;
    }

    if (string->length == string->size)
    {

        if ((string->str = (char *) realloc(string->str, sizeof(char) * string->length + REALLOC_SIZE)) == NULL)
        {
            return RET_INTERNAL_ERROR;
        }
        string->size += REALLOC_SIZE;
    }

    string->str[string->length] = var;
    string->length++;
    string->str[string->length] = 0;

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

    dest->length += src_length;
    strncat(dest->str, src, src_length);
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
    unsigned int new_length = src->length;
    if (new_length >= dest->size)
    {
        if ((dest->str = (char *) realloc(dest->str, new_length + 1)) == NULL)
        {
            return RET_INTERNAL_ERROR;
        }
        dest->size = new_length + 1;
    }
    assert(strlen(src->str) == src->length);
    assert(strlen(dest->str) == dest->length);

    assert(dest->size >= dest->length);
    assert(src->size >= src->length);

    assert(dest->size >= src->length);

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
