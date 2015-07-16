#ifndef __ZERO_SHUNT_H__
#define __ZERO_SHUNT_H__

/*
 * We assume the following have been defined:
 * SHUNT_TOKEN          - a queue item with prev and next members
 */

/* API function prototypes */
SHUNT_TOKEN * shuntparse(SHUNT_TOKEN *srcqueue);
SHUNT_TOKEN * shunteval(SHUNT_TOKEN *srcqueue);

/* internal routines */
static __inline__ void
shuntqueue(SHUNT_TOKEN *token,
           SHUNT_TOKEN **queue, SHUNT_TOKEN **tail)
{
    if (token) {
        token->next = NULL;
        if (!*queue) {
            token->prev = NULL;
            *queue = token;
        } else if (*tail) {
            token->prev = *tail;
            (*tail)->next = token;
            *tail = token;
        } else {
            token->prev = *queue;
            (*queue)->next = token;
            *tail = token;
        }
    }

    return;
}

#define shuntpush(tok, stk)                                             \
    do {                                                                \
        if (tok) {                                                      \
            (tok)->prev = NULL;                                         \
            (tok)->next = *(stk);                                       \
            *(stk) = (tok);                                             \
        }                                                               \
    } while (0)
static __inline__ SHUNT_TOKEN *
shuntpop(SHUNT_TOKEN **stack)
{
    SHUNT_TOKEN *_token;

    if (stack) {
        _token = *stack;
        if (_token) {
            *stack = _token->next;
        }
    }

    return _token;
}

#endif /* __ZERO_SHUNT_H__ */

