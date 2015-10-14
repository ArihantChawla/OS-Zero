#ifndef __KERN_SCHED_H__
#define __KERN_SCHED_H__

#include <kern/conf.h>

extern void schedyield(void);
extern void schedinit(void);

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
#define SCHEDDEADLINE     (-3L) // deadline tasks
#define SCHEDRT           (-2L) // realtime tasks
#define SCHEDFIXED        (-1L) // SYNC, INPUT, AUDIO, VIDEO
#define SCHEDRESPONSIVE   0     // system tasks
#define SCHEDNORMAL       1     // user tasks
#define SCHEDBATCH        2     // batch tasks
#define SCHEDIDLE         3     // idle tasks
#define SCHEDNPRIOCLASS   4     // # of scheduler classes
#define SCHEDNPRIO        64    // # of priorities per class
/* fixed priorities */
#define SCHEDHID          0
#define SCHEDAUDIO        1
#define SCHEDVIDEO        2
#define SCHEDINIT         3
#define SCHEDNFIXED       4

#endif /* __KERN_SCHED_H__ */

