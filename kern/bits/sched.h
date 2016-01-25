#ifndef __KERN_BITS_SCHED_H__
#define __KERN_BITS_SCHED_H__

#include <kern/conf.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/mtx.h>
#include <zero/fastidiv.h>
//#include <kern/sched.h>
#include <kern/proc/task.h>

/* unlk-argument values for schedsetready() */
#define SCHEDUNLKTASK (1 << 0)
#define SCHEDUNLKCPU  (1 << 1)

#define schedlkcpuntick(cpu)     (mtxlk(&cpu->lk), (cpu)->ntick)
#define schedlkcpu(cpu)          (mtxlk(&cpu->lk))
#define schedunlkcpu(cpu)        (mtxunlk(&cpu->lk))
#define schedlktaskruntime(task) (mtxlk(&task->lk), (task)->runtime)
#define schedlktask(task)        (mtxlk(&task->lk))
#define schedunlktask(task)      (mtxunlk(&task->lk))

#if defined(ZEROSCHED)

#if !defined(HZ)
#define HZ                  250
#endif

/* macros */
#define schedsettask(task)                                              \
    do {                                                                \
        k_curtask = (task);                                             \
        k_curproc = (task)->proc;                                       \
        k_curpid = (task)->id;                                          \
    } while (0)

#define schedcalctime(task)  ((task)->ntick >> SCHEDTICKSHIFT)
#define schedcalcticks(task) (max((task)->lastrun - (task)->firstrun, kgethz()))
#define schedcalcnice(task)  (tasknicetab[(task)->nice])
#define schedcalcbaseprio(task, sched)                                  \
    ((sched) * SCHEDNCLASSPRIO)
#if 0
#define schedsetprio(task, pri)                                         \
    ((task)->prio = schedprioqueueid(pri))
#define schedsetprioincr(task, pri, incr)                               \
    ((task)->prio = schedprioqueueid(pri) + (incr))
#endif
#define schedcalcuserprio(task)                                         \
    (fastu32div24(schedcalctime(task),                                  \
                  (roundup(schedcalcticks(task), SCHEDPRIORANGE)        \
                   / SCHEDPRIORANGE),                                   \
                  fastu32div24tab))
/* timeshare-tasks have interactivity scores */
#define schedistimeshare(sched)                                         \
    ((sched) >= SCHEDRESPONSIVE || (sched) <= SCHEDBATCH)
#define schedisinteract(score)        ((score) < SCHEDSCORETHRESHOLD)
#define schedcalcqueueid(pri)         ((pri) >> 1)
#define schedcalcqueueidofs(pri, ofs) (schedcalcqueueid(pri) + (ofs))

/* interrupt priorities */
#define schedintrsoftprio(id)                                           \
    (SCHEDINTRSOFTPRIO + (id) * SCHEDNQUEUEPRIO)
#define SCHEDINTRRTPRIO     SCHEDINTRPRIOMIN
#define SCHEDINTRAVPRIO     (SCHEDINTRPRIOMIN + SCHEDNQUEUEPRIO)
#define SCHEDINTRHIDPRIO    (SCHEDINTRPRIOMIN + 2 * SCHEDNQUEUEPRIO)
#define SCHEDINTRNETPRIO    (SCHEDINTRPRIOMIN + 3 * SCHEDNQUEUEPRIO)
#define SCHEDINTRDISKPRIO   (SCHEDINTRPRIOMIN + 4 * SCHEDNQUEUEPRIO)
#define SCHEDINTRMISCPRIO   (SCHEDINTRPRIOMIN + 5 * SCHEDNQUEUEPRIO)
#define SCHEDINTRSOFTPRIO   (SCHEDINTRPRIOMIN + 6 * SCHEDNQUEUEPRIO)

#define SCHEDNLVL0DL      (1U << 16)
#define SCHEDNLVL1DL      (1U << 8)
#define SCHEDNLVL2DL      (1U << 8)
#define SCHEDNDLKEY       3

/* 32-bit time_t values */
#define taskdlkey0(dl)    (((dl) >> 16) & 0xffff)
#define taskdlkey1(dl)    (((dl) >> 8) & 0xff)
#define taskdlkey2(dl)    ((dl) & 0xff)

#define SCHEDDEADLINEMAPNWORD (SCHEDNLVL0DL / (CHAR_BIT * sizeof(long)))
#define SCHEDREADYMAPNWORD    max(SCHEDNQUEUE / (CHAR_BIT * sizeof(long)), \
                                 CLSIZE / sizeof(long))
#define SCHEDIDLEMAPNWORD     max(SCHEDNIDLE / (CHAR_BIT * sizeof(long)), \
                                  CLSIZE / sizeof(long))
#define SCHEDLOADMAPNWORD     max((SCHEDNTOTALQUEUE) / CHAR_BIT * sizeof(long), \
                                  CLSIZE / sizeof(long))
#define SCHEDIDLECOREMAPNWORD  max(NCORE / (CHAR_BIT * sizeof(long)),   \
                                   CLSIZE / sizeof(long))

/* data structures */

struct schedqueueset {
    volatile long   lk;
    long           *curmap;
    long           *nextmap;
    long           *idlemap;
    long           *loadmap;
    struct task   **cur;
    struct task   **next;
    struct task   **idle;
    uint8_t         pad[CLSIZE - sizeof(long) - 6 * sizeof(void *)];
};

extern struct cpu            cputab[NCPU];
extern long                  schedidlecoremap[NCPU][SCHEDIDLECOREMAPNWORD];
extern struct task          *schedreadytab0[NCPU][SCHEDNQUEUE];
extern struct task          *schedreadytab1[NCPU][SCHEDNQUEUE];
extern struct schedqueueset  schedreadytab[NCPU];
extern struct divu32         fastu32div24tab[rounduppow2(SCHEDHISTORYSIZE,
                                                         PAGESIZE)];
extern long                  schednicetab[SCHEDNICERANGE];
extern long                  schedslicetab[SCHEDNICERANGE];
extern long                 *schedniceptr;
extern long                 *schedsliceptr;

/* based on sched_pctcpu_update from ULE */
static __inline__ void
schedadjcpupct(struct task *task, long run)
{
    long     tick = k_curcpu->ntick;
    unsigned last = task->lastrun;
    long     diff = tick - last;
    long     delta;
    long     ntick;
    long     val;
    long     div;

    if (diff >= SCHEDHISTORYNTICK) {
        task->ntick = 0;
        task->firstrun = tick - SCHEDHISTORYNTICK;
    } else {
        unsigned long first = task->firstrun;

        delta = tick - first;
        if (delta >= SCHEDHISTORYSIZE) {
            ntick = task->ntick;
            div = last - first;
            val = tick - SCHEDHISTORYNTICK;
            last -= val;
            ntick = fastu32div24(ntick, div, fastu32div24tab);
            ntick *= last;
            task->firstrun = val;
        }
    }
    if (run) {
        ntick = diff >> SCHEDTICKSHIFT;
        task->ntick = ntick;
    }
    task->lastrun = tick;

    return;
}

static __inline__ void
schedswapqueues(long cpu)
{
    struct schedqueueset *set = &schedreadytab[cpu];
    void                 *ptr1 = set->cur;
    void                 *ptr2 = set->next;

    set->next = ptr1;
    set->cur = ptr2;
    ptr1 = set->curmap;
    ptr2 = set->nextmap;
    set->nextmap = ptr1;
    set->curmap = ptr2;

    return;
}

static __inline__ struct cpu *
schedfindidlecore(long cpu, long *retcore)
{
    struct cpu *unit = &cputab[cpu];
    long       *map = &schedidlecoremap[cpu][0];
    long       *ptr = &map[0];
    long        nunit = NCPU;
    long        ncore = NCORE;
    long        lim = min(ncore, (long)(CHAR_BIT * sizeof(long)));
    long        ndx = 0;
    long        val = 0;
    long        cur;
    long        last = NCPU;
    long        ntz;
    long        mask;

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
    ptr = map;
    for (cur = 0 ; cur != cpu ; cur++) {
        mtxlk(&unit->lk);
        for (ndx = 0 ; ndx < lim ; ndx++) {
            mask = *ptr;
            if (mask) {
                ntz = tzerol(mask);
                clrbit(ptr, ntz);
                mtxunlk(&unit->lk);
                unit = &cputab[cur];
                ndx *= CHAR_BIT * sizeof(long);
                ndx += ntz;
                *retcore = ndx;

                return unit;
            }
            ptr++;
        }
        mtxunlk(&unit->lk);
    }
    cur++;
    ptr++;
    for ( ; cur < last ; cur++) {
        mtxlk(&unit->lk);
        for (ndx = 0 ; ndx < lim ; ndx++) {
            mask = *ptr;
            if (mask) {
                ntz = tzerol(mask);
                clrbit(ptr, ntz);
                mtxunlk(&unit->lk);
                unit = &cputab[cur];
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

#endif /* defined(ZEROSCHED) */

#endif /* __KERN_BITS_SCHED_H__ */

