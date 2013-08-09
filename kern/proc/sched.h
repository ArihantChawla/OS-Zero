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

/* thread scheduler classes */
#define THRRT        0
#define THRUSER      1
#define THRBATCH     2
#define THRIDLE      3
#define THRNCLASS    4
#define THRNPRIO     64
#define THRNPRIOLOG2 6

#include <kern/proc/thr.h>

extern void (*schedyield)(void);
void          schedinit(void);

#if (ZEROSCHED)
struct thrprioq {
    long        lk;
    struct thr *head;
    struct thr *tail;
};
#endif

#endif /* __KERN_SCHED_H__ */

