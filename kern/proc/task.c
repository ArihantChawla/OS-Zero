#include <stddef.h>
#include <limits.h>
#include <stdint.h>
//#include <sys/io.h>
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
#include <kern/cpu.h>
#include <kern/proc/proc.h>
#include <kern/proc/task.h>
#include <kern/malloc.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/ia32/task.h>
#include <zero/mtx.h>

#define __LONGBITS     (CHAR_BIT * LONGSIZE)
#if (__LONGBITS == 32)
#define __LONGBITSLOG2 5
#elif (__LONGBITS == 64)
#define __LONGBITSLOG2 6
#endif

extern void taskinitids(void);

void tasksetready(struct task *task, long cpu);
void tasksetsleeping(struct task *task);
void tasksetstopped(struct task *task);
void tasksetzombie(struct proc *proc);

extern struct divul scheddivultab[SCHEDHISTORYSIZE] ALIGNED(PAGESIZE);

/* lookup table to convert nice values to priority offsets */
/* nice is between -20 and 19 inclusively */
/* taskniceptr = &tasknicetab[SCHEDNICEHALF]; */
/* prio += taskniceptr[nice]; */
long tasknicetab[SCHEDNICERANGE]
= {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -64, -60, -57, -54,
    -51, -47, -44, -41, -38, -35, -31, -28,
    -25, -22, -19, -15, -12, -9, -6, -3,
    0, 3, 6, 9, 13, 16, 19, 23,
    26, 29, 33, 36, 39, 43, 46, 49,
    53, 56, 59, 63, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};
/* lookup table to convert nice values to slices in 4-ms ticks */
/* nice is between -20 and 19 inclusively */
/* tasksliceptr = &taskslicetab[SCHEDNICEHALF]; */
/* slice = tasksliceptr[nice]; */
static const long taskslicetab[SCHEDNICERANGE] ALIGNED(CLSIZE)
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
static const long          *taskniceptr = &tasknicetab[SCHEDNICEHALF];
static const long          *tasksliceptr = &taskslicetab[SCHEDNICEHALF];
static struct tasktabl0     taskwaittab[TASKNLVL0WAIT] ALIGNED(PAGESIZE);
static struct tasktabl0     taskdeadlinetab[TASKNLVL0DL];
static struct task         *taskstoppedtab[NTASK];
static struct proc         *taskzombieproctab[NTASK];
struct task                *taskreadytab0[NCPU][SCHEDNTABQUEUE];
struct task                *taskreadytab1[NCPU][SCHEDNTABQUEUE];
static long                 taskreadymap0[NCPU][TASKREADYMAPNWORD];
static long                 taskreadymap1[NCPU][TASKREADYMAPNWORD];
static struct taskqueueset  taskreadytab[NCPU];
static struct task         *taskidletab[NCPU][SCHEDNTABQUEUE];
static long                 taskidlemap[NCPU][TASKIDLEMAPNWORD];
static long                 taskdeadlinemap[TASKDEADLINEMAPNWORD];
static long                 taskidlecpumap[TASKIDLECPUMAPNWORD];
static struct taskqueue     tasksleepqueue;

void
taskinitsched(void)
{
    long                 lim = NCPU;
    struct taskqueueset *queueset = &taskreadytab[0];
    long                 cpu;

    for (cpu = 0 ; cpu < lim ; cpu++) {
        queueset->cur = &taskreadytab0[cpu][0];
        queueset->next = &taskreadytab1[cpu][0];
        queueset->idle = &taskidletab[cpu][0];
        queueset->curmap = &taskreadymap0[cpu][0];
        queueset->nextmap = &taskreadymap1[cpu][0];
        queueset->idlemap = &taskidlemap[cpu][0];
        queueset++;
    }

    return;
}

static __inline__ void
taskswapqueues(long cpu)
{
    struct taskqueueset *queueset = &taskreadytab[cpu];
    void                *ptr1 = queueset->cur;
    void                *ptr2 = queueset->next;

    queueset->next = ptr1;
    queueset->cur = ptr2;
    ptr1 = queueset->curmap;
    ptr2 = queueset->nextmap;
    queueset->nextmap = ptr1;
    queueset->curmap = ptr2;

    return;
}

static __inline__ long
taskfindidlecpu(void)
{
    long *map = taskidlecpumap;
    long n = NCPU;
    long lim = min(n, (long)(CHAR_BIT * sizeof(long)));
    long ndx = 0;
    long val = 0;

    while (n > 0) {
        for (ndx = 0 ; ndx < lim ; ndx++) {
            if (m_cmpclrbit(map, ndx)) {
                /* FIXME: lock the CPU */
                ndx += val;
                
                return ndx;
            }
        }
        val += lim;
        map++;
        n -= lim;
    }

    return -1;
}

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

/* CHECKED against sched_interact_score() in ULE :) */
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
#if (SCHEDSCOREHALF == 64)
        run >>= 6;
#else
        run = fastuldiv(run, SCHEDSCOREHALF, scheddivultab);
#endif
        res = SCHEDSCOREHALF;
        div = max(1, run);
        res <<= 1;
        tmp = fastuldiv32(slp, div, scheddivultab);
        res -= tmp;
        task->score = res;
        
        return res;
    }
    if (slp > run) {
#if (SCHEDSCOREHALF == 64)
        slp >>= 6;
#else
        slp = fastuldiv(slp, SCHEDHALFSCORE, s);
#endif
        div = max(1, slp);
        res = fastuldiv32(run, div, scheddivultab);
        task->score = res;

        return res;
    }
    /* run == slp */
    if (run) {
        res = SCHEDSCOREHALF;
        task->score = res;

        return res;
    }
    task->score = 0;

    /* run == 0 && slp == 0 */
    return 0;
}

/* based on sched_interact_update() in ULE :) */
/*
 * enforce maximum limit of scheduling history kept; call after either runtime
 * or slptime is adjusted
 */
static __inline__ void
taskadjintparm(struct task *task)
{
    long run = task->runtime;
    long slp = task->slptime;
    long sum = run + slp;

    if (sum < SCHEDHISTORYMAX) {

        return;
    }
    if (sum > 2 * SCHEDHISTORYMAX) {
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

/* based on sched_interact_fork() in ULE */
static __inline__ void
taskforkintparm(struct task *task)
{
    long run = task->runtime;
    long slp = task->slptime;
#if !((HZ == 250) && (SCHEDHISTORYNSEC == 8))
    long ratio;
#endif
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
    long prio = ((sched < SCHEDNSYSCLASS)
                 ? task->prio
                 : SCHEDUSERPRIOMIN);

    task->state = TASKREADY;
    task->score = 0;

    return prio;
}

#define QUEUE_SINGLE_TYPE
#define QUEUE_TYPE      struct task
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
        queueappend(task, &queue->list);
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
    long                  sched = task->sched;
    long                  prio = task->prio;
    long                  score = SCHEDSCOREMAX;
    struct taskqueueset  *queueset = &taskreadytab[cpu];
    struct task         **queue;
    long                 *map;
    long                  qid = zeroabs(prio);
    long                  lim;
    long                  flg;

    if (sched <= SCHEDSYSTEM) {
        /* SCHEDDEADLINE, SCHEDINTERRUPT or SCHEDREALTIME */
        if (sched == SCHEDDEADLINE) {
            tasksetdeadline(task);
        } else {
            /* insert onto current queue */
            qid = schedcalcqueueid(qid);
            map = queueset->curmap;
            queue = &queueset->cur[qid];
            if (prio < 0) {
                /* SCHEDREALTIME with SCHED_FIFO */
                queuepush(task, queue);
            } else {
                /* SCHEDINTERRUPT, SCHEDREALTIME with SCHED_RR, SCHEDSYSTEM */
                queueappend(task, queue);
            }
            setbit(map, qid);
        }
    } else if (sched < SCHEDIDLE) {
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
//            prio = min(task->runprio, prio);
        }
        qid = schedcalcqueueid(prio);
        if (schedisinteract(score)) {
            /* if interactive, insert onto current queue */
            queue = &queueset->cur[qid];
            map = queueset->curmap;
        } else {
            /* if not interactive, insert onto next queue */
            queue = &queueset->next[qid];
            map = queueset->nextmap;
        }
        task->score = score;
        queueappend(task, queue);
        setbit(map, qid);
    } else {
        /* SCHEDIDLE */
        /* insert into idle queue */
        qid = schedcalcqueueid(prio);
        map = queueset->idlemap;
        queue = &queueset->idle[qid];
        queueappend(task, queue);
        setbit(map, qid);
    }
    
    return;
}
    
/* add task to wait table */
void
tasksetwait(struct task *task)
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
        queueappend(task, &queue->list);
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
tasksetsleeping(struct task *task)
{
    time_t            timelim = task->timelim;
    struct taskqueue *queue = &tasksleepqueue;
    struct task      *sleeptask;

    if (task->waitchan) {
        tasksetwait(task);
    } else {
        sleeptask = queue->list;
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
        } else {
            task->prev = NULL;
            task->next = NULL;
            queue->list = task;
        }
    }

    return;
}

void
tasksetstopped(struct task *task)
{
    long id = task->id;

    taskstoppedtab[id] = task;

    return;
}

void
tasksetzombie(struct proc *proc)
{
    long id = proc->ppid;       // FIXME: might need to use proc->pgrp here

    taskzombieproctab[id] = proc;

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
    long                  cpu = k_curcpu->id;
    struct task          *task = NULL;
    long                  state = (curtask) ? curtask->state : -1;
    struct taskqueueset  *queueset = &taskreadytab[cpu];
    struct task         **queue;
    long                 *map;
    long                  ntz;
    long                  val;
    long                  ndx;
    long                  lim;
    long                  loop;

    if (curtask) {
        if (state != TASKNEW) {
            switch (state) {
                case TASKREADY:
                    tasksetready(curtask, cpu);

                    break;
                case TASKSLEEPING:
                    tasksetsleeping(curtask);

                    break;
                case TASKSTOPPED:
                    tasksetstopped(curtask);

                    break;
                case TASKZOMBIE:
                    tasksetzombie(curtask->proc);

                    break;
                default:
                    panic(curtask->id, -1, 0); /* FIXME: error # */

                    break;
            }
        } else {
            ; /* TODO */
        }
    }
    do {
        loop = 1;
        do {
            /* loop over current and next priority-queues */
            lim = rounduppow2(SCHEDNQUEUE, CHAR_BIT * sizeof(long));
            map = queueset->curmap;
            queue = &queueset->cur[0];
            for (ndx = 0 ; ndx < lim ; ndx++) {
                val = map[ndx];
                if (val) {
                    ntz = tzerol(val);
                    ndx <<= __LONGBITSLOG2;
                    ntz += ndx;
                    queue += ntz;
                    task = queuepop(queue);
                    if (task) {
                        if (queueisemptyptr(queue)) {
                            clrbit(map, ntz);
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
        map = queueset->idlemap;
        queue = &queueset->idle[0];
        for (ndx = 0 ; ndx < lim ; ndx++) {
            val = map[ndx];
            if (val) {
                ntz = tzerol(val);
                ndx <<= __LONGBITSLOG2;
                ntz += ndx;
                queue += ntz;
                task = queuepop(queue);
                if (task) {
                    if (queueisemptyptr(queue)) {
                        clrbit(map, ntz);
                    }
                    m_settask(task);
                    
                    return task;
                }
            }
        }
        /* FIXME: try to pull threads from other cores here */
        /* mark the core as idle */
        val = cpu >> __LONGBITSLOG2;
        ndx = cpu & (1UL << __LONGBITSLOG2);
        map = &taskidlecpumap[val];
        m_cmpsetbit(map, ndx);
        k_enabintr();
        m_waitint();
    } while (1);

    return task;
}

/* awaken tasks waiting on wtchan */
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
                    task1 = queue->list;
                    while (task1) {
                        if (task1->next) {
                            task1->next->prev = NULL;
                        }
                        queue->list = task1->next;
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

