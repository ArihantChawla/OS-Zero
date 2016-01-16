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

void tasksetready(struct task *task);
void tasksetsleeping(struct task *task);
void tasksetstopped(struct task *task);
void tasksetzombie(struct task *task);

extern struct divul          scheddivultab[SCHEDHISTORYSIZE];

struct taskqueue             taskreadytab0[NCPU][SCHEDNQUEUE] ALIGNED(PAGESIZE);
struct taskqueue             taskreadytab1[NCPU][SCHEDNQUEUE];
static struct taskqueuepair  taskreadytab[NCPU];
static struct tasktabl0      taskwaittab[TASKNLVL0WAIT] ALIGNED(PAGESIZE);
static struct tasktabl0      taskdeadlinetab[TASKNLVL0DL];
static struct task          *taskstoppedtab[NTASK];
static struct task          *taskzombietab[NTASK];
static long                  taskdeadlinemap[TASKDEADLINEMAPNWORD];
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
static struct taskqueue     taskidletab[SCHEDNCLASSQUEUE];
static struct taskqueuehdr  taskidlehdr;
static long                 taskidlemap[TASKIDLEMAPNWORD];
static struct taskqueue     tasksleepqueue;
static long                 taskreadymap0[NCPU][TASKREADYMAPNWORD];
static long                 taskreadymap1[NCPU][TASKREADYMAPNWORD];
typedef void tasksetfunc_t(struct task *);
tasksetfunc_t              *tasksetfunctab[TASKNSTATE]
= {
    NULL,               // TASKNEW
    tasksetready,       // TASKREADY
    tasksetsleeping,    // TASKSLEEPING
    tasksetstopped,     // TASKSTOPPED
    tasksetzombie       // TASKZOMBIE
};
static long             *taskniceptr = &tasknicetab[32];

void
taskinitsched(void)
{
    long                  lim = NCPU;
    struct taskqueuepair *pair = &taskreadytab[0];
    long                  cpu;

    for (cpu = 0 ; cpu < lim ; cpu++) {
        pair->cur = &taskreadytab0[cpu][0];
        pair->next = &taskreadytab1[cpu][0];
        pair->curmap = &taskreadymap0[cpu][0];
        pair->nextmap = &taskreadymap1[cpu][0];
        pair++;
    }
    taskidlehdr.tab = taskidletab;

    return;
}

void
taskswapqueues(long cpu)
{
    struct taskqueuepair *pair = &taskreadytab[cpu];
    
    ptrswap(pair->cur, pair->next);
    ptrswap(pair->curmap, pair->nextmap);

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
tasksetnice(struct task *task, long val)
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
    unsigned long run = task->runtime;
    unsigned long slp = task->slptime;
    unsigned long lim = SCHEDHISTORYSIZE;
    unsigned long sum = run + slp;

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
    unsigned long run = task->runtime;
    unsigned long slp = task->slptime;
    unsigned long div;
    unsigned long res;

    if (SCHEDSCORETHRESHOLD <= SCHEDSCOREHALF
        && run >= slp) {

        return SCHEDSCOREHALF;
    }
    if (slp > run) {
        div = max(1, slp >> 6);
        res = fastuldiv32(run, div, scheddivultab);

        return res;
    }
    if (run > slp) {
        res = SCHEDSCOREMAX;
        div = max(1, run >> 6);
        res -= fastuldiv32(slp, div, scheddivultab);

        return res;
    }
    /* run == slp */
    if (run) {

        return SCHEDSCOREHALF;
    }

    /* run == 0 && slp == 0 */
    return 0;
}

/* applied for time-share tasks of classes SCHEDRESPONSIVE and SCHEDNORMAL */
static __inline__ long
taskcalcintparm(struct task *task)
{
    unsigned long score;
    long          prio = task->prio;
    unsigned long delta = SCHEDINTPRIOMAX - SCHEDINTPRIOMIN + 1;
    unsigned long diff;
    unsigned long ntick;
    unsigned long tickhz;
    unsigned long total;
    unsigned long div;
    unsigned long tmp;
    
    score = taskcalcscore(task);
    score += task->nice;
    score = max(SCHEDINTPRIOMIN, score);
    if (score < SCHEDSCORETHRESHOLD) {
        prio = SCHEDINTPRIOMIN;
        delta = fastuldiv32(delta, SCHEDSCORETHRESHOLD, scheddivultab);
        delta *= score;
        prio += delta;
    } else {
        ntick = task->ntick;
        total = task->lasttick - task->firsttick;
        tickhz = ntick >> SCHEDTICKSHIFT;
        delta = SCHEDBATCH - (SCHEDNCLASSQUEUE >> 1);
        prio = SCHEDBATCH + (SCHEDNCLASSQUEUE >> 1);
        diff = delta - prio + 1;
        if (ntick) {
            tmp = roundup(total, diff);
            div = fastuldiv32(tmp, diff, scheddivultab);
            prio += task->nice;
            tmp = fastuldiv32(tickhz, div, scheddivultab);
            delta = min(delta, tmp);
            prio += delta;
        }
    }
    task->score = score;
    
    return prio;
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
tasksetready(struct task *task)
{
    long              cpu = k_curcpu->id;
    long              sched = task->sched;
    long              prio = task->prio;
    struct taskqueue *queue;
    long             *map;
    long              lim;
    long              flg;

    if (sched == SCHEDFIXED) {
        /* insert into fixed-priority queue */
        queue = taskreadytab[cpu].cur;
        map = taskreadytab[cpu].curmap;
        queue += prio;
        queueappend(task, &queue);
        setbit(map, prio);
    } else if (sched <= SCHEDREALTIME) {
        /* SCHEDDEADLINE, SCHEDFIXED, SCHEDINTERRUPT or SCHEDREALTIME */
        if (sched == SCHEDDEADLINE) {
            tasksetdeadline(task);
        } else {
            /* insert onto current queue */
            queue = taskreadytab[cpu].cur;
            map = taskreadytab[cpu].curmap;
            if (sched == SCHEDINTERRUPT) {
                queue += prio;
                queueappend(task, &queue);
                setbit(map, prio);
            } else if (prio < 0) {
                long qid = -prio;
                
                /* SCHED_FIFO */
                queue -= prio;
                queuepush(task, &queue);
                setbit(map, qid);
            } else {
                /* SCHED_RR */
                queue += prio;
                queueappend(task, &queue);
                setbit(map, prio);
            }
        }
    } else if (sched != SCHEDIDLE) {
        /* SCHEDRESPONSIVE..SCHEDBATCH */
        flg = task->schedflg;
        if (flg & TASKHASINPUT) {
            /* boost user-interrupt task to highest priority */
//            task->sched = SCHEDRESPONSIVE;
            prio = SCHEDUSERPRIOMIN;
            task->prio = prio;
        } else if (schedistimeshare(sched)) {
            /* SCHEDRESPONSIVE or SCHEDNORMAL; calculate timeshare priority */
            prio = taskcalcintparm(task);
            prio >>= 2;
        } else {
            /* SCHEDBATCH; increment priority by one */
            lim = SCHEDNCLASS * SCHEDNCLASSPRIO - 1;
            prio++;
            prio = min(prio, lim);
            task->prio = prio;
            prio >>= 2;
        }
        if (task->score) {
            /* if interactive, insert onto current queue */
            queue = taskreadytab[cpu].cur;
            map = taskreadytab[cpu].curmap;
        } else {
            /* if not interactive, insert onto next queue */
            queue = taskreadytab[cpu].next;
            map = taskreadytab[cpu].nextmap;
        }
        queue += prio;
        queueappend(task, &queue);
        setbit(map, prio);
    } else {
        mtxlk(&taskidlehdr.lk);
        /* insert into idle queue */
        queue = &taskidlehdr.tab[prio];
        map = taskidlemap;
        /* SCHEDIDLE */
        queueappend(task, &queue);
        setbit(map, prio);
        mtxunlk(&taskidlehdr.lk);
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
tasksetsleeping(struct task *task)
{
    time_t            timelim = task->timelim;
    struct taskqueue *queue = &tasksleepqueue;
    struct task      *sleeptask;

    if (task->waitchan) {
        tasksetwait(task);
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
tasksetstopped(struct task *task)
{
    long id = task->id;

    taskstoppedtab[id] = task;

    return;
}

void
tasksetzombie(struct task *task)
{
    long id = task->id;

    taskzombietab[id] = task;

    return;
}

/* switch tasks */
FASTCALL
struct task *
taskpick(struct task *curtask)
{
    long              cpu = k_curcpu->id;
    struct task      *task = NULL;
    long              sched = curtask->sched;
    long              state = curtask->state;
//    long             *map = &taskreadymap0[cpu][0];
    struct taskqueue *queue;
    long             *map;
    tasksetfunc_t    *func;
    long              ndx;
    long              val;
    long              qid;
    long              lim;
    long              loop;

    if (curtask) {
        if (sched >= SCHEDINTERRUPT) {
            func = tasksetfunctab[state];
            func(curtask);
            k_curtask = NULL;
        } else {
            tasksetdeadline(task);
        }
    }
    do {
        loop = 1;
        do {
            lim = rounduppow2(SCHEDNQUEUE,
                              CHAR_BIT * sizeof(long));
            queue = taskreadytab[cpu].cur;
            map = taskreadytab[cpu].curmap;
            for (ndx = 0 ; ndx < lim ; ndx += CHAR_BIT * sizeof(long)) {
                val = map[ndx];
                if (val) {
                    qid = tzerol(val);
                    qid += ndx * sizeof(long) * CHAR_BIT;
                    queue += qid;
                    task = queuepop(&queue);
                    if ((task) && queueisempty(&queue)) {
                        clrbit(map, qid);
                    }
                    if (task) {
                        
                        break;
                    }
                }
            }
            if (!task) {
                lim = rounduppow2(SCHEDNCLASSQUEUE, CHAR_BIT * sizeof(long));
                mtxlk(&taskidlehdr.lk);
                map = taskidlemap;
                for (ndx = 0 ; ndx < lim ; ndx += CHAR_BIT * sizeof(long)) {
                    val = map[ndx];
                    if (val) {
                        qid = tzerol(val);
                        qid += ndx * sizeof(long) * CHAR_BIT;
                        queue = &taskidlehdr.tab[qid];
                        task = queuepop(&queue);
                        if ((task) && queueisempty(&queue)) {
                            clrbit(map, qid);
                        }
                        if (task) {

                            break;
                        }
                    }
                }
                mtxunlk(&taskidlehdr.lk);
            }
            if ((loop) && !task) {
                taskswapqueues(cpu);
            }
        } while ((loop--) && !task);
        if (!task) {
            k_enabintr();
            m_waitint();
        }
    } while (!task);
    k_curtask = task;
    k_curproc = task->proc;
    k_curpid = task->id;
//    taskjmp(task);

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
                        tasksetready(task1);
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

