#ifndef __KERN_PROC_SCHED_H__
#define __KERN_PROC_SCHED_H__

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

#define HZ                 250

/* thread scheduler classes */
#define THR_INTR           0  // interrupt thread (keyboard, mouse, etc.)
#define THR_RT             1  // real-time thread
#define THR_USER           2  // user thread
#define THR_BATCH          3  // batch thread
#define THR_IDLE           4  // idle thread
#define THR_NCLASS         5  // number of scheduler policies
/* # of priorities per class */
#define THR_NCLASSPRIO     (1 << THR_NCLASSPRIOLOG2)
#define THR_NCLASSPRIOLOG2 5

#include <kern/proc/thr.h>

//extern volatile FASTCALL struct m_tcb * (*schedpickthr)(void);
extern void schedyield(void);
extern void schedinit(void);

#if (ZEROSCHED)
struct thrprioq {
    volatile long  lk;
    struct thr    *head;
    struct thr    *tail;
};
#endif

#endif /* __KERN_PROC_SCHED_H__ */

