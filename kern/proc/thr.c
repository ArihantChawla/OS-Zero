#include <stddef.h>
#include <sys/io.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/mtx.h>
#include <zero/trix.h>
#include <zero/randlfg2.h>
#include <zero/asm.h>
#include <kern/conf.h>
#include <kern/util.h>
#include <kern/obj.h>
#include <kern/proc/proc.h>
#include <kern/proc/sched.h>
#include <kern/mem.h>
#include <kern/unit/x86/cpu.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/ia32/thr.h>

static struct thrwait thrwaittab[NLVL0THR] ALIGNED(PAGESIZE);
static struct thrq    thrruntab[THRNCLASS * THRNPRIO];
extern long           trappriotab[NINTR];

/* save thread context */
ASMLINK
void
thrsave(struct thr *thr, long retadr, long fp)
{
    uint8_t *fctx = thr->m_tcb.fctx;

    if (k_curcpu->info->flags & CPUHASFXSR) {
        __asm__ __volatile__ ("fxsave (%0)\n" : : "r" (fctx));
    } else {
        __asm__ __volatile__ ("fnsave (%0)\n" : : "r" (fctx));
    }
    m_tcbsave(&thr->m_tcb);
    thr->m_tcb.iret.eip = retadr;
    thr->m_tcb.iret.uesp = fp - sizeof(struct m_trapframe);
    thr->m_tcb.genregs.ebp = fp;

    return;
}

/* run thread */
FASTCALL
void
thrjmp(struct thr *thr)
{
    uint8_t *fctx = thr->m_tcb.fctx;

    if (k_curcpu->info->flags & CPUHASFXSR) {
        __asm__ __volatile__ ("fxrstor (%0)\n" : : "r" (fctx));
    } else {
        __asm__ __volatile__ ("frstor (%0)\n" : : "r" (fctx));
    }
    k_curthr = thr;
    k_curproc = thr->proc;
    k_curpid = k_curproc->pid;
    m_tcbjmp(&thr->m_tcb);

    /* NOTREACHED */
    return;
}

#if (ZEROSCHED)

/* adjust thread priority */
static __inline__ long
thradjprio(struct thr *thr)
{
    long class = thr->class;
    long prio = thr->prio;
    long nice = thr->nice;

    if (class == THRINTR) {
        /* thr->prio is IRQ ID */
        prio = trappriotab[thr->prio];
    } else if (class != THRRT) {
        /* wrap around back to 0 at THRNPRIO / 2 */
        prio++;
        prio &= (THRNPRIO >> 1) - 1;
//        prio = (THRNPRIO * class) + (THRNPRIO >> 1) + prio + thr->nice;
        prio += (THRNPRIO * class)
            + (randlfg2() & ((THRNPRIO >> 1) - 1))
            + nice;
        prio = min(THRNPRIO * THRNCLASS - 1, prio);
    }
    thr->prio = prio;

    return prio;
}

/* TODO: use <zero/list.h> */

/* add thread to end of queue */
void
thrqueue(struct thr *thr, struct thrq *thrq)
{
    thr->prev = thrq->tail;
    if (thr->prev) {
        thr->prev->next = thr;
    } else {
        thrq->head = thr;
    }
    thrq->tail = thr;

    return;
}

/* add thread to wait queue */
long
thraddwait(struct thr *thr)
{
    struct thrwait *tab;
    struct thrwait *ptr = NULL;
    long            wchan = thr->wchan;
    long            ret = -1;
    long            fail = 0;    
    uint64_t        key0;
    uint64_t        key1;
    uint64_t        key2;
    uint64_t        key3;
    void           *ptab[THRNKEY] = { NULL, NULL, NULL, NULL };

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
        ptab[0] = ptr;
    }
    if (ptr) {
        ptr = ptr[key0].ptr;
        if (!ptr) {
            ptr = kmalloc(NLVL1THR * sizeof(struct thrwait));
            if (ptr) {
                kbzero(ptr, NLVL1THR * sizeof(struct thrwait));
            }
            ptab[1] = ptr;
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
            ptab[2] = ptr;
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
            ptab[3] = ptr;
        }
    } else {
        fail = 1;
    }
    if (!fail) {
        if (ptab[0]) {
            tab->nref++;
            tab->ptr = ptab[0];
        }
        tab = tab->ptr;
        if (ptab[1]) {
            tab[key0].nref++;
            tab[key0].ptr = ptab[1];
        }
        tab = tab[key0].ptr;
        if (ptab[2]) {
            tab[key1].nref++;
            tab[key1].ptr = ptab[2];
        }
        tab = tab[key1].ptr;
        if (ptab[3]) {
            tab[key2].nref++;
            tab[key2].ptr = ptab[3];
        }
        tab = tab[key2].ptr;
        tab = &tab[key3];
        tab->nref++;
        thr->prev = NULL;
        thr->next = tab->ptr;
        if (thr->next) {
            thr->next->prev = thr;
        }
        tab->ptr = thr;
        ret ^= ret;
    }
    mtxunlk(&tab->lk);
    
    return ret;
}

/* move a thread from wait queue to ready queue */
void
thrwakeup(uintptr_t wchan)
{
    struct thrwait *tab;
    struct thrwait *ptr = NULL;
    struct thrq    *thrq;
    struct thr     *thr1;
    struct thr     *thr2;
    long            key0 = thrwaitkey0(wchan);
    long            key1 = thrwaitkey1(wchan);
    long            key2 = thrwaitkey2(wchan);
    long            key3 = thrwaitkey3(wchan);
//    long            n;
    struct thrwait *ptab[4] = { NULL };

    tab = &thrwaittab[key0];
    mtxlk(&tab->lk);
    ptr = tab;
    if (ptr) {
        ptab[0] = ptr;
        ptr = ((void **)ptr->ptr)[key0];
        if (ptr) {
            ptab[1] = ptr;
            ptr = ((void **)ptr->ptr)[key1];
            if (ptr) {
                ptab[2] = ptr;
                ptr = ((void **)ptr->ptr)[key2];
                if (ptr) {
                    ptab[3] = ptr;
                    ptr = ((void **)ptr->ptr)[key3];
                }
            }
        }
        thr1 = ptr->ptr;
        if (thr1) {
            thr2 = thr1->next;
            thrq = &thrruntab[thr1->prio];
            mtxlk(&thrq->lk);
            thrqueue(thr1, &thrruntab[thr1->prio]);
            ptr->ptr = thr2;
            mtxunlk(&thrq->lk);
        }
#if 0
        while (thr1) {
            thr2 = thr1->next;
            thrqueue(thr1, &thrruntab[thr1->prio]);
            thr1 = thr2;
        }
#endif
        /* TODO: free tables if possible */
        ptr = ptab[0];
        if (ptr) {
            if (!--ptr->nref) {
                kfree(ptr->ptr);
                ptr->ptr = NULL;
            }
            ptr = ptab[1];
            if (ptr) {
                if (!--ptr->nref) {
                    kfree(ptr->ptr);
                    ptr->ptr = NULL;
                }
                ptr = ptab[2];
                if (ptr) {
                    if (!--ptr->nref) {
                        kfree(ptr->ptr);
                        ptr->ptr = NULL;
                    }
                    ptr = ptab[3];
                    if (ptr) {
                        if (!--ptr->nref) {
                            kfree(ptr->ptr);
                            ptr->ptr = NULL;
                        }
                    }
                }
            }
        }
    }
    mtxunlk(&tab->lk);
}

/* switch threads */
FASTCALL
struct thr *
thrpick(void)
{
    struct thr  *thr = k_curthr;
    struct thrq *thrq;
    long         prio;
    long         state;

    if (thr) {
        prio = thradjprio(thr);
        state = thr->state;
        if (state == THRREADY) {
            thrq = &thrruntab[prio];
            mtxlk(&thrq->lk);
            thrqueue(thr, thrq);
            mtxunlk(&thrq->lk);
        } else if (state == THRWAIT) {
            thraddwait(thr);
        }
    }
    thr = NULL;
    while (!thr) {
        for (prio = 0 ; prio < THRNCLASS * THRNPRIO ; prio++) {
            thrq = &thrruntab[prio];
            mtxlk(&thrq->lk);
            thr = thrq->head;
            thr->prev = NULL;
            if (thr) {
                if (thr->next) {
                    thr->next->prev = NULL;
                } else {
                    thrq->tail = NULL;
                }
                thrq->head = thr->next;
                mtxunlk(&thrq->lk);

                return thr;
            } else {
                mtxunlk(&thrq->lk);
            }
        }
    }
    if (thr) {

        return thr;
    } else {

        return NULL;
    }
}

#endif /* ZEROSCHED */

