#include <kern/conf.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/types.h>
#include <zero/fastidiv.h>
#include <zero/mtx.h>
#include <kern/malloc.h>
#include <kern/sched.h>
#include <kern/cpu.h>
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
void                   schedsetready(struct task *task, long cpu, long unlk);
void                   schedsetsleeping(struct task *task);
void                   schedsetstopped(struct task *task);
void                   schedsetzombie(struct proc *proc);

extern struct cpu    cputab[NCPU];
extern struct divu32 fastu32div24tab[rounduppow2(SCHEDHISTORYSIZE, PAGESIZE)];

/* lookup table to convert nice values to priority offsets */
/* nice is between -20 and 19 inclusively */
/* schedniceptr = &schednicetab[SCHEDNICEHALF]; */
/* prio += schedniceptr[nice]; */
static const long  schednicetab[SCHEDNICERANGE] ALIGNED(CLSIZE)
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
static const long  schedslicetab[SCHEDNICERANGE] ALIGNED(CLSIZE)
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
const long                  *schedniceptr = &schednicetab[SCHEDNICEHALF];
const long                  *schedsliceptr = &schedslicetab[SCHEDNICEHALF];
static struct tasktabl0      schedwaittab[SCHEDNLVL0WAIT] ALIGNED(PAGESIZE);
static struct tasktabl0      scheddeadlinetab[SCHEDNLVL0DL];
static struct task          *schedstoppedtab[NTASK];
static struct proc          *schedzombieproctab[NTASK];
struct task                 *schedreadytab0[NCPU][SCHEDNQUEUE];
struct task                 *schedreadytab1[NCPU][SCHEDNQUEUE];
static long                  schedreadymap0[NCPU][SCHEDREADYMAPNWORD];
static long                  schedreadymap1[NCPU][SCHEDREADYMAPNWORD];
static struct schedqueueset  schedreadytab[NCPU];
static struct task          *schedidletab[NCPU][SCHEDNCLASSQUEUE];
/* SCHEDIDLE queues are not included in SCHEDNQUEUE */
static long                  schedloadmap[NCPU][SCHEDNTOTALQUEUE];
static long                  schedidlecoremap[NCPU][SCHEDIDLECOREMAPNWORD];
static long                  scheddeadlinemap[SCHEDDEADLINEMAPNWORD];
static struct taskqueue      schedsleepqueue;

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

#define SCHEDUNLKTASK (1 << 0)
#define SCHEDUNLKCPU  (1 << 1)

void
schedinitqueues(void)
{
    long                  lim = NCPU;
    struct schedqueueset *queueset = &schedreadytab[0];
    long                  cpu;

    for (cpu = 0 ; cpu < lim ; cpu++) {
        queueset->curmap = &schedreadymap0[cpu][0];
        queueset->nextmap = &schedreadymap1[cpu][0];
        queueset->idlemap = &schedidlecoremap[cpu][0];
        queueset->loadmap = &schedloadmap[cpu][0];
        queueset->cur = &schedreadytab0[cpu][0];
        queueset->next = &schedreadytab1[cpu][0];
        queueset->idle = &schedidletab[cpu][0];
        queueset++;
    }

    return;
}

static __inline__ void
schedswapqueues(long cpu)
{
    struct schedqueueset *queueset = &schedreadytab[cpu];
    void                 *ptr1 = queueset->cur;
    void                 *ptr2 = queueset->next;

    queueset->next = ptr1;
    queueset->cur = ptr2;
    ptr1 = queueset->curmap;
    ptr2 = queueset->nextmap;
    queueset->nextmap = ptr1;
    queueset->curmap = ptr2;

    return;
}

static __inline__ struct cpu *
schedfindidlecore(long cpu, long *retcore)
{
    struct cpu   *unit0 = &cputab[cpu];
    struct cpu   *unit = unit0;
    struct cpu   *end = &cputab[NCPU];
    struct core  *core = NULL;
    long        **map = schedidlecoremap;
    long         *ptr = &map[cpu][0];
    long          nunit = NCPU;
    long          ncore = NCORE;
    long          lim = min(ncore, (long)(CHAR_BIT * sizeof(long)));
    long          ndx = 0;
    long          val = 0;
    long          ntz;
    long          mask;

    mtxlk(&unit->lk);
    for (ndx = 0 ; ndx < lim ; ndx++) {
        mask = *ptr;
        if (mask) {
            ntz = tzerol(mask);
            clrbit(ptr, ntz);
            mtxunlk(&unit->lk);
            ndx *= CHAR_BIT * sizeof(long);
            ndx += ntz;
            *retcore = ndx;

            return unit;
        }
        ptr++;
    }
    mtxunlk(&unit->lk);
    for (unit = &cputab[0] ; unit != unit0 ; unit++) {
        mtxlk(&unit->lk);
        ptr = &map[cpu][0];
        for (ndx = 0 ; ndx < lim ; ndx++) {
            mask = *ptr;
            if (mask) {
                ntz = tzerol(mask);
                clrbit(ptr, ntz);
                mtxunlk(&unit->lk);
                ndx *= CHAR_BIT * sizeof(long);
                ndx += ntz;
                *retcore = ndx;

                return unit;
            }
            ptr++;
        }
        mtxunlk(&unit->lk);
    }
    cpu++;
    unit++;
    for ( ; unit < end ; unit++) {
        mtxlk(&unit->lk);
        ptr = &map[cpu][0];
        for (ndx = 0 ; ndx < lim ; ndx++) {
            mask = *ptr;
            if (mask) {
                ntz = tzerol(mask);
                clrbit(ptr, ntz);
                mtxunlk(&unit->lk);
                ndx *= CHAR_BIT * sizeof(long);
                ndx += ntz;
                *retcore = ndx;

                return unit;
            }
            ptr++;
        }
        mtxunlk(&unit->lk);
    }

    return NULL;
}

static __inline__ void
schedsetnice(struct task *task, long val)
{
    struct proc *proc = task->proc;
    long         nice;
    
    val = max(-20, val);
    val = min(19, val);
    nice = schedniceptr[val];
    proc->nice = nice;
    proc->niceval = val;

    return;
}

/* CHECKED against sched_interact_score() in ULE :) */
static __inline__ long
schedcalcscore(struct task *task)
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
        run = fastu32div24(run, SCHEDSCOREHALF, fastu32div24tab);
#endif
        res = SCHEDSCOREHALF;
        div = max(1, run);
        res <<= 1;
        tmp = fastu32div24(slp, div, fastu32div24tab);
        res -= tmp;
        task->score = res;
        
        return res;
    }
    if (slp > run) {
#if (SCHEDSCOREHALF == 64)
        slp >>= 6;
#else
        slp = fastu32div24(slp, SCHEDHALFSCORE, s);
#endif
        div = max(1, slp);
        res = fastu32div24(run, div, fastu32div24tab);
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

/* based on sched_priority() from ULE */
static __inline__ void
schedcalcprio(struct task *task)
{
    long score = schedcalcscore(task);
    long nice = task->proc->nice;
    long runprio = task->runprio;
    long prio;
    long ntick;
    long delta;
    long tmp;

    score += nice;
    score = max(0, score);
    if (score < SCHEDSCORETHRESHOLD) {
        prio = SCHEDINTPRIOMIN;
        delta = SCHEDINTRANGE;
#if (SCHEDSCORETHRESHOLD == 32)
        delta >>= 5;
#else
        delta = fastu32div24(delta, SCHEDSCORETHRESHOLD, fastu32div24tab);
#endif
        delta *= score;
        prio += delta;
    } else {
        ntick = task->ntick;
        prio = SCHEDUSERPRIOMIN;
        if (ntick) {
            tmp = schedcalcuserprio(task);
            prio += min(tmp, SCHEDUSERRANGE - 1);
        }
        prio += nice;
        prio = min(runprio, prio);
    }
    task->prio = prio;

    return;
}

/* based on sched_interact_update() in ULE :) */
/*
 * enforce maximum limit of scheduling history kept; call after either runtime
 * or slptime is adjusted
 */
static __inline__ void
schedadjintparm(struct task *task)
{
    long run = task->runtime;
    long slp = task->slptime;
    long sum = run + slp;

    if (sum < SCHEDRECTIMEMAX) {

        return;
    }
    if (sum > 2 * SCHEDRECTIMEMAX) {
        if (run > slp) {
            task->runtime = SCHEDRECTIMEMAX;
            task->slptime = 1;
        } else {
            task->runtime = 1;
            task->slptime = SCHEDRECTIMEMAX;
        }

        return;
    }
    if (sum > (SCHEDRECTIMEMAX >> 3) * 9) {
        /* exceeded by more than 1/8th, divide by 2 */
        run >>= 1;
        slp >>= 1;
    } else {
        /* multiply by 3 / 4; this gives us less than 0.85 * SCHEDRECTIMEMAX */
        run >>= 2;
        slp >>= 2;
        run *= 3;
        slp *= 3;
    }
    task->runtime = run;
    task->slptime = slp;

    return;
}

/* based on sched_interact_fork() in ULE */
static __inline__ void
schedadjforkintparm(struct task *task)
{
    long run = task->runtime;
    long slp = task->slptime;
#if !((HZ == 250) && (SCHEDHISTORYNSEC == 8))
    long ratio;
#endif
    long run2;
    long slp2;
    long sum = run + slp;

    if (sum > SCHEDRECTIMEFORKMAX) {
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
        ratio = fastu32div24(sum, SCHEDRECTIMEFORKMAX, fastu32div24tab);
        run = fastu32div24(run, ratio, fastu32div24tab);
        slp = fastu32div24(slp, ratio, fastu32div24tab);
#endif
        task->runtime = run;
        task->slptime = slp;
    }

    return;
}

/* applied for time-share tasks of classes SCHEDRESPONSIVE..SCHEDBATCH */
/* return value is new priority */
static __inline__ long
schedcalcintparm(struct task *task, long *retscore)
{
    long range = SCHEDINTRANGE;
    long res = 0;
    long nice = task->proc->nice;
    long score;
    long diff;
    long ntick;
    long tickhz;
    long total;
    long div;
    long tmp;
    
    score = schedcalcscore(task);
    score += nice;
    score = max(0, score);
    if (score < SCHEDSCORETHRESHOLD) {
        /* map interactive tasks to priorities SCHEDRTMIN..SCHEDBATCHPRIOMIN */
        res = SCHEDINTPRIOMIN;
#if (SCHEDSCORETHRESHOLD == 32)
        range >>= 5;
#else
        range = fastu32div24(range, SCHEDSCORETHRESHOLD, fastu32div24tab);
#endif
        range *= score;
        res += range;
    } else {
        ntick = task->ntick;
        if (ntick) {
            /* map noninteractive tasks to priorities SCHEDSYSTEM..SCHEDBATCH */
            total = task->lastrun - task->firstrun;
            tickhz = ntick >> SCHEDTICKSHIFT;
            diff = SCHEDUSERPRIOMAX + SCHEDNICEHALF;
            res = SCHEDUSERPRIOMIN - SCHEDNICEHALF;
            total = max(total, kgethz());
            range = diff - res + 1;
            tmp = roundup(total, range);
            res += nice;
            div = fastu32div24(total, tmp, fastu32div24tab);
            range--;
            total = fastu32div24(tickhz, div, fastu32div24tab);
            diff = min(total, range);
            res += diff;
        }
    }
    *retscore = score;
    
    return res;
}

/* based on sched_wakeup() from ULE :) */
static __inline__ void
schedwakeup(struct task *task)
{
    long cpu = k_curcpu->id;
    long sched = task->sched;
    long slptick = task->slptick;
    long slp;
    long tick;
    long ntick;
    long diff;

    task->slptick = 0;
    if (slptick) {
        tick = k_curcpu->ntick;
        if (slptick != tick) {
            diff = tick - slptick;
            slp = task->slptime;
            diff <<= SCHEDTICKSHIFT;
            task->slptime = diff;
            schedadjintparm(task);
            schedadjcpupct(task, 0);
        }
    }
    task->slice = 0;
    if (schedistimeshare(sched)) {
        schedcalcprio(task);
    }
#if (SMP)
    cpu = schedfindidlecore(cpu);
#endif
    task->state = TASKREADY;
    task->cpu = cpu;
    schedsetready(task, cpu, SCHEDUNLKTASK);
    /* FIXME: sched_setpreempt() */
}

#define QUEUE_SINGLE_TYPE
#define QUEUE_TYPE      struct task
#include <zero/queue.h>

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

    mtxlk(&scheddeadlinetab[key0].lk);
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
        queueappend(task, &queue->list);
        tab = ptab[0];
        tab->nref++;
        tab->tab = ptab[1];
        tab = ptab[1];
        tab->nref++;
    }
    mtxunlk(&scheddeadlinetab[key0].lk);
    
    return;
}

void
schedsetready(struct task *task, long cpu, long unlk)
{
    long                   sched = task->sched;
    long                   prio = task->prio;
    long                   score = SCHEDSCOREMAX;
    struct schedqueueset  *queueset = &schedreadytab[cpu];
    struct task          **queue = NULL;
    long                  *map = NULL;
    long                   qid = zeroabs(prio);
    long                   load;
    long                   lim;
    long                   flg;

    if (sched <= SCHEDSYSTEM) {
        /* SCHEDDEADLINE, SCHEDINTERRUPT or SCHEDREALTIME */
        if (sched == SCHEDDEADLINE) {
            schedsetdeadline(task);
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
            prio = schedcalcintparm(task, &score);
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
    } else {
        /* SCHEDIDLE */
        /* insert into idle queue */
        qid = schedcalcqueueid(prio);
        map = queueset->idlemap;
        queue = &queueset->idle[qid];
    }
    load = queueset->loadmap[qid];
    queueappend(task, queue);
    load++;
    setbit(map, qid);
    queueset->loadmap[qid] = load;
    if (unlk & SCHEDUNLKTASK) {
        mtxunlk(&task->lk);
    }
    
    return;
}
    
/* add task to wait table */
void
schedsetwait(struct task *task)
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
    void              *ptab[SCHEDNWAITKEY - 1] = { NULL, NULL, NULL };

    key0 = taskwaitkey0(wtchan);
    key1 = taskwaitkey1(wtchan);
    key2 = taskwaitkey2(wtchan);
    key3 = taskwaitkey3(wtchan);
    mtxlk(&schedwaittab[key0].lk);
    l0tab = &schedwaittab[key0];
    ptr = l0tab->tab;
    pptr = ptr;
    if (!ptr) {
        ptr = kmalloc(SCHEDNLVL1WAIT * sizeof(struct tasktab));
        if (ptr) {
            kbzero(ptr, SCHEDNLVL1WAIT * sizeof(struct tasktab));
        }
        l0tab->tab = ptr;
        ptab[0] = ptr;
        pptr = ptr;
    }
    if (ptr) {
        ptr = pptr[key1];
        if (!ptr) {
            ptr = kmalloc(SCHEDNLVL2WAIT * sizeof(struct tasktab));
            if (ptr) {
                kbzero(ptr, SCHEDNLVL2WAIT * sizeof(struct tasktab));
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
            queue = kmalloc(SCHEDNLVL3WAIT * sizeof(struct taskqueue));
            if (queue) {
                kbzero(queue, SCHEDNLVL3WAIT * sizeof(struct taskqueue));
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
    mtxunlk(&schedwaittab[key0].lk);
    
    return;
}

/* FIXME: add a multilevel tree for sleeping tasks for speed */
void
schedsetsleeping(struct task *task)
{
    time_t            timelim = task->timelim;
    struct taskqueue *queue = &schedsleepqueue;
    struct task      *sleeptask;

    if (task->waitchan) {
        schedsetwait(task);
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

    schedzombieproctab[qid] = proc;

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
    struct schedqueueset  *queueset = &schedreadytab[cpu];
    struct task          **queue;
    long                  *map;
    long                   ntz;
    long                   val;
    long                   ndx;
    long                   lim;
    long                   loop;

    if (curtask) {
        if (state != TASKNEW) {
            switch (state) {
                case TASKREADY:
                    schedsetready(curtask, cpu, 0);

                    break;
                case TASKSLEEPING:
                    schedsetsleeping(curtask);

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
    void              *ptab[SCHEDNWAITKEY - 1] = { NULL, NULL, NULL };
    void             **pptab[SCHEDNWAITKEY - 1] = { NULL, NULL, NULL };

    mtxlk(&schedwaittab[key0].lk);
    l0tab = &schedwaittab[key0];
    if (l0tab) {
        ptab[0] = l0tab;
        pptab[0] = (void **)&schedwaittab[key0];
        tab = ((void **)l0tab)[key1];
        if (tab) {
            ptab[1] = tab;
            pptab[1] = (void **)&tab[key1];
            tab = ((void **)tab)[key2];
            if (tab) {
                ptab[2] = tab;
                pptab[2] = (void **)&tab[key2];
                queue = ((void **)tab)[key3];
                if (queue) {
                    task1 = queue->list;
                    while (task1) {
                        mtxlk(&task1->lk);
                        if (task1->next) {
                            task1->next->prev = NULL;
                        }
                        queue->list = task1->next;
                        task2 = task1->next;
                        schedwakeup(task1);
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
    mtxunlk(&schedwaittab[key0].lk);
}

#endif /* ZEROSCHED */

