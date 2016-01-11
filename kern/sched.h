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

#if defined(ZEROSCHED)

#if !defined(HZ)
#define HZ                250
#endif

/* task scheduler classes */
#define taskistimeshare(tp)                                             \
    ((tp)->sched == SCHEDRESPONSIVE || (tp)->sched == SCHEDNORMAL)
#define SCHEDDEADLINE       (-2L)       // deadline tasks
#define SCHEDRT             (-1L)       // realtime tasks
#define SCHEDFIXED          0           // SYNC, INPUT, AUDIO, VIDEO
#define SCHEDRESPONSIVE     1           // system and interactive tasks
#define SCHEDNORMAL         2           // user tasks
#define SCHEDBATCH          3           // batch tasks
#define SCHEDIDLE           4           // idle tasks
#define SCHEDNPRIOCLASS     5           // # of scheduler classes
#define SCHEDNPRIO          64          // # of priorities per class
#define SCHEDNCLASSQUEUE    32          // # of scaled priorities per class
#define SCHEDINTPRIOMIN     (SCHEDRESPONSIVE * SCHEDNPRIO)
#define SCHEDINTPRIOMAX     (SCHEDNORMAL * SCHEDNPRIO - 1)
/* fixed priorities */
#define SCHEDHID            0
#define SCHEDAUDIO          1
#define SCHEDVIDEO          2
#define SCHEDINIT           3
#define SCHEDNFIXED         SCHEDNCLASSQUEUE
/* interactivity score */
#define SCHEDSCOREMAX       128
#define SCHEDSCORETHRESHOLD 32
#define SCHEDSCOREHALF      (SCHEDSCOREMAX >> 1)
#define SCHEDHISTORYMAX     4
#define SCHEDHISTORYSIZE    (SCHEDHISTORYMAX * (HZ << SCHEDTICKSHIFT))
#define SCHEDTIMEINCR       ((HZ << SCHEDTICKSHIFT) / HZ)
#define SCHEDTICKSHIFT      10

#endif /* defined(ZEROSCHED) */

#endif /* __KERN_SCHED_H__ */

