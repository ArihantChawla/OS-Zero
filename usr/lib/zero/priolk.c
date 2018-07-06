#include <stdlib.h>
#include <stdio.h>
#include <zero/cdefs.h>
#include <mach/asm.h>
#include <zero/priolk.h>

/* <vendu> eliminated the giant mutex */

static THREADLOCAL struct priolkdata *t_priolkptr;
static volatile struct priolkdata    *g_priofree;

void
priolkset(unsigned long prio)
{
    t_priolkptr->val = 1UL << prio;

    return;
}

/* initiase priority lock data for a thread */
void
priolkinit(struct priolkdata *data, unsigned long val)
{
    struct priolkdata *ptr = data;
    unsigned long      prio = 1UL << val;
    m_atomic_t         res = 0;
    struct priolkdata *head;
    struct priolkdata *next;

    if (!ptr) {
        /* structure not supplied */
        if (g_priofree) {
            /* try to grab from free list */
            do {
                head = (struct priolkdata *)g_priofree;
                if (head) {
                    next = (struct priolkdata *)head->next;
                    res = m_cmpswapptr((m_atomic_t **)&g_priofree,
                                       (long *)head,
                                       (long *)next);
                }
            } while (!res && (g_priofree));
            if (res) {
                /* success */
                ptr = head;
            }
        }
        if (!res) {
            /* try to allocate */
            ptr = PRIOLKALLOC(sizeof(struct priolkdata));
#if defined(PRIOLKUSEMMAP)
            if (ptr == PRIOLKALLOCFAILED) {
                ptr = NULL;
            }
#endif
        }
        if (!ptr) {
            fprintf(stderr, "PRIOLK: failed to initialise priority\n");

            exit(1);
        }
    }
    ptr->val = prio;
    ptr->orig = prio;
    t_priolkptr = ptr;

    return;
}

void
priolkget(struct priolk *priolk)
{
    unsigned long      prio = t_priolkptr->val;
    struct priolkdata *owner;
    unsigned long      mask;
    long               res;

    /* read priority atomically */
    m_membar();
    mask = prio - 1;
    while (priolk->waitbits & mask) {
        /* unfinished higher-priority tasks */
        if (t_priolkptr->val != prio) {
            /* priority changed, reread and update mask */
            prio = t_priolkptr->val;
            m_membar();
            mask = prio - 1;
        }
        m_waitspin();
    }
    /* see if no higher-priority waiters and unlocked */
    res = m_cmpswapptr((m_atomic_t **)&priolk->owner,
                       NULL,
                       (long *)t_priolkptr);
    if (res) {
        /* success */

        return;
    }
    m_atomor(&priolk->waitbits, prio);
    do {
        while (priolk->waitbits & mask) {
            if (t_priolkptr->val != prio) {
                /* priority changed */
                m_atomand(&priolk->waitbits, ~prio);
                prio = t_priolkptr->val;
                m_atomor(&priolk->waitbits, prio);
                mask = prio - 1;
            }
            m_waitspin();
        }
        /* see if no higher-priority waiters and unlocked */
        owner = m_fetchswapptr((m_atomic_t **)&priolk->owner,
                               NULL,
                               (long *)t_priolkptr);
        if (!owner) {
            /* success */
            m_atomand(&priolk->waitbits, ~prio);

            return;
        }
        if (owner->val > prio) {
            owner->val = prio;
        }
        m_waitspin();
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
    m_endspin();

    return;
}

void
priolkfin(void)
{
    struct priolkdata *ptr = t_priolkptr;
    struct priolkdata *head;

    do {
        head = (struct priolkdata *)g_priofree;
        ptr->next = head;
        if (m_cmpswapptr((m_atomic_t **)&g_priofree,
                         (long *)head,
                         (long *)ptr)) {
            t_priolkptr = NULL;

            break;
        }
    } while (1);

    return;
}

