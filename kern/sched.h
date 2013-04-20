#ifndef __KERN_SCHED_H__
#define __KERN_SCHED_H__

#include <kern/conf.h>
/*
 * thread scheduler
 * ----------------
 * - 256 priorities
 * - deadline
 * - synchronous
 * - interactive
 * - batch
 * - idle
 */

#define HZ         250
#define NPRIO      256

#define THRRT      0
#define THRINTACT  1
#define THRBATCH   2
#define THRIDLE    3
#define THRNPRIO   32

#include <kern/thr.h>

extern void (*schedyield)(void);

#if (ZEROSCHED)
struct thrprioq {
    long        lk;
    struct thr *head;
    struct thr *tail;
};
#endif

#endif /* __KERN_SCHED_H__ */

