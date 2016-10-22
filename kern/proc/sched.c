#include <kern/conf.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/types.h>
#include <zero/fastidiv.h>
#include <zero/mtx.h>
#include <kern/malloc.h>
#include <kern/cpu.h>
#include <kern/sched.h>
#include <kern/proc/proc.h>
#include <kern/proc/task.h>
#include <kern/unit/x86/asm.h>
#if (!APIC)
#include <sys/io.h>
#include <kern/unit/x86/pic.h>
#endif
#include <kern/unit/ia32/task.h>

void                   schedinitset(void);
FASTCALL struct task * schedswitchtask(struct task *curtask);
void                   schedsetready(struct task *task);
void                   schedsetstopped(struct task *task);
void                   schedsetzombie(struct proc *proc);

extern struct cpu        cputab[NCPU];
extern struct proc      *proczombietab[NTASK];
extern struct tasktabl0  taskwaittab[TASKNLVL0WAIT] ALIGNED(PAGESIZE);
extern struct divu16     fastu32div16tab[rounduppow2(SCHEDHISTORYSIZE,
                                                     PAGESIZE)];

/* lookup table to convert nice values to priority offsets */
/* nice is between -20 and 19 inclusively */
/* schedniceptr = &schednicetab[SCHEDNICEHALF]; */
/* prio += schedniceptr[nice]; */
long schednicetab[SCHEDNICERANGE] ALIGNED(CLSIZE)
= {
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -64, -60, -57, -54, -51,
    -47, -44, -41, -38, -35, -31, -28, -25,
    -22, -19, -15, -12, -9, -6, -3, 0,
    3, 6, 9, 13, 16, 19, 23, 26,
    29, 33, 36, 39, 43, 46, 49, 53,
    56, 59, 63, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};
/* lookup table to convert nice values to slices in 4-ms ticks */
/* nice is between -20 and 19 inclusively */
/* schedsliceptr = &schedslicetab[SCHEDNICEHALF]; */
/* slice = schedsliceptr[nice]; */
long schedslicetab[SCHEDNICERANGE] ALIGNED(CLSIZE)
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

long                    *schedniceptr = &schednicetab[SCHEDNICEHALF];
long                    *schedsliceptr = &schedslicetab[SCHEDNICEHALF];
static struct tasktabl0  scheddeadlinetab[SCHEDNLVL0DL];
static struct task      *schedstoppedtab[NTASK];
struct task             *schedreadytab0[SCHEDNQUEUE];
struct task             *schedreadytab1[SCHEDNQUEUE];
static struct task      *schedidletab[SCHEDNIDLE];
static long              schedreadymap0[SCHEDREADYMAPNWORD];
static long              schedreadymap1[SCHEDREADYMAPNWORD];
static long              schedidlemap[SCHEDIDLEMAPNWORD];
/* SCHEDIDLE queues are not included in SCHEDNQUEUE */
static long              schedloadmap[SCHEDLOADMAPNWORD];
static long              scheddeadlinemap[SCHEDDEADLINEMAPNWORD];
long                     schedidlecoremap[SCHEDIDLECOREMAPNWORD];
struct schedqueueset     schedreadyset;

void
schedinit(void)
{
#if 0
#if (ZEROSCHED)
    schedswtchtask = schedswitchtask;
#else
#error define supported scheduler such as ZEROSCHED
#endif
#endif
    kprintf("SCHEDHISTORYSIZE == %ld\n", SCHEDHISTORYSIZE);
    fastu32div16gentab(fastu32div16tab, SCHEDHISTORYSIZE);
    schedinitset();

    return;
}

void
schedyield(void)
{
    struct task *oldtask = (struct task *)k_curtask;
    struct task *task = NULL;

    task = schedswitchtask(oldtask);
    if (task != oldtask) {
        m_taskjmp(&task->m_task);
    } else {

        return;
    }

    /* NOTREACHED */
    for ( ; ; ) { ; }
}

#if (ZEROSCHED)

#define __LONGBITS     (CHAR_BIT * LONGSIZE)
#if (__LONGBITS == 32)
#define __LONGBITSLOG2 5
#elif (__LONGBITS == 64)
#define __LONGBITSLOG2 6
#endif

void
schedinitset(void)
{
    long                  lim = NCPU;
    struct schedqueueset *set = &schedreadyset;
    long                  unit = k_curcpu->unit;
    long                  id;

    lim >>= __LONGBITSLOG2;
    for (id = 0 ; id < lim ; id++) {
        schedidlecoremap[id] = ~1L;
    }
    clrbit(schedidlecoremap, unit);
    set->curmap = schedreadymap0;
    set->nextmap = schedreadymap1;
    set->idlemap = schedidlemap;
    set->loadmap = schedloadmap;
    set->cur = schedreadytab0;
    set->next = schedreadytab1;
    set->idle = schedidletab;

    return;
}

#define DEQ_SINGLE_TYPE
#define DEQ_TYPE      struct task
#include <zero/deq.h>

void
schedsetdeadline(struct task *task)
{
    struct tasktabl0  *l0tab;
    struct tasktab    *tab;
    time_t             deadline = task->timelim;
    unsigned long      key0 = taskdlkey0(deadline);
    unsigned long      key1 = taskdlkey1(deadline);
    unsigned long      key2 = taskdlkey2(deadline);
    long              *map = scheddeadlinemap;
    void              *ptr = NULL;
    void             **pptr = NULL;
    long               fail = 0;
    struct taskqueue  *queue;
    void              *ptab[SCHEDNDLKEY - 1] = { NULL, NULL };

    fmtxlk(&scheddeadlinetab[key0].lk);
    l0tab = &scheddeadlinetab[key0];
    ptr = l0tab->tab;
    pptr = ptr;
    if (!ptr) {
        ptr = kmalloc(SCHEDNLVL1DL * sizeof(struct tasktab));
        if (ptr) {
            kbzero(ptr, SCHEDNLVL1DL * sizeof(struct tasktab));
        }
        l0tab->tab = ptr;
        ptab[0] = ptr;
        pptr = ptr;
    }
    if (ptr) {
        ptr = pptr[key1];
        if (!ptr) {
            queue = kmalloc(SCHEDNLVL2DL * sizeof(struct taskqueue));
            if (queue) {
                kbzero(queue, SCHEDNLVL2DL * sizeof(struct taskqueue));
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
        deqappend(task, &queue->list);
        tab = ptab[0];
        tab->nref++;
        tab->tab = ptab[1];
        tab = ptab[1];
        tab->nref++;
    }
    fmtxunlk(&scheddeadlinetab[key0].lk);
    
    return;
}

void
schedsetready(struct task *task)
{
    long                   sched = task->sched;
    long                   prio = task->prio;
    struct schedqueueset  *set = &schedreadyset;
    struct task          **queue;
    long                  *map;
    long                   score;
    long                   type;
    long                   ndx;
    long                   qid;
    long                   load;
    long                   lim;
    long                   flg;

    type = sched;
    if (sched == SCHEDDEADLINE) {
        /* SCHEDDEADLINE */
        schedsetdeadline(task);

        return;
    } else if (sched <= SCHEDSYSTEM) {
        /* SCHEDTRAP, SCHEDREALTIME or SCHEDSYSTEM */
        if (sched == SCHEDREALTIMERR) {
            type = SCHEDREALTIME;
        }
        if (type == SCHEDTRAP) {
            qid = schedcalctrapqueue(prio);
        } else if (type == SCHEDREALTIME) {
            qid = schedcalcqueue(type, prio);
        } else {
            /* SCHEDSYSTEM */
            lim = schedclassmaxprio(SCHEDSYSTEM);
            if (prio < lim) {
                prio++;
                task->prio = prio;
            }
            qid = schedcalcqueue(SCHEDSYSTEM, prio);
        }
        fmtxlk(&set->lk);
        map = set->curmap;
        queue = &set->cur[qid];
        setbit(map, qid);
    } else if (sched < SCHEDIDLE) {
        /* SCHEDNORMAL, SCHEDBATCH */
        flg = task->flg;
        score = 0;
        type = sched;
        if (task->sched == SCHEDRESPONSIVE) {
            lim = schedclassmaxprio(SCHEDRESPONSIVE);
            prio++;
            if (prio == lim) {
                type = SCHEDNORMAL;
            }
        } else if (flg & TASKHASINPUT) {
            /* boost user-interrupt task to highest priority */
            sched = SCHEDRESPONSIVE;
            prio = schedclassminprio(SCHEDRESPONSIVE);
        } else {
            /* SCHEDNORMAL or SCHEDBATCH; calculate timeshare priority */
            prio = schedcalcintparm(task, &score);
        }
        task->sched = type;
        task->prio = prio;
        qid = schedcalcqueue(type, prio);
        if (schedisinteract(score)) {
            /* if interactive, insert onto current queue */
            fmtxlk(&set->lk);
            map = set->curmap;
            queue = &set->cur[qid];
        } else {
            /* if not interactive, insert onto next queue */
            fmtxlk(&set->lk);
            map = set->nextmap;
            queue = &set->next[qid];
        }
        setbit(map, qid);
    } else {
        /* SCHEDIDLE */
        lim = SCHEDNCLASS * SCHEDNCLASSPRIO + SCHEDNIDLE - 1;
        if (prio < lim) {
            prio++;
            task->prio = prio;
        }
        qid = schedcalcqueue(SCHEDIDLE, prio);
        ndx = qid - schedclassminqueue(SCHEDIDLE);
        fmtxlk(&set->lk);
        map = set->idlemap;
        queue = &set->idle[ndx];
        setbit(map, ndx);
    }
    load = set->loadmap[qid];
    load++;
    if (sched != SCHEDREALTIMERR) {
        deqappend(task, queue);
    } else {
        /* SCHEDREALTIME with SCHED_FIFO */
        deqpush(task, queue);
    }
    set->loadmap[qid] = load;
    fmtxunlk(&set->lk);
    
    return;
}
    
void
schedsetstopped(struct task *task)
{
    long id = task->id;

    schedstoppedtab[id] = task;

    return;
}

void
schedsetzombie(struct proc *proc)
{
    long qid = proc->ppid;      // FIXME: might need to use proc->pgrp here

    proczombietab[qid] = proc;

    return;
}

/* switch tasks */
FASTCALL
struct task *
schedswitchtask(struct task *curtask)
{
    long                   unit = k_curcpu->unit;
    long                   state = (curtask) ? curtask->state : -1;
    struct schedqueueset  *set = &schedreadyset;
    struct task           *task;
    struct task           *next;
    struct task          **queue;
    long                  *map;
    long                   val;
    long                   ndx;
    long                   ofs;
    long                   lim;
    long                   loop;

    switch (state) {
        case TASKNEW:
        case TASKREADY:
            schedsetready(curtask);
            
            break;
        case TASKSLEEPING:
            schedsetsleep(curtask);
            
            break;
        case TASKSTOPPED:
            schedsetstopped(curtask);
            
            break;
        case TASKZOMBIE:
            schedsetzombie(curtask->proc);
            
            break;
        default:
            panic(curtask->id, -1, 0); /* FIXME: error # */
            
            break;
    }
    do {
        loop = 1;
        do {
            /* loop over current and next priority-queues */
            lim = SCHEDNQUEUE;
            lim >>= __LONGBITSLOG2;
            fmtxlk(&set->lk);
            map = set->curmap;
            queue = &set->cur[SCHEDTRAPPRIOMIN];
            for (ndx = 0 ; ndx < lim ; ndx++) {
                val = map[ndx];
                if (val) {
                    ofs = ndx * CHAR_BIT * sizeof(long);
                    ndx = tzerol(val);
                    ofs += ndx;
                    if (ofs < SCHEDNQUEUE) {
                        queue += ofs;
                        task = deqpop(queue);
                        if (!queue) {
                            m_clrbit((m_atomic_t *)map, ofs);
                        }
                        fmtxunlk(&set->lk);
                    
                        return task;
                    }
                }
            }
            if (loop) {
                /* if no task found during the first iteration, switch queues */
                schedswapqueues();
            }
            fmtxunlk(&set->lk);
        } while (loop--);
        /* if both current and next queues are empty, look for an idle task */
        lim = SCHEDNIDLE;
        lim >>= __LONGBITSLOG2;
        fmtxlk(&set->lk);
        map = &set->idlemap[0];
        queue = set->idle;
        for (ndx = 0 ; ndx < lim ; ndx++) {
            val = map[ndx];
            if (val) {
                ofs = ndx * CHAR_BIT * sizeof(long);
                ndx = tzerol(val);
                ofs += ndx;
                if (ofs < SCHEDNIDLE) {
                    queue += ofs;
                    task = deqpop(queue);
                    if (!queue) {
                        m_clrbit((m_atomic_t *)map, ofs);
                    }
                    fmtxunlk(&set->lk);
                    
                    return task;
                }
            }
        }
        /* FIXME: try to pull threads from other cores here */
//        task = taskpull(unit);
        /* mark the core as idle */
        map = &set->idlemap[0];
        setbit(map, unit);
        fmtxunlk(&set->lk);
        k_enabintr();
        m_waitint();
    } while (1);

    /* NOTREACHED */
    return NULL;
}

#endif /* ZEROSCHED */

