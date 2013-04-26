#include <stddef.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <kern/conf.h>
#include <kern/obj.h>
//#include <kern/proc.h>
#include <kern/proc/thr.h>
#include <kern/unit/x86/cpu.h>

struct thr   thrtab[NTHR] ALIGNED(PAGESIZE);
struct thrq  thrruntab[THRNCLASS * THRNPRIO];
struct thr  *corethrtab[NCPU];

/* save thread context */
void
thrsave(struct thr *thr)
{
    uint32_t pc;

    /* threads return to thryield() */
    m_getretadr(pc);
    m_tcbsave(&thr->m_tcb);
    thr->m_tcb.iret.eip = pc;

    return;
}

/* run thread */
void
thrjmp(struct thr *thr)
{
    curthr = thr;
    curproc = thr->proc;
    m_tcbjmp(&thr->m_tcb);

    /* NOTREACHED */
    return;
}

#if (ZEROSCHED)

/* add thread to end of queue */
void
thrqueue(struct thr *thr, struct thrq *thrq)
{
    long          prio = thr->prio;

    mtxlk(&thrq->lk);
    thr->prev = thrq[prio].tail;
    if (thr->prev) {
        thr->prev->next = thr;
    } else {
        thrq[prio].head = thr;
    }
    thrq[prio].tail = thr;
    mtxunlk(&thrq->lk);

    return;
}

/* adjust thread priority */
long
thradjprio(struct thr *thr)
{
    long class = thr->class;
    long prio = thr->prio;

    if (class != THRRT) {
        /* wrap around back to 0 at maximum value */
        prio++;
        prio &= ((THRNPRIO >> 1) - 1);
        prio = (THRNPRIO * class) + max(0, prio + thr->nice);
        thr->prio = prio;
    }

    return prio;
}

/* switch threads */
void
thryield(void)
{
    struct thr  *thr = NULL;
    struct thrq *thrq;
    long         prio;

    thrsave(curthr);
    prio = thradjprio(curthr);
    thrq = &thrruntab[prio];
    thrqueue(curthr, thrq);
    while (!thr) {
        for (prio = 0 ; prio < THRNCLASS * THRNPRIO ; prio++) {
            mtxlk(&thrq->lk);
            thr = thrruntab[prio].head;
            if (thr) {
                if (thr->next) {
                    thr->next->prev = thr;
                } else {
                    thrruntab[prio].tail = NULL;
                }
                thrruntab[prio].head = thr->next;
                mtxunlk(&thrq->lk);
                if (thr != curthr) {
                    thrjmp(thr);
                } else {

                    return;
                }
            } else {
                mtxunlk(&thrq->lk);
            }
        }
        if (!thr) {
            m_waitint();
        }
    }

    return;
}

#endif /* ZEROSCHED */

