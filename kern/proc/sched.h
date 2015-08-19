#ifndef __KERN_SCHED_H__
#define __KERN_SCHED_H__

#include <kern/conf.h>
#include <zero/types.h>
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
#define THRINTR      0  // interrupt thread (keyboard, mouse, etc.)
#define THRRT        1  // real-time thread
#define THRUSER      2  // user thread
#define THRBATCH     3  // batch thread
#define THRIDLE      4  // idle thread
#define THRNSCHED    5  // number of scheduler policies
#define THRNPRIO     32 // # of priorities per class
#define THRNPRIOLOG2 5

#include <kern/proc/thr.h>

//extern volatile FASTCALL struct m_tcb * (*schedpickthr)(void);
extern void schedyield(void);
extern void schedinit(void);

#if (ZEROSCHED)
struct thrprioq {
    long        lk;
    struct thr *head;
    struct thr *tail;
};
#endif

#endif /* __KERN_SCHED_H__ */

