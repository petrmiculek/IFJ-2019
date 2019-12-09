/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
            xjacko04 (Jacko Daniel)
            xsetin00 (Setinský Jiří)
            xsisma01 (Šišma Vojtěch)
 */
#include "symtable.h"
#include "err.h"
#include "parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
  // How to initialize
 	ptrht = (tHTable *) malloc(sizeof(tHTable));
	ht_init(ptrht);

 */
static int hash_table_size = MAX_HTSIZE;

unsigned int
hash(const char *str)
{
    unsigned int hash = 0;
    int c;

    while ((c = *str++))
        hash = c + (hash << 6U) + (hash << 16U) - hash;

    return hash;
}

table_t *
ht_init()
{
    table_t *ptrht = malloc(sizeof(table_t));
    if (ptrht == NULL)
    {
        fprintf(stderr, "# %s: failed to alloc \n", __func__);
        return NULL;
    }

    for (int i = 0; i < hash_table_size; ++i)
    {
        (*ptrht)[i] = NULL;
    }

    return ptrht;
}

ht_item_t *
ht_search(table_t *ptrht, char *key)
{
    unsigned int keyHash = hash(key) % hash_table_size;

    ht_item_t *tmp = (ht_item_t *)(*ptrht)[keyHash];

    while (tmp != NULL)
    {
        if (strcmp(tmp->key, key) == 0)
        {
            return tmp;
        }
        else
        {
            tmp = tmp->next;
        }
    }

    return NULL;
}

int ht_insert(table_t *ptrht, char *key, sym_table_item *data)
{
    ht_item_t *searched_item = ht_search(ptrht, key);

    if (searched_item)
    {
        fprintf(stderr, "# %s,%u: overwriting sym_table_item (%s)\n", __func__, __LINE__, key);
        return RET_SEMANTICAL_ERROR; // Redefining identifier
    }
    else
    {
        // item not in hashTable
        unsigned int keyHash = hash(key) % hash_table_size;

        ht_item_t *tmp;

        // create item

        if (NULL == (tmp = calloc(sizeof(ht_item_t), 1)))
        {
            fprintf(stderr, "# %s, %u: alloc error\n", __func__, __LINE__);
            return RET_INTERNAL_ERROR;
        }

        tmp->next = (ht_item_t *)(*ptrht)[keyHash];

        tmp->key = calloc(strlen(key) + 1, 1);
        tmp->data = calloc(sizeof(sym_table_item), 1);

        if (tmp->key == NULL || tmp->data == NULL)
        {
            return RET_INTERNAL_ERROR;
        }

        strcpy(tmp->key, key);

        // *tmp->data = *data;
        copy_string(&(tmp->data->identifier), &(data->identifier));
        tmp->data->function_params_count = data->function_params_count;
        *(tmp->data->global_variables) = *(data->global_variables);
        tmp->data->is_defined = data->is_defined;
        tmp->data->is_function = data->is_function;
        tmp->data->just_index = data->just_index;
        (*ptrht)[keyHash] = tmp;
    }

    return RET_OK;
}

sym_table_item *
ht_get_data(table_t *ptrht, char *key)
{
    ht_item_t *item = ht_search(ptrht, key);

    if (item)
    {
        return item->data;
    }
    else
    {
        return NULL;
    }
}

void ht_delete(table_t *ptrht, char *key)
{
    unsigned int keyHash = hash(key) % hash_table_size;

    ht_item_t *tmp = (ht_item_t *)(*ptrht)[keyHash];
    ht_item_t *prev_item = NULL;

    while (tmp != NULL)
    {
        if (strcmp(tmp->key, key) == 0)
        {
            // delete item

            if (prev_item)
            {
                // item being deleted has a predecessor
                prev_item->next = tmp->next;
                free(tmp);
                tmp = NULL;
            }
            else
            {
                // item being deleted is first item
                ht_item_t *to_be_freed = tmp;
                (*ptrht)[keyHash] = tmp->next;
                free(to_be_freed);

                break;
            }
        }
        else
        {
            prev_item = tmp;
            tmp = tmp->next;
        }
    }
}

void ht_clear_all(table_t *ptrht)
{
    for (int i = 0; i < hash_table_size; ++i)
    {
        ht_item_t *tmp = (ht_item_t *)(*ptrht)[i];
        ht_item_t *prev_item = NULL;

        while (tmp != NULL)
        {
            prev_item = tmp;
            tmp = tmp->next;
            free(prev_item);
        }

        (*ptrht)[i] = NULL;
    }
}

int check_all_functions_defined(void *data_void)
{
    data_t *data_typed = (data_t *)data_void;
    table_t *table = data_typed->global_sym_table; // TODO cast? (ht_item_t**)

    if (table == NULL)
        return RET_INTERNAL_ERROR;

    ht_item_t *tmp;

    for (int i = 0; i < hash_table_size; ++i)
    {
        tmp = (*table)[i];
        while (tmp != NULL)
        {
            // TODO recent change
            if (tmp->data && tmp->data->is_defined == false)
            {
                fprintf(stderr, "# %s, %u: undefined function (%s)\n", __func__, __LINE__, tmp->key);
                return RET_SEMANTICAL_ERROR; // missing function definition
            }

            tmp = tmp->next;
        }
    }

    return RET_OK;
}
