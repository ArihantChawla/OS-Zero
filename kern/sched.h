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

#define HZ           250

#define THRKERN      0
#define THRRT        1
#define THRINTERACT  2
#define THRBATCH     3
#define THRIDLE      4
#define THRNCLASS    5
#define THRNPRIO     64
#define THRNPRIOLOG2 6

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

