#ifndef __KERN_PROC_SCHED_H__
#define __KERN_PROC_SCHED_H__

#include <kern/conf.h>
#include <zero/types.h>
/*
 * task scheduler
 * ----------------
 * - 256 priorities
 * - deadline
 * - synchronous
 * - interactive
 * - batch
 * - idle
 */

#define HZ                 250

/* task scheduler classes */
#define SCHEDRT           (-1L)
#define SCHEDFIXED        (-2L)
#define SCHEDSYS          0
#define SCHEDUSER         1
#define SCHEDBATCH        2
#define SCHEDIDLE         3
#define SCHEDNCLASS       4
#define SCHEDNPRIO        32
/* # of priorities per class */
#if 0
#define TASK_NCLASSPRIO     (1 << TASK_NCLASSPRIOLOG2)
#define TASK_NCLASSPRIOLOG2 5
#endif

#include <kern/proc/task.h>

//extern volatile FASTCALL struct m_tcb * (*schedpicktask)(void);
extern void schedyield(void);
extern void schedinit(void);

#if (ZEROSCHED)
struct taskprioq {
    volatile long  lk;
    struct task   *head;
    struct task   *tail;
};
#endif

#endif /* __KERN_PROC_SCHED_H__ */

