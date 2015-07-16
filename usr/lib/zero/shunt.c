#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zero/mtx.h>
#if (SHUNTZPC)
#include <zpc/zpc.h>
#include <zpc/op.h>
#endif
#include <zero/shunt.h>

#if (SHUNTC)
uint8_t shuntcopchartab[256];
long    shuntcopprectab[SHUNTCNOP];
long    shuntcopnargtab[SHUNTCNOP];
#endif

void
shuntinitcop(uint8_t *chartab, long *prectab, long *nargtab)
{
    /* lookup table */
    chartab['~'] = '~';
    chartab['&'] = '&';
    chartab['|'] = '|';
    chartab['^'] = '^';
    chartab['<'] = '<';
    chartab['>'] = '>';
    chartab['+'] = '+';
    chartab['-'] = '-';
    chartab['*'] = '*';
    chartab['/'] = '/';
    chartab['%'] = '%';
    chartab['='] = '=';
    /* precedences */
    prectab[SHUNTCNOT] = SHUNTCRTOL | 8;
    prectab[SHUNTCINC] = 8;
    prectab[SHUNTCDEC] = 8;
    prectab[SHUNTCSHL] = 7;
    prectab[SHUNTCSHR] = 7;
    prectab[SHUNTCAND] = 6;
    prectab[SHUNTCXOR] = SHUNTCRTOL | 5;
    prectab[SHUNTCOR] = 4;
    prectab[SHUNTCMUL] = 3;
    prectab[SHUNTCDIV] = 3;
    prectab[SHUNTCMOD] = 3;
    prectab[SHUNTCADD] = SHUNTCRTOL | 2;
    prectab[SHUNTCSUB] = 2;
    prectab[SHUNTCASSIGN] = 1;
    /* # of arguments */
    nargtab[SHUNTCNOT] = 1;
    nargtab[SHUNTCINC] = 1;
    nargtab[SHUNTCDEC] = 1;
    nargtab[SHUNTCSHL] = 2;
    nargtab[SHUNTCSHR] = 2;
    nargtab[SHUNTCAND] = 2;
    nargtab[SHUNTCXOR] = 2;
    nargtab[SHUNTCOR] = 2;
    nargtab[SHUNTCMUL] = 2;
    nargtab[SHUNTCDIV] = 2;
    nargtab[SHUNTCMOD] = 2;
    nargtab[SHUNTCADD] = 2;
    nargtab[SHUNTCSUB] = 2;

    return;
}

/*
 * Dijkstra's shunting yard algorithm
 * - turns infix-format expressions into RPN queues
 * - https://en.wikipedia.org/wiki/Shunting-yard_algorithm
 */
SHUNT_TOKEN *
shuntparse(SHUNT_TOKEN *srcqueue)
{
    SHUNT_TOKEN *token;
    SHUNT_TOKEN *token1 = srcqueue;
    SHUNT_TOKEN *token2 = NULL;
    SHUNT_TOKEN *token3 = NULL;
    SHUNT_TOKEN *queue = NULL;
    SHUNT_TOKEN *tail = NULL;
    SHUNT_TOKEN *stack = NULL;

    while (token1) {
//        zpcprinttoken(token1);
        token = malloc(sizeof(SHUNT_TOKEN));
        token3 = token1->next;
        memcpy(token, token1, sizeof(SHUNT_TOKEN));
        token = token1;
        if (shuntcisvalue(token)) {
            shuntqueue(token, &queue, &tail);
        } else if (shuntcisfunc(token)) {
            shuntpush(token, &stack);
        } else if (shuntcissep(token)) {
            token2 = stack;
            while ((token2) && token2->type != SHUNT_LEFTPAREN) {
                token2 = shuntpop(&stack);
                shuntqueue(token2, &queue, &tail);
                token2 = shuntpop(&stack);
            }
            if ((token2) && token2->type == SHUNT_LEFTPAREN) {
                
                continue;
            } else {
                fprintf(stderr, "mismatched parentheses: %s\n", token2->str);
                
                return NULL;
            }
        } else if (shuntcisop(token)) {
            token2 = stack;
            while (shuntcisop(token2)) {
                if ((!shuntcisrtol(token)
                     && shuntcopprec(token) <= shuntcopprec(token2))
                    || shuntcopprec(token) < shuntcopprec(token2)) {
//                    fprintf(stderr, "POP: %s (%s)\n", token2->str, token->str);
                    token2 = shuntpop(&stack);
                    shuntqueue(token2, &queue, &tail);
                    token2 = stack;
                } else {

                    break;
                }
            }
//            fprintf(stderr, "PUSH: %s\n", token->str);
            shuntpush(token, &stack);
        } else if (token->type == SHUNT_LEFTPAREN) {
            shuntpush(token, &stack);
        } else if (token->type == SHUNT_RIGHTPAREN) {
            token2 = stack;
            while ((token2) && token2->type != SHUNT_LEFTPAREN) {
                token2 = shuntpop(&stack);
                shuntqueue(token2, &queue, &tail);
                token2 = stack;
            }
            if ((token2) && token2->type == SHUNT_LEFTPAREN) {
                token2 = shuntpop(&stack);
            } else {
                if (token2) {
                    fprintf(stderr, "mismatched parentheses: %s\n",
                            token2->str);
                }
                
                return NULL;
            }
            if (shuntcisfunc(stack)) {
                token2 = shuntpop(&stack);
                shuntqueue(token2, &queue, &tail);
            }
        }
        token1 = token3;
    }
#if 0
    fprintf(stderr, "QUEUE: ");
    zpcprintqueue(queue);
    fprintf(stderr, "STACK: ");
    zpcprintqueue(stack);
#endif
    do {
        token1 = stack;
        if (shuntcisop(token1)) {
            token1 = shuntpop(&stack);
            shuntqueue(token1, &queue, &tail);
        } else if ((token1)
                   && (token1->type == SHUNT_LEFTPAREN
                       || token1->type == SHUNT_RIGHTPAREN)) {
            fprintf(stderr, "mismatched parentheses: %s\n", token1->str);

            return NULL;
        }
    } while (stack);

    return queue;
}

SHUNT_TOKEN *
shunteval(SHUNT_TOKEN *srcqueue)
{
    SHUNT_TOKEN  *token = srcqueue;
    SHUNT_TOKEN  *queue = NULL;
    SHUNT_TOKEN  *tail = NULL;
    SHUNT_TOKEN  *stack = NULL;
    SHUNT_TOKEN  *token1 = token;
    SHUNT_TOKEN  *token2;
    SHUNT_TOKEN  *arg1;
    SHUNT_TOKEN  *arg2;
    SHUNT_RESULT  dest;
    SHUNT_OP     *func;
    long          radix;

    while (token) {
        token2 = token->next;
        if (shuntcisvalue(token)) {
            shuntpush(token, &stack);
        } else if (shuntcisop(token)) {
            if (!token1) {
                fprintf(stderr, "missing argument 1\n");
                
                return NULL;
            }
            arg2 = NULL;
            if (SHUNT_NARGTAB[token->type] == 2) {
                arg2 = shuntpop(&stack);
                if (!arg2) {
                    fprintf(stderr, "missing argument 2\n");

                    return NULL;
                }
            }
            arg1 = token1;
#if (SHUNTZPC)
            fprintf(stderr, "ARGS:\n");
            if (arg1) {
                zpcprinttoken(arg1);
            }
            if (arg2) {
                zpcprinttoken(arg2);
            }
#endif /* SHUNTZPC */
            switch (SHUNT_NARGTAB[token->type]) {
                case 2:
                    if (!arg2) {
                        fprintf(stderr, "invalid argument 2\n");

                        return NULL;
                    }
                case 1:
                    if (!arg1) {
                        fprintf(stderr, "invalid argument 1\n");

                        return NULL;
                    }

                    break;
            }
            func = SHUNT_EVALTAB[token->type];
            if (func) {
                if (arg2) {
                    dest = func(arg2, arg1);
                    if (arg1->radix == 16 || arg2->radix == 16) {
                        token1->radix = 16;
                    } else if (arg1->radix == 8 || arg2->radix == 8) {
                        token1->radix = 8;
                    } else if (arg1->radix == 2 || arg2->radix == 2) {
                        token1->radix = 2;
                    } else {
                        token1->radix = 10;
                    }
#if (SMARTTYPES)
                    token1->type = arg1->type;
                    token1->flags = arg1->flags;
                    token1->sign = arg1->sign;
#endif
                } else if (arg1) {
                    dest = func(arg1, arg2);
                    token1->radix = arg1->radix;
#if (SMARTTYPES)
                    token1->type = arg2->type;
                    token1->flags = arg2->flags;
                    token1->sign = arg2->sign;
#endif
                }
                token1->data.ui64.i64 = dest;
                if (arg1->type == SHUNT_INT64 || arg1->type == SHUNT_UINT64) {
                    radix = token1->radix;
                    if (!radix) {
                        radix = shuntradix;
                    }
                    token1->radix = radix;
                    fprintf(stderr, "RADIX: %ld\n", radix);
                    shuntprintstr(token1, token1->data.ui64.u64, radix);
                }
            }
        }
        token = token2;
    }
    shuntqueue(token1, &queue, &tail);

    return queue;
}

