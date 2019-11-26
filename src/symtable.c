/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
            xjacko04 (Jacko Daniel)
            xsetin00 (Setinský Jiří)
            xsisma01 (Šišma Vojtěch)
 */
#include "symtable.h"
#include <stdlib.h>
#include <stdio.h>

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
        fprintf(stderr, "%s: failed to alloc \n", __func__);
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
    unsigned int keyHash = hash(key);

    ht_item_t *tmp = (ht_item_t *) (*ptrht)[keyHash];

    while (tmp != NULL)
    {
        if (tmp->key == key)
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

void
ht_insert(table_t *ptrht, char *key, sym_table_item data)
{
    ht_item_t *searched_item = ht_search(ptrht, key);

    if (searched_item)
    {
        searched_item->data = data;
    }
    else
    {
        // item not in hashTable
        unsigned int keyHash = hash(key);

        ht_item_t *tmp;

        // create item

        if (NULL == (tmp = calloc(sizeof(ht_item_t), 1)))
        {
            fprintf(stderr, "%s, %u: alloc error\n", __func__, __LINE__);
            return;
        }

        tmp->next = (ht_item_t *) (*ptrht)[keyHash];
        tmp->key = key;
        tmp->data = data;
        (*ptrht)[keyHash] = tmp;
    }
}

sym_table_item *
ht_get_data(table_t *ptrht, char *key)
{
    ht_item_t *item = ht_search(ptrht, key);

    if (item)
    {
        return &item->data;
    }
    else
    {
        return NULL;
    }
}

void
ht_delete(table_t *ptrht, char *key)
{
    unsigned int keyHash = hash(key);

    ht_item_t *tmp = (ht_item_t *) (*ptrht)[keyHash];
    ht_item_t *prev_item = NULL;

    while (tmp != NULL)
    {
        if (tmp->key == key)
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

void
ht_clear_all(table_t *ptrht)
{
    for (int i = 0; i < hash_table_size; ++i)
    {
        ht_item_t *tmp = (ht_item_t *) (*ptrht)[i];
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
