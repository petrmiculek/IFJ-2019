#ifndef MY_STRING
#define MY_STRING

#define INITIAL_SIZE 20
#define REALLOC_SIZE 5

typedef struct
{
    char *str;
    unsigned int length;
    unsigned int size;
} string_t;

/**
 * @brief alloc memory for string
 *
 * @param string pointer
 * @return unsigned int INTERNAL_ERROR if malloc fails, otherwise OK
 */
unsigned int
init_string(string_t *string);

/**
 * @brief add one character at end of string
 *
 * @param dest string to which will be appended
 * @param src Char which is to be appended
 * @return unsigned int INTERNAL_ERROR if realloc fails, otherwise OK
 */
unsigned int
append_char_to_string(string_t *dest, char src);

/**
 * @brief concatenate strings
 *
 * @param dest string to which will be appended
 * @param src string which is to be appended
 * @return unsigned int INTERNAL_ERROR if realloc fails, otherwise OK
 */
unsigned int
append_string_to_string(string_t *dest, string_t *src);

/**
 * @brief concatenate strings
 *
 * @param dest string to which will be appended
 * @param src c_string (char*) which is to be appended
 * @return unsigned int INTERNAL_ERROR if realloc fails, otherwise OK
 */
unsigned int
append_c_string_to_string(string_t *dest, const char *src);

/**
 * TODO missing doc comment
 * Kod pouzit z jednoduchy_interpret
 * @param dest
 * @param src
 * @return
 */
int
copy_string(string_t *dest, string_t *src);

/**
 * @brief free
 *
 * @param string
 * @return unsigned int
 */
void
free_string(string_t *string);
#endif
