#include <stddef.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <kern/conf.h>
#include <kern/obj.h>
//#include <kern/proc.h>
#include <kern/proc/thr.h>
#include <kern/mem.h>
#include <kern/unit/x86/cpu.h>

static struct thrwait  thrwaittab[NLVL0THR] ALIGNED(PAGESIZE);
static struct thr      thrtab[NTHR] ALIGNED(PAGESIZE);
static struct thrq     thrruntab[THRNCLASS * THRNPRIO];
static struct thr     *corethrtab[NCPU];

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
    mtxlk(&thrq->lk);
    thr->prev = thrq->tail;
    if (thr->prev) {
        thr->prev->next = thr;
    } else {
        thrq->head = thr;
    }
    thrq->tail = thr;
    mtxunlk(&thrq->lk);

    return;
}

void *
thraddwait(struct thr *thr)
{
    struct thrwait *tab;
    struct thrwait *ptr = NULL;
    long            wchan = thr->wchan;
    void           *ret = NULL;
    long            fail = 0;    
    uint64_t        key0;
    uint64_t        key1;
    uint64_t        key2;
    uint64_t        key3;
    void           *pstk[THRNKEY] = { NULL, NULL, NULL, NULL };

    key0 = thrwaitkey0(wchan);
    key1 = thrwaitkey1(wchan);
    key2 = thrwaitkey2(wchan);
    key3 = thrwaitkey3(wchan);
    tab = &thrwaittab[key0];
    mtxlk(&tab->lk);
    ptr = tab->ptr;
    if (!ptr) {
        ptr = kmalloc(NLVL0THR * sizeof(struct thrwait));
        if (ptr) {
            kbzero(ptr, NLVL0THR * sizeof(struct thrwait));
        }
        pstk[0] = ptr;
    }
    if (ptr) {
        ptr = ptr[key0].ptr;
        if (!ptr) {
            ptr = kmalloc(NLVL1THR * sizeof(struct thrwait));
            if (ptr) {
                kbzero(ptr, NLVL1THR * sizeof(struct thrwait));
            }
            pstk[1] = ptr;
        }
    } else {
        fail = 1;
    }
    if (ptr) {
        ptr = ptr[key1].ptr;
        if (!ptr) {
            ptr = kmalloc(NLVL2THR * sizeof(struct thrwait));
            if (ptr) {
                kbzero(ptr, NLVL2THR * sizeof(struct thrwait));
            }
            pstk[2] = ptr;
        }
    } else {
        fail = 1;
    }
    if (ptr) {
        ptr = ptr[key2].ptr;
        if (!ptr) {
            ptr = kmalloc(NLVL3THR * sizeof(struct thrwait));
            if (ptr) {
                kbzero(ptr, NLVL3THR * sizeof(struct thrwait));
            }
            pstk[3] = ptr;
        }
    } else {
        fail = 1;
    }
    if (!fail) {
        if (pstk[0]) {
            tab->nref++;
            tab->ptr = pstk[0];
        }
        tab = tab->ptr;
        if (pstk[1]) {
            tab[key0].nref++;
            tab[key0].ptr = pstk[1];
        }
        tab = tab[key0].ptr;
        if (pstk[2]) {
            tab[key1].nref++;
            tab[key1].ptr = pstk[2];
        }
        tab = tab[key1].ptr;
        if (pstk[3]) {
            tab[key2].nref++;
            tab[key2].ptr = pstk[3];
        }
        tab = tab[key2].ptr;
        tab = &tab[key3];
        tab->nref++;
        tab->ptr = thr;
        ret = thr;
    }
    mtxunlk(&tab->lk);
    
    return ret;
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
        prio &= (THRNPRIO - 1);
        prio = (THRNPRIO * class) + max(0, prio + thr->nice);
        prio = max(THRNPRIO, prio);
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
    long         state = curthr->state;

    thrsave(curthr);
    prio = thradjprio(curthr);
    if (state == THRREADY) {
        thrq = &thrruntab[prio];
        thrqueue(curthr, thrq);
    } else if (state == THRWAIT) {
        thraddwait(curthr);
    }
    while (!thr) {
        for (prio = 0 ; prio < THRNCLASS * THRNPRIO ; prio++) {
            thrq = &thrruntab[prio];
            mtxlk(&thrq->lk);
            thr = thrq->head;
            if (thr) {
                if (thr->next) {
                    thr->next->prev = thr;
                } else {
                    thrq->tail = NULL;
                }
                thrq->head = thr->next;
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

