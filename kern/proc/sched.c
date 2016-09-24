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

void                   schedinitqueues(void);
FASTCALL struct task * schedswitchtask(struct task *curtask);
void                   schedsetready(struct task *task, long cpu);
void                   schedsetstopped(struct task *task);
void                   schedsetzombie(struct proc *proc);

extern struct cpu        cputab[NCPU];
extern struct proc      *proczombietab[NTASK];
extern struct tasktabl0  taskwaittab[TASKNLVL0WAIT] ALIGNED(PAGESIZE);
extern struct divu32     fastu32div24tab[rounduppow2(SCHEDHISTORYSIZE,
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
struct task             *schedreadytab0[NCPU][SCHEDNQUEUE];
struct task             *schedreadytab1[NCPU][SCHEDNQUEUE];
static m_atomic_t        schedreadymap0[NCPU][SCHEDREADYMAPNWORD];
static m_atomic_t        schedreadymap1[NCPU][SCHEDREADYMAPNWORD];
struct schedqueueset     schedreadytab[NCPU];
static struct task      *schedidletab[NCPU][SCHEDNCLASSQUEUE];
/* SCHEDIDLE queues are not included in SCHEDNQUEUE */
static m_atomic_t        schedloadmap[NCPU][SCHEDNTOTALQUEUE];
m_atomic_t               schedidlecoremap[NCPU][SCHEDIDLECOREMAPNWORD];
static m_atomic_t        scheddeadlinemap[SCHEDDEADLINEMAPNWORD];

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
    fastu32div24gentab(fastu32div24tab, SCHEDHISTORYSIZE);
    schedinitqueues();

    return;
}

void
schedyield(void)
{
    struct task *oldtask = k_curtask;
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
schedinitqueues(void)
{
    long                  lim = NCPU;
    struct schedqueueset *set = &schedreadytab[0];
    long                  cpu;

    for (cpu = 0 ; cpu < lim ; cpu++) {
        set->curmap = &schedreadymap0[cpu][0];
        set->nextmap = &schedreadymap1[cpu][0];
        set->idlemap = &schedidlecoremap[cpu][0];
        set->loadmap = &schedloadmap[cpu][0];
        set->cur = &schedreadytab0[cpu][0];
        set->next = &schedreadytab1[cpu][0];
        set->idle = &schedidletab[cpu][0];
        set++;
    }

    return;
}

#define DEQ_SINGLE_TYPE
#define DEQ_TYPE struct task
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
    m_atomic_t        *map = scheddeadlinemap;
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
schedsetready(struct task *task, long cpu)
{
    long                   sched = task->sched;
    long                   prio = task->prio;
    long                   score = SCHEDSCOREMAX;
    struct schedqueueset  *set = &schedreadytab[cpu];
    struct task          **queue = NULL;
    m_atomic_t            *map = NULL;
    long                   qid = zeroabs(prio);
    long                   load;
    long                   lim;
    long                   flg;

    if (sched <= SCHEDSYSTEM) {
        /* SCHEDDEADLINE, SCHEDINTERRUPT or SCHEDREALTIME */
        if (sched == SCHEDDEADLINE) {
            schedsetdeadline(task);

            return;
        } else {
            /* insert onto current queue */
            fmtxlk(&set->lk);
            qid = schedcalcqueueid(qid);
            map = set->curmap;
            queue = &set->cur[qid];
            if (prio < 0) {
                /* SCHEDREALTIME with SCHED_FIFO */
                deqpush(task, queue);
            } else {
                /* SCHEDINTERRUPT, SCHEDREALTIME with SCHED_RR, SCHEDSYSTEM */
                deqappend(task, queue);
            }
            setbit(map, qid);
            fmtxunlk(&set->lk);

            return;
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
            prio = schedcalcintparm(task, &score);
//            prio = min(task->runprio, prio);
        }
        qid = schedcalcqueueid(prio);
        fmtxlk(&set->lk);
        if (schedisinteract(score)) {
            /* if interactive, insert onto current queue */
            queue = &set->cur[qid];
            map = set->curmap;
        } else {
            /* if not interactive, insert onto next queue */
            queue = &set->next[qid];
            map = set->nextmap;
        }
        task->score = score;
    } else {
        /* SCHEDIDLE */
        /* insert into idle queue */
        qid = schedcalcqueueid(prio);
        fmtxlk(&set->lk);
        map = set->idlemap;
        queue = &set->idle[qid];
    }
    load = set->loadmap[qid];
    deqappend(task, queue);
    load++;
    setbit(map, qid);
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
    long                   cpu = k_curcpu->id;
    struct task           *task = NULL;
    long                   state = (curtask) ? curtask->state : -1;
    struct schedqueueset  *set = &schedreadytab[cpu];
    struct task          **queue;
    m_atomic_t            *map;
    long                   ntz;
    long                   val;
    long                   ndx;
    long                   lim;
    long                   loop;

    if (curtask) {
        if (state != TASKNEW) {
            switch (state) {
                case TASKREADY:
                    schedsetready(curtask, cpu);

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
        } else {
            ; /* TODO */
        }
    }
    do {
        loop = 1;
        do {
            /* loop over current and next priority-queues */
            lim = rounduppow2(SCHEDNQUEUE, CHAR_BIT * sizeof(long));
            map = set->curmap;
            queue = &set->cur[0];
            for (ndx = 0 ; ndx < lim ; ndx++) {
                val = map[ndx];
                if (val) {
                    ntz = tzerol(val);
                    ndx <<= __LONGBITSLOG2;
                    ntz += ndx;
                    queue += ntz;
                    task = deqpop(queue);
                    if (task) {
                        if (deqisemptyptr(queue)) {
                            clrbit(map, ntz);
                        }
                        schedsettask(task);

                        return task;
                    }
                }
            }
            if (loop) {
                /* if no task found during the first iteration, switch queues */
                schedswapqueues(cpu);
            }
        } while (loop--);
        /* if both current and next queues are empty, look for an idle task */
        lim = rounduppow2(SCHEDNIDLE, CHAR_BIT * sizeof(long));
        map = set->idlemap;
        queue = &set->idle[0];
        for (ndx = 0 ; ndx < lim ; ndx++) {
            val = map[ndx];
            if (val) {
                ntz = tzerol(val);
                ndx <<= __LONGBITSLOG2;
                ntz += ndx;
                queue += ntz;
                task = deqpop(queue);
                if (task) {
                    if (deqisemptyptr(queue)) {
                        clrbit(map, ntz);
                    }
                    schedsettask(task);
                    
                    return task;
                }
            }
        }
        /* FIXME: try to pull threads from other cores here */
//        task = taskpull(cpu);
        /* mark the core as idle */
        val = cpu >> __LONGBITSLOG2;
        ndx = cpu & (1UL << __LONGBITSLOG2);
        map = &schedidlecoremap[cpu][val];
        m_cmpsetbit(map, ndx);
        k_enabintr();
        m_waitint();
    } while (1);

    return task;
}

#endif /* ZEROSCHED */

