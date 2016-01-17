#include <stddef.h>
#include <limits.h>
#include <stdint.h>
#include <sys/io.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/fastidiv.h>
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

void tasksetready(struct task *task, long cpu);
void tasksetsleeping(struct task *task, long cpu);
void tasksetstopped(struct task *task, long cpu);
void tasksetzombie(struct task *task, long cpu);

extern struct divul scheddivultab[SCHEDHISTORYSIZE];

/* lookup table to convert nice values to priority offsets */
/* nice is between -20 and 19 inclusively */
/* taskniceptr = &tasknicetab[SCHEDNICEHALF]; */
/* prio += taskniceptr[nice]; */
static const long tasknicetab[SCHEDNICERANGE]
= {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -32, -30, -28, -27,
    -25, -23, -22, -20, -19, -17, -15, -14,
    -12, -11, -9, -7, -6, -4, -3, -1,
    0, 1, 3, 4, 6, 8, 9, 11,
    13, 14, 16, 17, 19, 21, 22, 24,
    26, 27, 29, 31, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};
/* lookup table to convert nice values to slices in 4-ms ticks */
/* nice is between -20 and 19 inclusively */
/* tasksliceptr = &taskslicetab[SCHEDNICEHALF]; */
/* slice = tasksliceptr[nice]; */
static const long taskslicetab[SCHEDNICERANGE]
= {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 1, 2, 2,
    3, 3, 4, 4, 5, 5, 6, 6,
    7, 7, 8, 8, 9, 9, 10, 10,
    11, 11, 12, 12, 13, 13, 14, 14,
    15, 15, 16, 16, 17, 17, 18, 18,
    19, 19, 20, 20, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
};
static long                *taskniceptr = &tasknicetab[32];
static long                *tasksliceptr = &taskslicetab[32];
static struct tasktabl0     taskwaittab[TASKNLVL0WAIT] ALIGNED(PAGESIZE);
static struct tasktabl0     taskdeadlinetab[TASKNLVL0DL];
static struct task         *taskstoppedtab[NTASK];
static struct task         *taskzombietab[NTASK];
struct taskqueue            taskreadytab0[NCPU][SCHEDNQUEUE];
struct taskqueue            taskreadytab1[NCPU][SCHEDNQUEUE];
static long                 taskreadymap0[NCPU][TASKREADYMAPNWORD];
static long                 taskreadymap1[NCPU][TASKREADYMAPNWORD];
static struct taskqueueset  taskreadytab[NCPU];
static struct taskqueue     taskidletab[NCPU][SCHEDNIDLE];
static long                 taskidlemap[NCPU][TASKIDLEMAPNWORD];
static long                 taskdeadlinemap[TASKDEADLINEMAPNWORD];
typedef void tasksetfunc_t(struct task *, long);
tasksetfunc_t              *tasksetfunctab[TASKNSTATE]
= {
    NULL,               // TASKNEW
    tasksetready,       // TASKREADY
    tasksetsleeping,    // TASKSLEEPING
    tasksetstopped,     // TASKSTOPPED
    tasksetzombie       // TASKZOMBIE
};
static struct taskqueue  tasksleepqueue;

void
taskinitsched(void)
{
    long                 lim = NCPU;
    struct taskqueueset *cpuset = &taskreadytab[0];
    long                 cpu;

    for (cpu = 0 ; cpu < lim ; cpu++) {
        cpuset->cur = &taskreadytab0[cpu][0];
        cpuset->next = &taskreadytab1[cpu][0];
        cpuset->idle = &taskidletab[cpu][0];
        cpuset->curmap = &taskreadymap0[cpu][0];
        cpuset->nextmap = &taskreadymap1[cpu][0];
        cpuset->idlemap = &taskidlemap[cpu][0];
        cpuset++;
    }

    return;
}

void
taskswapqueues(long cpu)
{
    struct taskqueueset *cpuset = &taskreadytab[cpu];
    
    ptrswap(cpuset->cur, cpuset->next);
    ptrswap(cpuset->curmap, cpuset->nextmap);

    return;
}

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

static __inline__ void
taskadjnice(struct task *task, long val)
{
    long nice;
    
    val = max(-20, val);
    val = min(19, val);
    nice = taskniceptr[val];
    task->nice = nice;

    return;
}

static __inline__ void
taskadjscore(struct task *task)
{
    long run = task->runtime;
    long slp = task->slptime;
    long lim = SCHEDHISTORYSIZE;
    long sum = run + slp;

    if (sum < lim) {

        return;
    }
    lim <<= 1;
    if (sum > lim) {
        if (run > slp) {
            run = lim;
            slp = 1;
        } else {
            run = 1;
            slp = lim;
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

static __inline__ long
taskcalcscore(struct task *task)
{
    long run = task->runtime;
    long slp = task->slptime;
    long div;
    long res;
    long tmp;

    if (SCHEDSCORETHRESHOLD <= SCHEDSCOREHALF
        && run >= slp) {
        res = SCHEDSCOREHALF;
        task->score = res;

        return res;
    }
    if (run > slp) {
        res = SCHEDSCOREHALF;
        div = max(1, run >> 6);
        res <<= 1;
        tmp = fastuldiv32(run, div, scheddivultab);
        res -= tmp;
        task->score = res;

        return res;
    }
    if (slp > run) {
        div = max(1, slp >> 6);
        res = fastuldiv32(run, div, scheddivultab);
        task->score = res;

        return res;
    }
    /* run == slp */
    if (run) {
        res = SCHEDSCOREHALF;
        task->score = res;

        return SCHEDSCOREHALF;
    }
    task->score = 0;

    /* run == 0 && slp == 0 */
    return 0;
}

/*
 * enforce maximum limit of scheduling history kept; call after either runtime
 * or slptime is adjusted
 */
static __inline__ void
taskadjintparm(struct task *task)
{
    long run = task->runtime;
    long slp = task->slptime;
    long sched = task->sched;
    long sum = run + slp;

    if (sum < SCHEDHISTORYMAX) {

        return;
    }
    if (sum > SCHEDHISTORYMAX * 2) {
        if (run > slp) {
            task->runtime = SCHEDHISTORYMAX;
            task->slptime = 1;
        } else {
            task->runtime = 1;
            task->slptime = SCHEDHISTORYMAX;
        }

        return;
    }
    if (sum > (SCHEDHISTORYMAX >> 3) * 9) {
        /* exceeded by more than 1/8th, divide by 2 */
        run >>= 1;
        slp >>= 1;
    } else {
        /* multiply by 7 / 8 */
        run >>= 3;
        slp >>= 3;
        run *= 7;
        slp *= 7;
    }
    task->runtime = run;
    task->slptime = slp;

    return;
}

static __inline__ void
taskforkintparm(struct task *task)
{
    long run = task->runtime;
    long slp = task->slptime;
    long run2;
    long slp2;
    long sum = run + slp;

    if (sum > SCHEDHISTORYFORKMAX) {
#if (HZ == 250) && (SCHEDHISTORYNSEC == 8)
        /* multiply run and slp by 3 / 8 */
        run2 = run;
        slp2 = slp;
        run >>= 3;
        slp >>= 3;
        run2 >>= 2;
        slp2 >>= 2;
        run += run2;
        slp += slp2;
#else
        ratio = fastuldiv32(sum, SCHEDHISTORYFORKMAX, scheddivultab);
        run = fastuldiv32(run, ratio, scheddivultab);
        slp = fastuldiv32(slp, ratio, scheddivultab);
#endif
        task->runtime = run;
        task->slptime = slp;
    }

    return;
}

/* applied for time-share tasks of classes SCHEDRESPONSIVE..SCHEDBATCH */
/* return value is new priority */
static __inline__ long
taskcalcintparm(struct task *task, long *retscore)
{
    long range = SCHEDINTPRIOMAX - SCHEDINTPRIOMIN + 1;
    long res = 0;
    long score;
    long diff;
    long ntick;
    long tickhz;
    long total;
    long div;
    long tmp;
    
    score = taskcalcscore(task);
    score += task->nice;
    /* do not overlap highest [fixed] priority tasks */
    score = max(SCHEDINTRPRIOMIN, score);
    if (score < SCHEDSCORETHRESHOLD) {
        res = SCHEDINTPRIOMIN;
#if (SCHEDSCORETHRESHOLD == 32)
        range >>= 5;
#else
        range = fastuldiv32(range, SCHEDSCORETHRESHOLD, scheddivultab);
#endif
        range *= score;
        res += range;
    } else {
        ntick = task->ntick;
        if (ntick) {
            total = task->lastrun - task->firstrun;
            tickhz = ntick >> SCHEDTICKSHIFT;
            diff = SCHEDBATCHPRIOMAX - SCHEDNICEHALF;
            res = SCHEDBATCHPRIOMIN + SCHEDNICEHALF;
            total = max(total, HZ);
            range = diff - res + 1;
            tmp = roundup(total, range);
            res += task->nice;
            div = fastuldiv32(total, tmp, scheddivultab);
            range--;
            total = fastuldiv32(tickhz, div, scheddivultab);
            diff = min(total, range);
            res += diff;
        }
    }
    *retscore = score;
    
    return res;
}

static __inline__ long
taskwakeup(struct task *task)
{
    long sched = task->sched;
    long nice = task->nice;
    long prio = ((sched <= 0)
                 ? task->prio
                 : SCHEDUSERPRIOMIN);

    task->state = TASKREADY;
    task->score = 0;

    return prio;
}

#undef QUEUE_SINGLE_TYPE
#undef QUEUE_ITEM_TYPE
#undef QUEUE_TYPE
#define QUEUE_ITEM_TYPE struct task
#define QUEUE_TYPE      struct taskqueue
#include <zero/queue.h>

/* 32-bit time_t values */
#define taskdlkey0(dl) (((dl) >> 16) & 0xffff)
#define taskdlkey1(dl) (((dl) >> 8) & 0xff)
#define taskdlkey2(dl) ((dl) & 0xff)

void
tasksetdeadline(struct task *task)
{
    struct tasktabl0  *l0tab;
    struct tasktab    *tab;
    time_t             deadline = task->timelim;
    unsigned long      key0 = taskdlkey0(deadline);
    unsigned long      key1 = taskdlkey1(deadline);
    unsigned long      key2 = taskdlkey2(deadline);
    long              *map = taskdeadlinemap;
    void              *ptr = NULL;
    void             **pptr = NULL;
    long               fail = 0;
    struct taskqueue  *queue;
    void              *ptab[TASKNDLKEY - 1] = { NULL, NULL };

    mtxlk(&taskdeadlinetab[key0].lk);
    l0tab = &taskdeadlinetab[key0];
    ptr = l0tab->tab;
    pptr = ptr;
    if (!ptr) {
        ptr = kmalloc(TASKNLVL1DL * sizeof(struct tasktab));
        if (ptr) {
            kbzero(ptr, TASKNLVL1DL * sizeof(struct tasktab));
        }
        l0tab->tab = ptr;
        ptab[0] = ptr;
        pptr = ptr;
    }
    if (ptr) {
        ptr = pptr[key1];
        if (!ptr) {
            queue = kmalloc(TASKNLVL2DL * sizeof(struct taskqueue));
            if (queue) {
                kbzero(queue, TASKNLVL2DL * sizeof(struct taskqueue));
            } 
            ptab[1] = queue;
            pptr[key1] = queue;
        } else {
            queue = pptr[key1];
        }
    } else {
        fail = 1;
    }
    if (!fail) {
        queue = &queue[key2];
        queueappend(task, &queue);
        tab = ptab[0];
        tab->nref++;
        tab->tab = ptab[1];
        tab = ptab[1];
        tab->nref++;
    }
    mtxunlk(&taskdeadlinetab[key0].lk);
    
    return;
}

void
tasksetready(struct task *task, long cpu)
{
    long                 sched = task->sched;
    long                 prio = task->prio;
    long                 score = ~0L;
    struct taskqueueset *cpuset = &taskreadytab[cpu];
    struct taskqueue    *queue;
    long                *map;
    long                 lim;
    long                 flg;

    if (sched == SCHEDFIXED) {
        /* insert into [current] fixed-priority queue */
        queue = cpuset->cur;
        map = cpuset->curmap;
        queue += prio;
        queueappend(task, &queue);
        setbit(map, prio);
    } else if (sched <= SCHEDREALTIME) {
        /* SCHEDDEADLINE, SCHEDINTERRUPT or SCHEDREALTIME */
        if (sched == SCHEDDEADLINE) {
            tasksetdeadline(task);
        } else {
            /* insert onto current queue */
            queue = cpuset->cur;
            map = cpuset->curmap;
            if (prio < 0) {
                long qid = -prio;
                
                /* SCHEDREALTIME with SCHED_FIFO */
                queue -= prio;
                queuepush(task, &queue);
                setbit(map, qid);
            } else {
                /* SCHEDINTERRUPT or SCHEDREALTIME with SCHED_RR */
                queue += prio;
                queueappend(task, &queue);
                setbit(map, prio);
            }
        }
    } else if (sched != SCHEDIDLE) {
        /* SCHEDRESPONSIVE, SCHEDNORMAL, SCHEDBATCH */
        flg = task->schedflg;
        if (flg & TASKHASINPUT) {
            /* boost user-interrupt task to highest priority */
//            task->sched = SCHEDRESPONSIVE;
            prio = SCHEDUSERPRIOMIN;
            task->prio = prio;
        } else {
            /* SCHEDRESPONSIVE or SCHEDNORMAL; calculate timeshare priority */
            prio = taskcalcintparm(task, &score);
            prio = min(task->lendprio, prio);
            prio >>= 1;
        }
        if (schedisinteract(score)) {
            /* if interactive, insert onto current queue */
            queue = cpuset->cur;
            map = cpuset->curmap;
        } else {
            /* if not interactive, insert onto next queue */
            queue = cpuset->next;
            map = cpuset->nextmap;
        }
        queue += prio;
        task->score = score;
        queueappend(task, &queue);
        setbit(map, prio);
    } else {
        /* SCHEDIDLE */
        /* insert into idle queue */
        queue = cpuset->idle;
        map = cpuset->idlemap;
        queueappend(task, &queue);
        setbit(map, prio);
    }
    
    return;
}
    
/* add task to wait table */
void
tasksetwait(struct task *task, long cpu)
{
    struct tasktabl0  *l0tab;
    struct tasktab    *tab;
    void              *ptr = NULL;
    void             **pptr;
    struct taskqueue  *queue;
    uintptr_t          wtchan = task->waitchan;
    long               fail = 0;    
    long               key0;
    long               key1;
    long               key2;
    long               key3;
    void              *ptab[TASKNWAITKEY - 1] = { NULL, NULL, NULL };

    key0 = taskwaitkey0(wtchan);
    key1 = taskwaitkey1(wtchan);
    key2 = taskwaitkey2(wtchan);
    key3 = taskwaitkey3(wtchan);
    mtxlk(&taskwaittab[key0].lk);
    l0tab = &taskwaittab[key0];
    ptr = l0tab->tab;
    pptr = ptr;
    if (!ptr) {
        ptr = kmalloc(TASKNLVL1WAIT * sizeof(struct tasktab));
        if (ptr) {
            kbzero(ptr, TASKNLVL1WAIT * sizeof(struct tasktab));
        }
        l0tab->tab = ptr;
        ptab[0] = ptr;
        pptr = ptr;
    }
    if (ptr) {
        ptr = pptr[key1];
        if (!ptr) {
            ptr = kmalloc(TASKNLVL2WAIT * sizeof(struct tasktab));
            if (ptr) {
                kbzero(ptr, TASKNLVL2WAIT * sizeof(struct tasktab));
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
            queue = kmalloc(TASKNLVL3WAIT * sizeof(struct taskqueue));
            if (queue) {
                kbzero(queue, TASKNLVL3WAIT * sizeof(struct taskqueue));
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
    mtxunlk(&taskwaittab[key0].lk);
    
    return;
}

/* FIXME: add a multilevel tree for sleeping tasks for speed */
void
tasksetsleeping(struct task *task, long cpu)
{
    time_t            timelim = task->timelim;
    struct taskqueue *queue = &tasksleepqueue;
    struct task      *sleeptask;

    if (task->waitchan) {
        tasksetwait(task, cpu);
    } else {
        sleeptask = queue->next;
        if (sleeptask) {
            while ((sleeptask) && (sleeptask->next)) {
                if (task->timelim < sleeptask->timelim) {
                    task->prev = sleeptask->prev;
                    task->next = sleeptask;
                    sleeptask->prev = task;
                    
                    return;
                }
                sleeptask = sleeptask->next;
            }
            task->prev = sleeptask;
            task->next = NULL;
            sleeptask->next = task;
            queue->prev = task;
        } else {
            queueappend(task, &queue);
        }
    }

    return;
}

void
tasksetstopped(struct task *task, long cpu)
{
    long id = task->id;

    taskstoppedtab[id] = task;

    return;
}

void
tasksetzombie(struct task *task, long cpu)
{
    long id = task->id;

    taskzombietab[id] = task;

    return;
}

#define m_settask(task)                                                 \
    do {                                                                \
        k_curtask = (task);                                             \
        k_curproc = (task)->proc;                                       \
        k_curpid = (task)->id;                                          \
    } while (0)

/* switch tasks */
FASTCALL
struct task *
taskswtch(struct task *curtask)
{
    long                 cpu = k_curcpu->id;
    struct task         *task = NULL;
    long                 sched = curtask->sched;
    long                 state = curtask->state;
    struct taskqueueset *cpuset = &taskreadytab[cpu];
    struct taskqueue    *queue;
    long                *map;
    tasksetfunc_t       *func;
    long                 ndx;
    long                 val;
    long                 qid;
    long                 lim;
    long                 loop;

    if (curtask) {
        func = tasksetfunctab[state];
        func(curtask, cpu);
        k_curtask = NULL;
    }
    do {
        loop = 1;
        do {
            /* loop over current and next priority-queues */
            lim = rounduppow2(SCHEDNQUEUE, CHAR_BIT * sizeof(long));
            queue = cpuset->cur;
            map = cpuset->curmap;
            for (ndx = 0 ; ndx < lim ; ndx += CHAR_BIT * sizeof(long)) {
                val = map[ndx];
                if (val) {
                    qid = tzerol(val);
                    qid += ndx * sizeof(long) * CHAR_BIT;
                    queue += qid;
                    task = queuepop(&queue);
                    if (task) {
                        if (queueisempty(&queue)) {
                            clrbit(map, qid);
                        }
                        m_settask(task);

                        return task;
                    }
                }
            }
            if (loop) {
                /* if no task found during the first iteration, switch queues */
                taskswapqueues(cpu);
            }
        } while (loop--);
        /* if both current and next queues are empty, look for an idle task */
        lim = rounduppow2(SCHEDNIDLE, CHAR_BIT * sizeof(long));
        queue = cpuset->idle;
        map = cpuset->idlemap;
        for (ndx = 0 ; ndx < lim ; ndx += CHAR_BIT * sizeof(long)) {
            val = map[ndx];
            if (val) {
                qid = tzerol(val);
                qid += ndx * sizeof(long) * CHAR_BIT;
                queue += qid;
                task = queuepop(&queue);
                if (task) {
                    if (queueisempty(&queue)) {
                        clrbit(map, qid);
                    }
                    m_settask(task);
                    
                    return task;
                }
            }
        }
        /* FIXME: try to pull threads from other cores here */
        k_enabintr();
        m_waitint();
    } while (1);

    return task;
}

/* move a task from wait queue to ready queue */
void
taskunwait(uintptr_t wtchan)
{
    long               cpu = k_curcpu->id;
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
    void             **pptr;
    void              *ptab[TASKNWAITKEY - 1] = { NULL, NULL, NULL };
    void             **pptab[TASKNWAITKEY - 1] = { NULL, NULL, NULL };

    mtxlk(&taskwaittab[key0].lk);
    l0tab = &taskwaittab[key0];
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
                        taskwakeup(task1);
                        tasksetready(task1, cpu);
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
    mtxunlk(&taskwaittab[key0].lk);
}

void
taskinitenv(void)
{
    taskinitids();
    taskinitsched();

    return;
}

#endif /* ZEROSCHED */

