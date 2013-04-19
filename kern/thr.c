#include <stddef.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <kern/conf.h>
#include <kern/proc.h>
#include <kern/thr.h>
#include <kern/unit/x86/cpu.h>

struct thr   thrtab[NTHR] ALIGNED(PAGESIZE);
struct thrq  thrruntab[NPRIO];
struct thr  *corethrtab[NCPU];

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

void
thrjmp(struct thr *thr)
{
    curthr = thr;
    curproc = thr->proc;
    m_tcbjmp(&thr->m_tcb);

    /* NOTREACHED */
}

void
thrqueue(struct thr *thr, struct thrq *thrq)
{
    long          id = thr->id;
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

long
thradjprio(struct thr *thr)
{
    long class;
    long prio;
    long retval;

    class = thr->class;
    if (class != THRRT) {
        prio = ++thr->prio & (THRNPRIO - 1);   // wrap around
        thr->prio = prio;
        retval = class * NPRIO + prio;
    } else {
        retval = thr->prio;
    }

    return retval;
}

void
thryield(void)
{
    struct thr  *thr;
    struct thrq *thrq;
    long         prio;

    thrsave(curthr);
    thr = NULL;
    thradjprio(curthr);
    prio = curthr->prio;
    thrq = &thrruntab[prio];
    thrqueue(curthr, thrq);
    for (prio = 0 ; prio < NPRIO ; prio++) {
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

            break;
        }
        mtxunlk(&thrq->lk);
    }
    if ((thr) && thr != curthr) {
        thrjmp(thr);
    }

    /* fall back to running the earlier thread */
    return;
}

