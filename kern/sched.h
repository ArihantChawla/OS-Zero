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
#define SCHEDRT           (-1L)
#define SCHEDFIXED        (-2L) // SYNC, INPUT, AUDIO, VIDEO
#define SCHEDDEADLINE     (-3L)
#define SCHEDSYS          0     // system tasks
#define SCHEDUSER         1     // user tasks
#define SCHEDBATCH        2     // batch tasks
#define SCHEDIDLE         3     // idle tasks
#define SCHEDNCLASS       4     // # of scheduler classes
#define SCHEDNPRIO        32    // # of priorities per class
/* # of priorities per class */

#endif /* __KERN_SCHED_H__ */

