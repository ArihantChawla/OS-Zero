#ifndef __KERN_SCHED_H__
#define __KERN_SCHED_H__

#include <kern/conf.h>
#include <zero/trix.h>

extern void schedyield(void);
extern void schedinit(void);

#if defined(ZEROSCHED)

#if !defined(HZ)
#define HZ                  250
#endif

/* timeshare-tasks have interactivity scores */
#define schedistimeshare(sched)                                         \
    ((sched) == SCHEDRESPONSIVE || (sched) == SCHEDNORMAL)
#define schedisinteract(score)                                          \
    ((score) < SCHEDSCORETHRESHOLD)
/* task scheduler classes */
#define SCHEDNCLASSPRIO     64          // # of priorities per class
#define SCHEDNCLASSQUEUE    32          // # of priority queues per class
/* 'system' classes */
#define SCHEDDEADLINE       (-1)        // deadline tasks
#define SCHEDFIXED          0           // SYNC, INPUT, AUDIO, VIDEO
#define SCHEDINTERRUPT      1           // interrupt tasks
#define SCHEDREALTIME       2           // realtime threads
#define SCHEDNSYSCLASS      3           // # of system scheduler classes
/* 'user' classes */
#define SCHEDRESPONSIVE     3           // 'quick' timeshare tasks
#define SCHEDNORMAL         4           // 'normal' timeshare tasks
#define SCHEDBATCH          5           // batch tasks
#define SCHEDNUSERCLASS     3
/* exclude SCHEDFIXED priorities from priority raises by propagation */
#define SCHEDNCLASS         6           // user scheduler classes
#define SCHEDIDLE           SCHEDNCLASS // idle tasks
#define SCHEDNQUEUE         (SCHEDNCLASS * SCHEDNCLASSQUEUE)
/* fixed priorities */
#define SCHEDHID            0           // human interface devices (kbd, mouse)
#define SCHEDAUDIO          1           // audio synchronisation
#define SCHEDVIDEO          2           // video synchronisation
#define SCHEDINIT           3           // init; creation of new processes
#define SCHEDFIXEDPRIOMIN   0
#define SCHEDNIDLE          SCHEDNCLASSQUEUE
/* exclude SCHEDFIXED-priorities from calculations */
#define SCHEDSYSPRIOMIN     (SCHEDINTERRUPT * SCHEDNCLASSPRIO)
/* timeshare priority limits */
#define SCHEDUSERPRIOMIN    (SCHEDRESPONSIVE * SCHEDNCLASSPRIO)
#define SCHEDUSERPRIOMAX    SCHEDBATCHPRIOMAX
#define SCHEDUSERRANGE      (SCHEDUSERPRIOMAX - SCHEDUSERPRIOMIN + 1)
/* batch priority limits */
#define SCHEDBATCHPRIOMIN   (SCHEDBATCH * SCHEDNCLASSPRIO)
#define SCHEDBATCHPRIOMAX   (SCHEDBATCHPRIOMIN + SCHEDNCLASSPRIO - 1)
#define SCHEDBATCHRANGE     (SCHEDBATCHPRIOMAX - SCHEDBATCHPRIOMIN + 1)
/* idle priority limits */
#define SCHEDIDLEPRIOMIN    (SCHEDIDLE * SCHEDNCLASSPRIO)
#define SCHEDIDLEPRIOMAX    (SCHEDIDLEPRIOMIN + SCHEDNCLASSPRIO - 1)
#define SCHEDIDLERANGE      (SCHEDIDLEPRIOMAX - SCHEDIDLEPRIOMIN + 1)
/* nice limits */
#define SCHEDNICEMIN        (-(SCHEDNCLASSPRIO >> 1))
#define SCHEDNICEMAX        ((SCHEDNCLASSPRIO >> 1) - 1)
#define SCHEDNICERANGE      (SCHEDNICEMAX - SCHEDNICEMIN + 1)
#define SCHEDNICEHALF       (SCHEDNICERANGE >> 1)
/* highest and lowest priorities are reserved for nice */
#define SCHEDPRIOMIN        (SCHEDUSERPRIOMIN + SCHEDNICEHALF)
#define SCHEDPRIOMAX        (SCHEDUSERPRIOMAX - SCHEDNICEHALF)
#define SCHEDPRIORANGE      (SCHEDPRIOMAX - SCHEDPRIOMIN)
/* interactive priority limits */
#define SCHEDINTPRIOMIN     SCHEDUSERPRIOMIN
#define SCHEDINTPRIOMAX     (SCHEDBATCHPRIOMIN + SCHEDBATCHPRIOMAX - 1)
#define SCHEDINTRANGE       (SCHEDINTPRIOMAX - SCHEDINTPRIOMIN + 1)
/* macros */
#define schedcalctime(task) ((task)->ntick >> SCHEDTICKSHIFT)
#define schedestticks(task) (max((task)->lastrun - (task)->firstrun, HZ))
#define schedcalcnice(task) (tasknicetab[(task)->nice])
#define schedcalcprio(task)                                             \
    (fastuldiv32(schedcalctime(task),                                   \
                 (roundup(schedcalcticks(task), SCHEDPRIORANGE)         \
                  / SCHEDPRIORANGE),                                    \
                 divultab))
/* interactivity scores are in the range [0, 128] */
#define SCHEDSCOREMAX       128
/* minimum score to mark thread as interactive */
#define SCHEDSCORETHRESHOLD 32
/* half of maximum interactivity score */
#define SCHEDSCOREHALF      (SCHEDSCOREMAX >> 1)
/* number of seconds to keep cpu stats around */
#define SCHEDHISTORYNSEC    8
//#define SCHEDHISTORYSIZE    (SCHEDHISTORYMAX * (HZ << SCHEDTICKSHIFT))
/* number of ticks to keep cpu stats around */
#define SCHEDHISTORYNTICK   (SCHEDHISTORYNSEC * HZ)
/* maximum number of ticks before scaling back */
#define SCHEDHISTORYSIZE    (SCHEDHISTORYNSEC + HZ)
//#define SCHEDTIMEINCR       ((HZ << SCHEDTICKSHIFT) / HZ)
#define SCHEDTICKSHIFT      10
/* maximum number of sleep time + run time stored */
#define SCHEDHISTORYMAX     ((HZ * 5) << SCHEDTICKSHIFT)
#define SCHEDHISTORYFORKMAX (HZ << (SCHEDTICKSHIFT - 1))

#endif /* defined(ZEROSCHED) */

#endif /* __KERN_SCHED_H__ */

