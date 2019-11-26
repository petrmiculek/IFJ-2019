#ifndef HEADER_SYM_TABLE_H
#define HEADER_SYM_TABLE_H

#include "my_string.h"
#include <stdbool.h>

#define MAX_HTSIZE 101 // FIXME find suitable prime number

typedef struct _sym_table_item_t
{
    char *identifier;
    bool function_id;
    params parametrs;

} sym_table_item;

typedef struct _hash_table_item_t ht_item_t;

typedef struct params
{
    char **param;
    int param_count;

}params;

struct _hash_table_item_t
{
    char *key;
    sym_table_item data;
    ht_item_t *next;
};

typedef ht_item_t *table_t[MAX_HTSIZE]; // tHTable is an array of pointers to tHTItem, with a size of MAX_HTSIZE


table_t *
ht_init();

ht_item_t *
ht_search(table_t *ptrht, char *key);

void
ht_insert(table_t *ptrht, char *key, sym_table_item data);

sym_table_item *
ht_get_data(table_t *ptrht, char *key);

void
ht_delete(table_t *ptrht, char *key);

void
ht_clear_all(table_t *ptrht);

#endif // HEADER_SYM_TABLE_H
