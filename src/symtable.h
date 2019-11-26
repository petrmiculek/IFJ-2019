#ifndef HEADER_SYM_TABLE_H
#define HEADER_SYM_TABLE_H
/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
            xjacko04 (Jacko Daniel)
            xsetin00 (Setinský Jiří)
            xsisma01 (Šišma Vojtěch)
 */

#include <stdbool.h>
#include "my_string.h"

#define MAX_HTSIZE 101 // FIXME find suitable prime number

typedef struct _sym_table_item_t
{
    string_t identifier;
    bool is_function;
    bool is_function_defined;
    unsigned int function_params_count;



    /**
     * if a function does not have any return statement, we should simulate one?
     */
    bool function_contains_return;

} sym_table_item;

typedef struct _hash_table_item_t ht_item_t;

struct _hash_table_item_t
{
    char *key;
    sym_table_item *data;
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
