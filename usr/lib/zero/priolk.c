#include <stdlib.h>
#include <stdio.h>
#include <zero/cdefs.h>
#include <zero/asm.h>
#include <zero/mtx.h>
#include <zero/priolk.h>

/* <vendu> eliminated the giant mutex */
#define PRIOLKNONBLOCK

static THREADLOCAL volatile struct priolkdata *t_priolkptr;
static volatile struct priolkdata             *priofree;
#if !defined(PRIOLKNONBLOCK)
#if (ZEROFMTX)
static zerofmtx                                priolkmtx = FMTXINITVAL;
#endif
#endif

void
priolkset(unsigned long prio)
{
    t_priolkptr->val = 1UL << prio;

    return;
}

void
priolkinit(struct priolkdata *data, unsigned long val)
{
    unsigned long               prio = 1UL << val;
    volatile struct priolkdata *head;
    volatile struct priolkdata *next;

    if (data) {
        data->next = NULL;
    }
#if !defined(PRIOLKNONBLOCK)
    fmtxlk(&priolkmtx);
#endif
    if (priofree) {
#if !defined(PRIOLKNONBLOCK)
        t_priolkptr = priofree;
        priofree = t_priolkptr->next;
#else
        do {
            next = NULL;
            head = priofree;
            if (head) {
                next = head->next;
            }
            if (!next) {

                break;
            }
        } while (!m_cmpswapptr((volatile long *)priofree,
                               (volatile long *)head,
                               (volatile long *)next));
        if (head) {
            t_priolkptr = head;
        } else {
            t_priolkptr = data;
        }
#endif
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
#if !defined(PRIOLKNONBLOCK)
    fmtxunlk(&priolkmtx);
#endif

    return;
}

void
priolkfin(void)
{
#if defined(PRIOLKNONBLOCK)
    volatile struct priolkdata *next;
#endif
    
#if !defined(PRIOLKNONBLOCK)
    fmtxlk(&priolkmtx);
#endif
#if defined(PRIOLKNONBLOCK)
    do {
        next = priofree;
        t_priolkptr->next = next;
    } while ((next) && !m_cmpswapptr((volatile long *)priofree,
                                     (volatile long *)next,
                                     (volatile long *)t_priolkptr));
#else
    t_priolkptr->next = priofree;
    priofree = t_priolkptr;
#endif
#if !defined(PRIOLKNONBLOCK)
    fmtxunlk(&priolkmtx);
#endif

    return;
}

void
priolk(struct priolk *priolk)
{
//    unsigned long               prio = t_priolkptr->val;
    unsigned long               prio;
    volatile struct priolkdata *owner;
    unsigned long               mask;

    m_membar();
    prio = t_priolkptr->val;
    mask = prio - 1;
    while (priolk->waitbits & mask) {
        if (t_priolkptr->val != prio) {
            prio = t_priolkptr->val;
            m_membar();
            mask = prio - 1;
        }
        priolkwait();
    }
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
            priolkwait();
        }
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
        priolkwait();
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

