#ifndef __KERN_SCHED_H__
#define __KERN_SCHED_H__

#include <kern/conf.h>

extern void schedyield(void);
extern void schedinit(void);

#if defined(ZEROSCHED)

#if !defined(HZ)
#define HZ                  250
#endif

#define schedistimeshare(prio)                                          \
    ((prio) == SCHEDRESPONSIVE || (prio) == SCHEDNORMAL)
#define schedpriotoclass(prio)                                          \
    ((prio) < 0 ? SCHEDREALTIME : ((prio) >> 6))
/* task scheduler classes */
#define SCHEDNCLASSPRIO     64          // # of priorities per class
#define SCHEDNCLASSQUEUE    16          // # of priority queues per class
#define SCHEDDEADLINE       (-2)        // deadline tasks
#define SCHEDREALTIME       (-1)
#define SCHEDSYSTEM         0           // system tasks such as shutdown/reboot
#define SCHEDINTERRUPT      1           // interrupt tasks
#define SCHEDREALTIME       2           // realtime tasks
#define SCHEDFIXED          3           // SYNC, INPUT, AUDIO, VIDEO
#define SCHEDNSYSCLASS      4           // # of system scheduler classes
#define SCHEDUSERBASEPRIO   (SCHEDNSYSCLASS * SCHEDNCLASSPRIO)
#define SCHEDRESPONSIVE     4           // interrupt and interactive tasks
#define SCHENORMAL          5           // time-share tasks
#define SCHEDBATCH          6           // batch tasks
#define SCHEDIDLE           7           // idle tasks
#define SCHEDNCLASS         8           // user scheduler classes
#define SCHEDNTOTALQUEUE    (SCHEDNCLASS * SCHEDNCLASSQUEUE)
#define SCHEDNTOTALPRIO     (SCHEDNCLASS * SCHEDNCLASSPRIO)
/* minimum (highest) interactive priority */
#define SCHEDINTERPRIOMIN                                               \
    (SCHEDUSERBASEPRIO + SCHEDRESPONSIVE * SCHEDNCLASSPRIO)
/* maximum (lowest) interactive priority */
#define SCHEDINTERPRIOMAX                                               \
    (SCHEDUSERBASEPRIO + SCHEDBATCH * SCHEDNCLASSPRIO - 1)
/* maximum timeshare priority (SCHEDNORMAL) */
#define SCHEDNORMALPRIOMAX  (SCHEDIDLE * SCHEDNCLASSPRIO - 1)
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

