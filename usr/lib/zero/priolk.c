#include <stdlib.h>
#include <stdio.h>
#include <zero/cdefs.h>
#include <zero/asm.h>
#define ZEROMTX 1
#include <zero/mtx.h>
#include <zero/priolk.h>

static THREADLOCAL struct priolkdata *t_priolkptr;
static struct priolkdata             *priofree;
static volatile long                  priolkmtx = MTXINITVAL;

void
priolkset(unsigned long prio)
{
    t_priolkptr->val = 1UL << prio;

    return;
}

void
priolkinit(struct priolkdata *data, unsigned long val)
{
    unsigned long prio = 1UL << val;

    mtxlk(&priolkmtx);
    if (priofree) {
        t_priolkptr = priofree;
        priofree = t_priolkptr->next;
    } else if (data) {
        t_priolkptr = data;
    } else {
        t_priolkptr = malloc(sizeof(struct priolkdata));
        if (!t_priolkptr) {
            fprintf(stderr, "PRIOLK: failed to allocate priority structure\n");

            exit(1);
        }
    }
    t_priolkptr->val = prio;
    t_priolkptr->orig = prio;
    mtxunlk(&priolkmtx);

    return;
}

void
priolkfinish(void)
{
    mtxlk(&priolkmtx);
    t_priolkptr->next = priofree;
    priofree = t_priolkptr;
    mtxunlk(&priolkmtx);

    return;
}

void
priolk(struct priolk *priolk)
{
    unsigned long               prio = t_priolkptr->val;
    volatile struct priolkdata *owner;
    unsigned long               mask;

    m_membar();
    mask = prio - 1;
    while (priolk->waitbits & mask) {
        if (t_priolkptr->val != prio) {
            prio = t_priolkptr->val;
            m_membar();
            mask = prio - 1;
        }
        priolkyield();
    }
//    owner = priolkcmpswap(&priolk->owner, NULL, t_priolkptr);
    owner = m_cmpswapptr((volatile long *)&priolk->owner,
                         NULL,
                         (volatile long *)t_priolkptr);
    if (!owner) {

        return;
    }
    m_atomor(&priolk->waitbits, prio);
    do {
        while (priolk->waitbits & mask) {
            if (t_priolkptr->val != prio) {
                m_atomand(&priolk->waitbits, ~prio);
                prio = t_priolkptr->val;
                m_atomor(&priolk->waitbits, prio);
                mask = prio - 1;
            }
            priolkyield();
        }
#if 0
        owner = priolkcmpswap(&priolk->owner,
                              NULL, prio);
#endif
        owner = m_cmpswapptr((volatile long *)&priolk->owner,
                             NULL,
                             (volatile long *)t_priolkptr);
        if (!owner) {
            m_atomand(&priolk->waitbits, ~prio);

            return;
        }
        if (owner->val > prio) {
            owner->val = prio;
        }
        priolkyield();
    } while (1);

    return;
}

void
priounlk(struct priolk *priolk)
{
    m_membar();
    priolk->owner = NULL;
    m_membar();
    t_priolkptr->val = t_priolkptr->orig;

    return;
}

