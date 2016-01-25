#ifndef __KERN_BITS_SCHED_H__
#define __KERN_BITS_SCHED_H__

#include <zero/mtx.h>

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

/* data structures */

extern struct divu32 fastu32div24tab[rounduppow2(SCHEDHISTORYSIZE, PAGESIZE)];

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

#if (PTRSIZE == 8)
#define SCHEDNLVLWAITLOG2 16
#elif (PTRSIZE == 4)
#define SCHEDNLVLWAITLOG2 8
#endif
#define SCHEDNLVL0WAIT    (1 << SCHEDNLVLWAITLOG2)
#define SCHEDNLVL1WAIT    (1 << SCHEDNLVLWAITLOG2)
#define SCHEDNLVL2WAIT    (1 << SCHEDNLVLWAITLOG2)
#define SCHEDNLVL3WAIT    (1 << SCHEDNLVLWAITLOG2)
#define SCHEDNWAITKEY     4

#define taskwaitkey0(wc)                                                \
    (((wc) >> (3 * SCHEDNLVLWAITLOG2)) & ((1UL << SCHEDNLVLWAITLOG2) - 1))
#define taskwaitkey1(wc)                                                \
    (((wc) >> (2 * SCHEDNLVLWAITLOG2)) & ((1UL << SCHEDNLVLWAITLOG2) - 1))
#define taskwaitkey2(wc)                                                \
    (((wc) >> (1 * SCHEDNLVLWAITLOG2)) & ((1UL << SCHEDNLVLWAITLOG2) - 1))
#define taskwaitkey3(wc)                                                \
    ((wc) & ((1UL << SCHEDNLVLWAITLOG2) - 1))

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

#endif /* defined(ZEROSCHED) */

#endif /* __KERN_BITS_SCHED_H__ */

