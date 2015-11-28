#include <stddef.h>
#include <limits.h>
#include <stdint.h>
#include <sys/io.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/mtx.h>
#include <zero/trix.h>
//#include <zero/randlfg2.h>
#include <zero/asm.h>
#include <kern/conf.h>
#include <kern/util.h>
#include <kern/obj.h>
#include <kern/sched.h>
#include <kern/proc/proc.h>
#include <kern/proc/task.h>
#include <kern/malloc.h>
#include <kern/unit/x86/cpu.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/ia32/task.h>
#include <zero/mtx.h>

extern void taskinitids(void);

void taskqueueready(struct task *task);
void taskaddwait(struct task *task);
void taskaddsleep(struct task *task);
void taskaddstopped(struct task *task);
void taskaddzombie(struct task *task);

struct taskqueue         taskrunqueuetab[SCHEDNFIXED + SCHEDNPRIOCLASS * SCHEDNPRIO] ALIGNED(PAGESIZE);
static struct tasktabl0  taskwaittab[NLVL0TASK];
static struct task      *taskstoppedtab[NTASK];
static struct task      *taskzombietab[NTASK];
//static struct task taskruntab[SCHEDNPRIOCLASS * SCHEDNPRIO];
//static struct task taskrtqueue;
//extern long             trappriotab[NINTR];
static long tasknicetab[64]
= {
    -51,
    -49,
    -48,
    -46,
    -44,
    -43,
    -41,
    -40,
    -38,
    -36,
    -35,
    -33,
    -31,
    -29,
    -28,
    -26,
    -25,
    -23,
    -21,
    -20,
    -18,
    -17,
    -15,
    -13,
    -12,
    -10,
    -9,
    -7,
    -5,
    -4,
    -2,
    -1,
    0,
    2,
    3,
    5,
    6,
    8,
    10,
    11,
    13,
    14,
    16,
    18,
    19,
    21,
    22,
    24,
    26,
    27,
    29,
    30,
    30,
    32,
    33,
    35,
    36,
    38,
    40,
    41,
    43,
    44,
    46,
    48
};
static struct taskqueue  taskrtqueue;
static struct task      *tasksleepqueue;
typedef void taskfunc_t(struct task *);
taskfunc_t              *taskfunctab[TASKNSTATE]
= {
    NULL,               // TASKNEW
    taskqueueready,
    taskaddwait,
    taskaddsleep,
    taskaddstopped,
    taskaddzombie
};
static long             *taskniceptr = &tasknicetab[32];

#if 0
/* run task */
FASTCALL
void
taskjmp(struct task *task)
{
    uint8_t *fctx;

    if (task != k_curtask) {
        fctx = task->m_tcb.fctx;
        if (k_curcpu->info->flags & CPUHASFXSR) {
            __asm__ __volatile__ ("fxrstor (%0)\n" : : "r" (fctx));
        } else {
            __asm__ __volatile__ ("frstor (%0)\n" : : "r" (fctx));
        }
    }
    k_curtask = task;
    k_curproc = task->proc;
    k_curpid = task->id;
    m_tcbjmp(&task->m_tcb);

    /* NOTREACHED */
}
#endif

#if (ZEROSCHED)

static __inline__ long
tasksetnice(struct task *task, long val)
{
    long nice;
    
    val = max(-20, val);
    val = min(19, val);
    nice = taskniceptr[val];
    task->nice = nice;

    return nice;
}

/* adjust task priority */
static __inline__ long
taskadjprio(struct task *task)
{
    long sched = task->sched;
    long prio = task->prio;
    long nice = task->nice;

    /* wrap around back to 0 at SCHEDNPRIO */
    prio++;
    if (sched == SCHEDRESPONSIVE) {
        if (prio == SCHEDNPRIO) {
            task->sched = SCHEDNORMAL;
            task->prio = 0;
        }
        prio += SCHEDNFIXED;
    } else {
        prio &= SCHEDNPRIO - 1;
        task->prio = prio;
        prio += SCHEDNFIXED + SCHEDNPRIO * sched + nice;
        prio = min(SCHEDNFIXED + SCHEDNPRIOCLASS * SCHEDNPRIO - 1, prio);
        prio = max(prio, SCHEDNFIXED);
    }

    return prio;
}

static __inline__ long
taskwakeprio(struct task *task)
{
    long sched = task->sched;
    long nice = task->nice;
    long prio = ((sched == SCHEDFIXED)
                 ? task->prio
                 : SCHEDNFIXED + sched * SCHEDNPRIO);

    return prio;
}

#undef QUEUE_SINGLE_TYPE
#undef QUEUE_ITEM_TYPE
#undef QUEUE_TYPE
#define QUEUE_ITEM_TYPE struct task
#define QUEUE_TYPE      struct taskqueue
#include <zero/queue.h>

void
taskqueueready(struct task *task)
{
    long              sched = task->sched;
    long              prio = task->prio;
    long              state = task->state;
    struct taskqueue *queue;
    
    if (sched == SCHEDRT) {
        queue = &taskrtqueue;
        mtxlk(&queue->lk);
        if (prio < 0) {
            /* SCHED_FIFO */
            queuepush(task, &queue);
        } else {
            /* SCHED_RR */
            queueappend(task, &queue);
        }
        mtxunlk(&queue->lk);
    } else {
        if (sched != SCHEDFIXED) {
            prio = taskadjprio(task);
        }
        mtxlk(&taskrunqueuetab[prio].lk);
        queue = &taskrunqueuetab[prio];
        queueappend(task, &queue);
        mtxunlk(&taskrunqueuetab[prio].lk);
    }

    return;
}

/* switch tasks */
FASTCALL
struct task *
taskpick(struct task *curtask)
{
    struct task      *task = NULL;
    struct taskqueue *queue;
    long              sched = curtask->sched;
    long              prio = curtask->prio;
    long              state = curtask->state;
    taskfunc_t       *func = taskfunctab[state];

    if (curtask) {
        func(curtask);
    }
    do {
        for (prio = 0 ;
             prio < SCHEDNFIXED + SCHEDNPRIOCLASS * SCHEDNPRIO ;
             prio++) {
            queue = &taskrunqueuetab[prio];
            mtxlk(&queue->lk);
            task = queuepop(&queue);
            mtxunlk(&taskrunqueuetab[prio].lk);
            if (task) {
                
                break;
            }
        }
        if (!task) {
            k_enabintr();
            m_waitint();
        }
    } while (!task);
//    taskjmp(task);

    return task;
}

/* add task to wait queue */
void
taskaddwait(struct task *task)
{
    struct tasktabl0  *l0tab;
    struct tasktab    *tab;
    void              *ptr = NULL;
    void             **pptr;
    struct taskqueue  *queue;
    uintptr_t          wtchan = task->wtchan;
    long               fail = 0;    
    long               key0;
    long               key1;
    long               key2;
    long               key3;
    void              *ptab[TASKNKEY - 1] = { NULL, NULL, NULL };

    key0 = taskwaitkey0(wtchan);
    key1 = taskwaitkey1(wtchan);
    key2 = taskwaitkey2(wtchan);
    key3 = taskwaitkey3(wtchan);
    ptr = &taskwaittab[key0];
    l0tab = ptr;
    pptr = ptr;
    mtxlk(&l0tab->lk);
    if (!ptr) {
        ptr = kmalloc(NLVL1TASK * sizeof(struct tasktab));
        if (ptr) {
            kbzero(ptr, NLVL1TASK * sizeof(struct tasktab));
        }
        ptab[0] = ptr;
        pptr = ptr;
    }
    if (ptr) {
        ptr = pptr[key1];
        if (!ptr) {
            ptr = kmalloc(NLVL2TASK * sizeof(struct tasktab));
            if (ptr) {
                kbzero(ptr, NLVL2TASK * sizeof(struct tasktab));
            }
        }
        ptab[1] = ptr;
        pptr[key1] = ptr;
        pptr = ptr;
    } else {
        fail = 1;
    }
    if (ptr) {
        ptr = pptr[key2];
        if (!ptr) {
            queue = kmalloc(NLVL3TASK * sizeof(struct taskqueue));
            if (queue) {
                kbzero(queue, NLVL3TASK * sizeof(struct taskqueue));
            } 
            ptab[2] = queue;
            pptr[key2] = queue;
        } else {
            queue = pptr[key2];
        }
    } else {
        fail = 1;
    }
    if (!fail) {
        queue = &queue[key3];
        queueappend(task, &queue);
        tab = ptab[0];
        tab->nref++;
        tab->tab = ptab[1];
        tab = ptab[1];
        tab->nref++;
        tab->tab = ptab[2];
        tab = ptab[2];
        tab->nref++;
    }
    mtxunlk(&l0tab->lk);
    
    return;
}

/* move a task from wait queue to ready queue */
void
taskunwait(uintptr_t wtchan)
{
    struct tasktabl0  *l0tab;
    struct tasktab    *tab;
    void              *ptr = NULL;
    struct taskqueue  *queue;
    struct taskqueue  *runqueue;
    struct task       *task1;
    struct task       *task2;
    long               key0 = taskwaitkey0(wtchan);
    long               key1 = taskwaitkey1(wtchan);
    long               key2 = taskwaitkey2(wtchan);
    long               key3 = taskwaitkey3(wtchan);
    long               prio;
    void             **pptr;
    void              *ptab[TASKNKEY - 1] = { NULL, NULL, NULL };
    void             **pptab[TASKNKEY - 1] = { NULL, NULL, NULL };

    l0tab = &taskwaittab[key0];
    mtxlk(&l0tab->lk);
    if (l0tab) {
        ptab[0] = l0tab;
        pptab[0] = (void **)&taskwaittab[key0];
        tab = ((void **)l0tab)[key1];
        if (tab) {
            ptab[1] = tab;
            pptab[1] = (void **)&tab[key0];
            tab = ((void **)tab)[key2];
            if (tab) {
                ptab[2] = tab;
                pptab[2] = (void **)&tab[key2];
                queue = ((void **)tab)[key3];
                if (queue) {
                    task1 = queue->next;
                    while (task1) {
                        if (task1->next) {
                            task1->next->prev = NULL;
                        }
                        queue->next = task1->next;
                        task2 = task1->next;
                        prio = taskwakeprio(task1);
                        runqueue = &taskrunqueuetab[prio];
                        mtxlk(&runqueue->lk);
                        queueappend(task1, &runqueue);
                        mtxunlk(&runqueue->lk);
                        task1 = task2;
                    }
                    tab = ptab[2];
                    if (tab) {
                        if (!--tab->nref) {
                            pptr = pptab[2];
                            kfree(tab);
                            *pptr = NULL;
                        }
                        tab = ptab[1];
                        if (tab) {
                            if (!--tab->nref) {
                                pptr = pptab[1];
                                kfree(tab);
                                *pptr = NULL;
                            }
                            tab = ptab[0];
                            if (tab) {
                                if (!--tab->nref) {
                                    pptr = pptab[0];
                                    kfree(tab);
                                    *pptr = NULL;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    mtxunlk(&l0tab->lk);
}

void
taskaddsleep(struct task *task)
{
    time_t       waketm = task->waketm;
    struct task *sleeptask = tasksleepqueue;

    if (!sleeptask) {
        listinit(task);
        tasksleepqueue = task;
    } else {
        while ((sleeptask) && (sleeptask->next)) {
            if (task->waketm < sleeptask->waketm) {
                task->next = sleeptask;
                task->prev = sleeptask->prev;
                sleeptask->prev = task;

                break;
            }
            sleeptask = sleeptask->next;
        }
        if (task->waketm < sleeptask->waketm) {
            task->next = sleeptask;
            task->prev = sleeptask->prev;
            sleeptask->prev = task;
        } else {
            task->prev = sleeptask;
            sleeptask->next = task;
            task->next = NULL;
        }
    }

    return;
}

void
taskaddstopped(struct task *task)
{
    long id = task->id;

    taskstoppedtab[id] = task;

    return;
}

void
taskaddzombie(struct task *task)
{
    long id = task->id;

    taskzombietab[id] = task;

    return;
}

void
taskinitenv(void)
{
    taskinitids();

    return;
}

#endif /* ZEROSCHED */

