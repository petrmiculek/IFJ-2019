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
    bool is_defined;
    bool defined_inside_if;
    int function_params_count;
    char *global_variables[500]; // every function id has its global variables, for later check of definiton
    int just_index; //later it can be struct

    /**
        idea - function added to symtable upon call,
        its ifjCode label is generated, so that we can generate CALL $label_of_this_function,
        sometime later a definition is supplied
     */

} sym_table_item;

typedef struct _hash_table_item_t ht_item_t;

struct _hash_table_item_t
{
    char *key;
    ht_item_t *next;

    sym_table_item *data;
};

typedef ht_item_t *table_t[MAX_HTSIZE]; // tHTable is an array of pointers to tHTItem, with a size of MAX_HTSIZE

table_t *
ht_init();

ht_item_t *
ht_search(table_t *ptrht, char *key);

int
ht_insert(table_t *ptrht, char *key, sym_table_item *data);

sym_table_item *
ht_get_data(table_t *ptrht, char *key);

void
ht_delete(table_t *ptrht, char *key);

void
ht_clear_all(table_t *ptrht);

int
check_all_functions_defined(void *data_void);

#endif // HEADER_SYM_TABLE_H
