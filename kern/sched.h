#ifndef __KERN_SCHED_H__
#define __KERN_SCHED_H__

#include <kern/conf.h>

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

#if !defined(HZ)
#define HZ                250
#endif

/* task scheduler classes */
//#define SCHEDRT           (-1L)
#define SCHEDDEADLINE     (-1L)
#define SCHEDFIXED        (-2L) // SYNC, INPUT, AUDIO, VIDEO
#define SCHEDRT           0     // realtime tasks
#if 0
#define SCHEDRR           0     // round-robin, realtime
#define SCHEDFIFO         0     // FIFO, realtime
#endif
#define SCHEDSYS          1     // system tasks
#define SCHEDUSER         2     // user tasks
#define SCHEDBATCH        3     // batch tasks
#define SCHEDIDLE         4     // idle tasks
#define SCHEDNCLASS       5     // # of scheduler classes
#define SCHEDNPRIO        32    // # of priorities per class
/* # of priorities per class */
#if 0
#define TASK_NCLASSPRIO     (1 << TASK_NCLASSPRIOLOG2)
#define TASK_NCLASSPRIOLOG2 5
#endif

#endif /* __KERN_SCHED_H__ */

