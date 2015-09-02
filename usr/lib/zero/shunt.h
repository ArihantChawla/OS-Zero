#ifndef __ZERO_SHUNT_H__
#define __ZERO_SHUNT_H__

#if (SHUNTC)
#include <zero/shuntc.h>
#endif

extern void shuntconvtobin(SHUNT_UINT val, char *str, size_t len);

/*
 * USAGE
 * -----
 * SHUNT_TOKEN is a structure with at least the following fields:
 * - prev and next; pointers to previous and next queue item (structure)
 * - type; C-language operation, function, separator, integral types, pointer
 * - param; e.g. type wordsize
 */

/*
 * We assume the following have been defined:
 * SHUNT_TOKEN          - a queue item with prev and next members
 */

/* API function prototypes */
SHUNT_TOKEN * shuntparse(SHUNT_TOKEN *tokq);
SHUNT_TOKEN * shunteval(SHUNT_TOKEN *tokq);

/* internal routines */

/* token queue operations */
static __inline__ void
shuntqueue(SHUNT_TOKEN *token, SHUNT_TOKEN **queue, SHUNT_TOKEN **tail)
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

/* token stack operations */

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
    SHUNT_TOKEN *_token = NULL;

    if (stack) {
        _token = *stack;
        if (_token) {
            *stack = _token->next;
        }
    }

    return _token;
}

#define shuntprintstr(tok, val, rad)                                    \
    do {                                                                \
        switch (rad) {                                                  \
            case 2:                                                     \
                shuntconvtobin((val), (tok)->str, (tok)->slen);         \
                                                                        \
                break;                                                  \
            case 8:                                                     \
                snprintf((tok)->str, (tok)->slen, "0%llo", (long long)(val)); \
                                                                        \
                break;                                                  \
            case 10:                                                    \
            default:                                                    \
                if ((tok)->type == SHUNT_INT64) {                       \
                    snprintf((tok)->str, (tok)->slen, "%lld", (long long)(val)); \
                } else {                                                \
                    snprintf((tok)->str, (tok)->slen, "%llu", (unsigned long long)(val)); \
                }                                                       \
                                                                        \
                break;                                                  \
            case 16:                                                    \
                if (val <= 0xff) {                                      \
                    sprintf((tok)->str, "0x%02llx", (long long)(val));  \
                } else if (val <= 0xffff) {                             \
                    sprintf((tok)->str, "0x%04llx", (long long)(val));  \
                } else if (val <= 0xffffffff) {                         \
                    sprintf((tok)->str, "0x%08llx", (long long)(val));  \
                } else {                                                \
                    sprintf((tok)->str, "0x%016llx", (long long)(val)); \
                }                                                       \
                                                                        \
                break;                                                  \
        }                                                               \
        (tok)->len = strlen((tok)->str);                                \
    } while (0)

#endif /* __ZERO_SHUNT_H__ */

