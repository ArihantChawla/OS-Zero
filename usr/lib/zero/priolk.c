#include <stdlib.h>
#include <stdio.h>
#include <zero/cdefs.h>
#include <zero/asm.h>
#if (ZEROSPIN)
#include <zero/spin.h>
#elif (ZEROFMTX)
#include <zero/mtx.h>
#endif
#include <zero/priolk.h>

/* <vendu> eliminated the giant mutex */
#define PRIOLKNONBLOCK 1

THREADLOCAL volatile struct priolkdata *t_priolkptr;
static volatile struct priolkdata      *g_priofree;
#if !defined(PRIOLKNONBLOCK)
#if (ZEROSPIN)
static zerospin                         g_priolkspin = SPININITVAL;
#elif (ZEROFMTX)
static zerofmtx                         g_priolkmtx = FMTXINITVAL;
#endif
#endif

#if !defined(PRIOLKNONBLOCK)
#if (ZEROSPIN)
#define priolkgetspin(lp) spinlk(lp)
#define priolkrelspin(lp) spinunlk(lp)
#elif (ZEROFMTX)
#define priolkgetmtx(lp)  fmtxlk(lp)
#define priolkrelmtx(lp)  fmtxunlk(lp)
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
#if (ZEROSPIN)
    priolkgetspin(&g_priolkspin);
#elif (ZEROFMTX)
    priolkgetmtx(&g_priolkmtx);
#endif
#endif
    if (g_priofree) {
#if !defined(PRIOLKNONBLOCK)
        t_priolkptr = g_priofree;
        g_priofree = t_priolkptr->next;
#else
        do {
            next = NULL;
            head = g_priofree;
            if (head) {
                next = head->next;
            }
            if (!next) {

                break;
            }
        } while (!m_cmpswapptr((volatile long *)g_priofree,
                               (long *)head,
                               (long *)next));
        if (head) {
            t_priolkptr = head;
        } else {
            t_priolkptr = data;
        }
#endif
    } else if (data) {
        t_priolkptr = data;
    } else {
        t_priolkptr = PRIOLKALLOC(sizeof(struct priolkdata));
        if (t_priolkptr == PRIOLKALLOCFAILED) {
            fprintf(stderr, "PRIOLK: failed to allocate priority structure\n");

            exit(1);
        }
    }
    t_priolkptr->val = prio;
    t_priolkptr->orig = prio;
#if !defined(PRIOLKNONBLOCK)
#if (ZEROSPIN)
    priolkrelspin(&g_priolkspin);
#elif (ZEROFMTX)
    priolkrelmtx(&g_priolkmtx);
#endif
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
#if (ZEROSPIN)
    priolkgetspin(&g_priolkspin);
#elif (ZEROFMTX)
    priolkgetmtx(&g_priolkmtx);
#endif
#endif
#if defined(PRIOLKNONBLOCK)
    do {
        next = g_priofree;
        t_priolkptr->next = next;
    } while ((next) && !m_cmpswapptr((volatile long *)g_priofree,
                                     (long *)next,
                                     (long *)t_priolkptr));
#else
    t_priolkptr->next = g_priofree;
    g_priofree = t_priolkptr;
#endif
#if !defined(PRIOLKNONBLOCK)
#if (ZEROSPIN)
    priolkrelspin(&g_priolkspin);
#elif (ZEROFMTX)
    priolkrelmtx(&6_priolkmtx);
#endif
#endif

    return;
}

void
priolkget(struct priolk *priolk)
{
//    unsigned long               prio = t_priolkptr->val;
    unsigned long prio;
    unsigned long mask;
    long          res;

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
    res = m_cmpswapptr((volatile long *)&priolk->owner,
                       NULL,
                       (long *)t_priolkptr);
    if (!res) {

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
        res = m_cmpswapptr((volatile long *)&priolk->owner,
                           NULL,
                           (long *)t_priolkptr);
        if (!res) {
            m_atomand(&priolk->waitbits, ~prio);

            return;
        }
        if (t_priolkptr->val > prio) {
            t_priolkptr->val = prio;
        }
        priolkwait();
    } while (1);

    return;
}

void
priolkrel(struct priolk *priolk)
{
    m_membar();
    priolk->owner = NULL;
    m_membar();
    t_priolkptr->val = t_priolkptr->orig;

    return;
}

