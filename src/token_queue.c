#include <assert.h>
#include "token_queue.h"
#include "err.h"

token_queue_t *
q_init_queue()
{
    token_queue_t *queue;

    if (NULL != (queue = malloc(sizeof(token_queue_t))))
    {
        queue->first = NULL;
        queue->last = NULL;
    }

    return queue;
}

int
q_enqueue(token_t *token, token_queue_t *queue)
{
    if (queue == NULL || token == NULL)
    {
        return RET_INTERNAL_ERROR;
    }

    token_elem_t *token_elem;
    if (NULL == (token_elem = malloc(sizeof(token_elem_t))))
    {
        return RET_INTERNAL_ERROR;
    }

    token_elem->token = copy_token(token);
    token_elem->prev = queue->last;
    token_elem->next = NULL;

    if (queue->last == NULL)
    {

        /* If the queue was previously empty, both the first and
         * last must be pointed at the new entry */

        queue->first = token_elem;
        queue->last = token_elem;

    }
    else
    {

        /* The current entry at the last must have next pointed to this
         * new entry */

        queue->last->next = token_elem;

        /* Only the last must be pointed at the new entry */

        queue->last = token_elem;
    }

    return RET_OK;
}

token_t *
q_pop(token_queue_t *queue)
{
    if (queue == NULL || queue->first == NULL)
    {
        return NULL;
    }

    token_elem_t *tmp_elem = queue->first;

    token_t *tmp_token = tmp_elem->token;

    queue->first = queue->first->next;

    if (queue->first == NULL)
    {
        queue->last = NULL;
    }
    else
    {
        queue->first->prev = NULL;
    }

    free(tmp_elem);

    return tmp_token;
}

void
q_free_queue(token_queue_t *queue)
{
    if (!queue)
        return;

    while (queue->first)
    {
        token_elem_t *tmp = queue->first;
        queue->first = queue->first->next;
        free(tmp);
    }
    queue->last = NULL;

    free(queue);
    // FIXME cannot set queue to NULL
}

token_t *
copy_token(token_t *token)
{
    token_t *new_token;
    if (NULL != (new_token = malloc(sizeof(token_t))))
    {
        new_token->type = token->type;
        init_string(&new_token->string);
        if (RET_OK != copy_string(&new_token->string, &token->string))
        {
            return NULL;
        }
    }
    return new_token;
}













