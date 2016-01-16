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
#define SCHEDFIXED          0           // SYNC, INPUT, AUDIO, VIDEO
#define SCHEDINTERRUPT      1           // interrupt tasks
#define SCHEDREALTIME       2           // realtime threads
#define SCHEDNSYSCLASS      3           // # of system scheduler classes
/* 'user' classes */
#define SCHEDUSERPRIOMIN    (SCHEDRESPONSIVE * SCHEDNCLASSPRIO)
#define SCHEDRESPONSIVE     3           // 'quick' timeshare tasks
#define SCHEDNORMAL         4           // 'normal' timeshare tasks
#define SCHEDBATCH          5           // batch tasks
#define SCHEDNUSERCLASS     3
/* exclude SCHEDFIXED priorities from priority raises by propagation */
#define SCHEDSYSPRIOMIN     (SCHEDINTERRUPT * SCHEDNCLASSQUEUE)
#define SCHEDUSERPRIOMAX    (SCHEDIDLE * SCHEDNCLASSPRIO - 1)
#define SCHEDNCLASS         6           // user scheduler classes
#define SCHEDIDLE           SCHEDNCLASS // idle tasks
#define SCHEDNQUEUE         (SCHEDNCLASS * SCHEDNCLASSQUEUE)
/* fixed priorities */
#define SCHEDHID            0           // human interface devices (kbd, mouse)
#define SCHEDAUDIO          1           // audio synchronisation
#define SCHEDVIDEO          2           // video synchronisation
#define SCHEDINIT           3           // init; creation of new processes
#define SCHEDFIXEDPRIOMIN   0
#define SCHEDNFIXED         SCHEDNCLASSQUEUE
#define SCHEDNIDLE          SCHEDNCLASSQUEUE
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

