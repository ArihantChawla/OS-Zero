#ifndef __ZERO_SHUNT_H__
#define __ZERO_SHUNT_H__

/*
 * USAGE
 * -----
 * SHUNT_TOKEN is a structure with at least the following fields:
 * - prev and next; pointers to previous and next queue item (structure)
 * - type; C-language operation, function, separator, integral types, pointer
 * - param; e.g. type wordsize
 */

#if (SHUNTC)
/* C-language operations */
#define SHUNTCNOT        0x01
#define SHUNTCINC        0x02
#define SHUNTCDEC        0x03
#define SHUNTCSHL        0x04
#define SHUNTCSHR        0x05
#define SHUNTCAND        0x06
#define SHUNTCXOR        0x07
#define SHUNTCOR         0x08
#define SHUNTCMUL        0x09
#define SHUNTCDIV        0x0a
#define SHUNTCMOD        0x0b
#define SHUNTCADD        0x0c
#define SHUNTCSUB        0x0d
#define SHUNTCASSIGN     0x0e
#define SHUNTCNOP        16
#define SHUNTCRTOL       (1L << 31)
#define SHUNTCSEP        (SHUNTCNOP + 0x01)
#define SHUNTCFUNC       (SHUNTCNOP + 0x02)
#define SHUNTCINT        (SHUNTCNOP + 0x03)
#define SHUNTCUINT       (SHUNTCNOP + 0x04)
#define SHUNTCFLOAT      (SHUNTCNOP + 0x05)
#define SHUNTCPTR        (SHUNTCNOP + 0x06)
#define SHUNTCNTAB       (SHUNTCPTR + 1)
/* global tables */
extern uint8_t shuntcopchartab[256];
extern long    shuntcopprectab[SHUNTCNTAB];
extern long    shuntcopnargtab[SHUNTCNTAB];
#endif

/*
 * We assume the following have been defined:
 * SHUNT_TOKEN          - a queue item with prev and next members
 */

/* API function prototypes */
SHUNT_TOKEN * shuntparse(SHUNT_TOKEN *srcqueue);
SHUNT_TOKEN * shunteval(SHUNT_TOKEN *srcqueue);

#if (SHUNTC)
#define PARAMSIZEMASK 0xff
#define shuntcisopchar(c)                                               \
    (shuntcopchartab[(int)(c)])
#define shuntcopprec(tok)                                               \
    (shuntcopprectab[(tok)->type] & ~SHUNTCRTOL)
#define shuntcopisrtol(tok)                                             \
    (shuntcopprectab[(tok)->type] & SHUNTCRTOL)
#define shuntcisvalue(tok)                                              \
    ((tok) && ((tok)->type == SHUNTCINT || (tok)->type == SHUNTCUINT))
#define shuntcwordsize(tp)                                              \
    ((tp)->param & PARAMSIZEMASK)
#define shuntcisfunc(tok)                                               \
    ((tok) && (tok)->type == SHUNTCFUNC)
#define shuntcissep(tok)                                                \
    ((tok) && (tok)->type == SHUNTCSEP)
#define shuntcisop(tok)                                                 \
    ((tok) && ((tok)->type >= SHUNTCNOT && (tok)->type <= SHUNTCASSIGN))
#endif

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

#endif /* __ZERO_SHUNT_H__ */

