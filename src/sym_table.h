#ifndef HEADER_SYM_TABLE_H
#define HEADER_SYM_TABLE_H

#include "my_string.h"

// typedef struct htab htab_t
#define MAX_HTSIZE 101

int HTSIZE = MAX_HTSIZE;

typedef struct _data_t
{
    string_t identifier;

} data_t;

typedef struct _hash_table_item_t ht_item_t;

struct _hash_table_item_t
{
    char *key;
    data_t data;
    ht_item_t *next;
};

typedef ht_item_t *table_t[MAX_HTSIZE]; // tHTable is an array of pointers to tHTItem, with a size of MAX_HTSIZE


table_t *
htInit();

ht_item_t *
htSearch(table_t *ptrht, char *key);

void
htInsert(table_t *ptrht, char *key, data_t data);

data_t *
htRead(table_t *ptrht, char *key);

void
htDelete(table_t *ptrht, char *key);

void
htClearAll(table_t *ptrht);

#endif // HEADER_SYM_TABLE_H
