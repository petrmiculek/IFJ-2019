/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
 */
#include "token_queue.h"
#include "err.h"

token_queue_t *
q_init_queue()
{
    token_queue_t *queue;

    if (NULL != (queue = calloc(sizeof(token_queue_t), 1)))
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
    if (NULL == (token_elem = calloc(sizeof(token_elem_t), 1)))
    {
        return RET_INTERNAL_ERROR;
    }

    token_elem->token = copy_token(token);
    token_elem->prev = queue->last;
    token_elem->next = NULL;

    if (queue->last == NULL)
    {
        queue->first = token_elem;
        queue->last = token_elem;
    }
    else
    {
        queue->last->next = token_elem;
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

    token_elem_t *token_elem_tmp = queue->first;

    token_t *token_tmp = token_elem_tmp->token;

    queue->first = queue->first->next;

    if (queue->first == NULL)
    {
        queue->last = NULL;
    }
    else
    {
        queue->first->prev = NULL;
    }

    free(token_elem_tmp);

    return token_tmp;
}

void
q_free_queue(token_queue_t **queue)
{
    if (!(*queue))
        return;

    while ((*queue)->first)
    {
        token_elem_t *tmp = (*queue)->first;
        (*queue)->first = (*queue)->first->next;
        free(tmp);
    }
    (*queue)->last = NULL;

    free((*queue));
    *queue = NULL;
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













