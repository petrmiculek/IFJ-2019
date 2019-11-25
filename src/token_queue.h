#ifndef HEADER_TOKEN_QUEUE
#define HEADER_TOKEN_QUEUE

#include "scanner.h"

typedef struct _token_elem_t token_elem_t;

struct _token_elem_t
{
    token_t *token;
    token_elem_t *next;
    token_elem_t *prev;
};

typedef struct
{
    token_elem_t *first;
    token_elem_t *last;

} token_queue_t;

token_t *
copy_token(token_t *token);

token_queue_t *
q_init_queue();

int
q_enqueue(token_t *token, token_queue_t *queue);

token_t *
q_pop(token_queue_t *queue);

void
q_free_queue(token_queue_t **queue);

#endif //HEADER_TOKEN_QUEUE
