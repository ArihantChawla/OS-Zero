#include <stddef.h>
#include <limits.h>
#include <stdint.h>
#include <sys/io.h>
#include <zero/cdefs.h>
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

void taskaddready(struct task *task);
//void taskaddwait(struct task *task);
void taskaddsleeping(struct task *task);
void taskaddstopped(struct task *task);
void taskaddzombie(struct task *task);

extern struct divul      scheddivultab[SCHEDHISTORYSIZE];

struct taskqueue         taskrunqueuetab[NCPU][SCHEDNPRIOCLASS * SCHEDNPRIOQUEUE] ALIGNED(PAGESIZE);
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
static struct taskqueue  tasksleepqueue;
static long              taskrunbitmap[NCPU][TASKRUNBITMAPSIZE];
typedef void taskfunc_t(struct task *);
taskfunc_t              *taskfunctab[TASKNSTATE]
= {
    NULL,               // TASKNEW
    taskaddready,
    taskaddsleeping,
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
        if (k_cpuinfo->flags & CPUHASFXSR) {
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

#if 0
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
    prio >>= 1;

    return prio;
}
#endif

static __inline__ long
taskwakeprio(struct task *task)
{
    long sched = task->sched;
    long nice = task->nice;
    long prio = ((sched == SCHEDFIXED)
                 ? task->prio
                 : SCHEDNFIXED + sched * SCHEDNPRIO);
    prio >>= 1;

    return prio;
}

static __inline__ long
taskcalcscore(struct task *task)
{
    unsigned long run = task->runtime;
    unsigned long slp = task->slptime;
    unsigned long div;
    unsigned long res;

    if (SCHEDSCORETHRESHOLD <= SCHEDSCOREHALF
        && run >= slp) {

        return SCHEDSCOREHALF;
    }
    if (run > slp) {
        res = SCHEDSCOREMAX;
        div = max(1, run >> 6);
        res -= fastuldiv(slp, div, scheddivultab);

        return res;
    }
    if (slp > run) {
        div = max(1, slp >> 6);
        res = fastuldiv(run, div, scheddivultab);

        return res;
    }
    /* run == slp */
    if (run) {

        return SCHEDSCOREHALF;
    }

    /* run == 0 && slp == 0 */
    return 0;
}

static __inline__ void
taskcalcprio(struct task *task)
{
    unsigned long score;
    long          prio;
    unsigned long delta = SCHEDINTPRIOMAX - SCHEDINTPRIOMIN + 1;
    unsigned long diff;
    unsigned long ntick;
    unsigned long tickhz;
    unsigned long total;
    unsigned long div;
    unsigned long tmp;

    if (!taskistimeshare(task)) {

        return;
    }
    score = taskcalcscore(task);
    score += task->nice;
    score = max(SCHEDINTPRIOMIN, score);
    if (score < SCHEDSCORETHRESHOLD) {
        prio = SCHEDINTPRIOMIN;
        delta = fastuldiv(delta, SCHEDSCORETHRESHOLD);
        delta *= score;
        prio += delta;
    } else {
        ntick = task->ntick;
        total = task->lasttick - task->firsttick;
        tickhz = ntick >> SCHEDTICKSHIFT;
        delta = SCHEDBATCH - (SCHEDNPRIO >> 1);
        prio = SCHEDBATCH + (SCHEDNPRIO >> 1);
        diff = delta - prio + 1;
        if (ntick) {
            tmp = roundup(total, diff);
            div = fastuldiv(tmp, diff);
            prio += task->nice;
            tmp = fastuldiv(tickhz, div);
            delta = min(delta, tmp);
            prio += delta;
        }
    }
    task->prio = prio;

    return;
}

static __inline__ void
taskadjscore(struct task *task)
{
    unsigned long run = task->runtime;
    unsigned long slp = task->slptime;
    unsigned long sum = run + slp;
    unsigned long lim = SCHEDHISTORYSIZE;

    if (sum < lim) {

        return;
    }
    lim <<= 1;
    if (sum > lim) {
        if (run > slp) {
            run = SCHEDHISTORYSIZE;
            slp = 1;
        } else {
            run = 1;
            slp = SCHEDHISTORYSIZE;
        }
        task->runtime = run;
        task->slptime = slp;

        return;
    }
    lim = divu3(lim);
    if (sum > lim) {
        run >>= 1;
        slp >>= 1;
        task->runtime = run;
        task->slptime = slp;

        return;
    }
    run <<= 1;
    slp <<= 1;
    run = divu3(run);
    slp = divu3(slp);
    task->runtime = run;
    task->slptime = slp;

    return;
}

//#undef QUEUE_SINGLE_TYPE
#define QUEUE_SINGLE_TYPE 1
#undef QUEUE_ITEM_TYPE
#undef QUEUE_TYPE
#define QUEUE_TYPE        struct task
//#define QUEUE_ITEM_TYPE struct task
//#define QUEUE_TYPE      struct taskqueue
#include <zero/queue.h>

void
taskaddready(struct task *task)
{
    long              cpu = k_curcpu;
    long              sched = task->sched;
    long              prio = task->prio;
    long              state = task->state;
    struct taskqueue *queue;
    
    if (sched == SCHEDRT) {
        queue = &taskrtqueue.next;
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
        queue = &taskrunqueuetab[cpu][prio].next;
        if (sched != SCHEDFIXED) {
//            prio = taskadjprio(task);
            taskadjscore(task);
        }
        mtxlk(&queue->lk);
        queueappend(task, &queue);
        mtxunlk(queue->lk);
    }

    return;
}

/* switch tasks */
FASTCALL
struct task *
taskpick(struct task *curtask)
{
    long              cpu = k_curcpu;
    struct task      *task = NULL;
    struct taskqueue *queue;
    long              sched = curtask->sched;
    long              state = curtask->state;
    taskfunc_t       *func = taskfunctab[state];
    long              ndx;
    long              ntz;

    if (curtask) {
        func(curtask);
        k_curtask = NULL;
    }
    do {
        for (ndx = 0 ; ndx < TASKRUNBITMAPSIZE ; ndx++) {
            ntz = tzerol(taskrunbitmap[cpu][ndx]);
            if (ntz != LONGSIZE * CHAR_BIT) {
                ntz += ndx * LONGSIZE * CHAR_BIT;
                queue = taskrunqueuetab[cpu][ntz].next;
                mtxlk(&queue->lk);
                task = queuepop(&queue);
                mtxunlk(&queue->lk);
            }
            if (task) {

                return task;
            }
        }
        k_enabintr();
        m_waitint();
    } while (1);
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
    long               cpu = k_curcpu;
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
                        runqueue = &taskrunqueuetab[cpu][prio].next;
                        prio = taskwakeprio(task1);
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

/* FIXME: add a multilevel tree for sleeping tasks for speed */
void
taskaddsleeping(struct task *task)
{
    time_t       waketime = task->waketime;
    struct task *sleeptask = tasksleepqueue.next;

    if (task->wtchan) {
        taskaddwait(task);
    } else if (!sleeptask) {
        queueinit(task);
    } else {
        while ((sleeptask) && (sleeptask->next)) {
            if (task->waketime < sleeptask->waketime) {
                task->next = sleeptask;
                task->prev = sleeptask->prev;
                sleeptask->prev = task;

                break;
            }
            sleeptask = sleeptask->next;
        }
        if (task->waketime < sleeptask->waketime) {
            task->next = sleeptask;
            task->prev = sleeptask->prev;
            sleeptask->prev = task;
        } else {
            task->next = NULL;
            task->prev = sleeptask;
            sleeptask->next = task;
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

