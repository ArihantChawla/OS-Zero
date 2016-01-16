#ifndef __KERN_SCHED_H__
#define __KERN_SCHED_H__

#include <kern/conf.h>

extern void schedyield(void);
extern void schedinit(void);

#if defined(ZEROSCHED)

#if !defined(HZ)
#define HZ                  250
#endif

/* timeshare-tasks have interactivity scores */
#define schedistimeshare(sched)                                         \
    ((sched) == SCHEDRESPONSIVE || (sched) == SCHEDNORMAL)
/* task scheduler classes */
#define SCHEDNCLASSPRIO     64          // # of priorities per class
#define SCHEDNCLASSQUEUE    16          // # of priority queues per class
/* 'system' classes */
#define SCHEDDEADLINE       (-1)        // deadline tasks
#define SCHEDINTERRUPT      0           // interrupt tasks
#define SCHEDREALTIME       1           // realtime threads
#define SCHEDFIXED          2           // SYNC, INPUT, AUDIO, VIDEO
#define SCHEDNSYSCLASS      3           // # of system scheduler classes
/* 'user' classes */
#define SCHEDUSERPRIOMIN    (SCHEDRESPONSIVE * SCHEDNCLASSPRIO)
#define SCHEDRESPONSIVE     3           // interrupt and interactive tasks
#define SCHEDNORMAL         4           // time-share tasks
#define SCHEDBATCH          5           // batch tasks
#define SCHEDNUSERCLASS     3
#define SCHEDUSERPRIOMAX    (SCHEDIDLE * SCHEDNCLASSPRIO - 1)
#define SCHEDIDLE           6           // idle tasks
#define SCHEDNCLASS         7           // user scheduler classes
#define SCHEDNQUEUE         (SCHEDNCLASS * SCHEDNCLASSPRIO)
/* fixed priorities */
#define SCHEDHID            0           // human interface devices (kbd, mouse)
#define SCHEDAUDIO          1           // audio synchronisation
#define SCHEDVIDEO          2           // video synchronisation
#define SCHEDINIT           3           // init; creation of new processes
#define SCHEDFIXEDPRIOMIN   (SCHEDFIXED * SCHEDNCLASSPRIO)
#define SCHEDNFIXED         SCHEDNCLASSQUEUE
/* minimum (highest) interactive priority */
#define SCHEDINTPRIOMIN     SCHEDUSERPRIOMIN
/* maximum (lowest) interactive priority */
#define SCHEDINTPRIOMAX     (SCHEDBATCH * SCHEDNCLASSPRIO - 1)
/* interactivity scores are in the range [0, 128] */
#define SCHEDSCOREMAX       128
/* minimum score to mark thread as interactive */
#define SCHEDSCORETHRESHOLD 32
#define SCHEDSCOREHALF      (SCHEDSCOREMAX >> 1)
#define SCHEDHISTORYMAX     4
#define SCHEDHISTORYSIZE    (SCHEDHISTORYMAX * (HZ << SCHEDTICKSHIFT))
#define SCHEDTIMEINCR       ((HZ << SCHEDTICKSHIFT) / HZ)
#define SCHEDTICKSHIFT      10

#endif /* defined(ZEROSCHED) */

#endif /* __KERN_SCHED_H__ */

