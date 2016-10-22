#ifndef __KERN_SCHED_H__
#define __KERN_SCHED_H__

#include <kern/conf.h>
#include <zero/trix.h>
#include <zero/fastidiv.h>
#if defined(__KERNEL__)
#include <kern/proc/task.h>
#endif

#if defined(__KERNEL__)
extern void schedinit(void);
#endif
extern void schedyield(void);

#if defined(ZEROSCHED)

/* task scheduler classes */
#define SCHEDNCLASSPRIO     64          // # of priorities per class
#define SCHEDNCLASSQUEUE    (SCHEDNCLASSPRIO >> SCHEDNQUEUESHIFT)
#define SCHEDNQUEUESHIFT    1
#define SCHEDNQUEUEPRIO     (1 << SCHEDNQUEUESHIFT)
/* 'system' classes */
#define SCHEDDEADLINE       (-SCHEDREALTIME - 1) // deadline tasks
#define SCHEDREALTIMERR     (-SCHEDREALTIME) // 
#define SCHEDTRAP           0           // interrupt tasks
#define SCHEDREALTIME       1           // realtime threads
#define SCHEDSYSTEM         2
#define SCHEDNSYSCLASS      3           // # of system scheduler classes
/* timeshare classes */
#define SCHEDRESPONSIVE     3           // 'quick' timeshare tasks
#define SCHEDNORMAL         4           // 'normal' timeshare tasks
#define SCHEDBATCH          5           // batch tasks
#define SCHEDNUSERCLASS     3           // number of user ('timeshare') classes
#define SCHEDNCLASS         6           // # of user scheduler classes
#define SCHEDIDLE           SCHEDNCLASS // idle tasks
#define SCHEDNQUEUE         (SCHEDNCLASS * SCHEDNCLASSQUEUE)
#define SCHEDNTOTALQUEUE    (SCHEDNQUEUE + SCHEDNIDLE) // SCHEDIDLE too
#define SCHEDNTABQUEUE      512
#define SCHEDNOCLASS        (-0x7f)

#if 0 /* FIXME: these will be handled in SCHEDTRAP */
/* fixed priorities */
#define SCHEDHID            0           // human interface devices (kbd, mouse)
#define SCHEDAUDIO          1           // audio synchronisation
#define SCHEDVIDEO          2           // video synchronisation
#define SCHEDINIT           3           // init; creation of new processes
#define SCHEDFIXEDPRIOMIN   0
#endif

#define schedclassminprio(c)                                            \
    ((c) * SCHEDNCLASSPRIO)
#define schedclassmaxprio(c)                                            \
    (schedclassminprio(c) + SCHEDNCLASSPRIO - 1)
#define schedclassminqueue(c)                                           \
    ((c) * SCHEDNCLASSQUEUE)
/* priority organisation */
#define SCHEDNIDLE          SCHEDNCLASSQUEUE
#define SCHEDSYSPRIOMIN     schedclassminprio(SCHEDSYSTEM)
/* interrupt priority limits */
#define SCHEDTRAPPRIOMIN    schedclassminprio(SCHEDTRAP)
#define SCHEDTRAPPRIOMAX    schedclassmaxprio(SCHEDTRAP)
/* realtime priority limits */
#define SCHEDRTPRIOMIN      schedclassminprio(SCHEDREALTIME)
#define SCHEDRTPRIOMAX      schedclassmaxprio(SCHEDREALTIME)
/* timeshare priority limits */
#define SCHEDUSERPRIOMIN    schedclassminprio(SCHEDRESPONSIVE)
/* positive nice values will not be mapped to SCHEDIDLE */
#define SCHEDUSERPRIOMAX    (schedclassmaxprio(SCHEDBATCH) - SCHEDNICEHALF)
#define SCHEDUSERRANGE      (SCHEDUSERPRIOMAX - SCHEDUSERPRIOMIN + 1)
/* batch priority limits */
#define SCHEDBATCHPRIOMIN   schedclassminprio(SCHEDBATCH)
#define SCHEDBATCHPRIOMAX   schedclassmaxprio(SCHEDBATCH)
#define SCHEDBATCHRANGE     (SCHEDBATCHPRIOMAX - SCHEDBATCHPRIOMIN + 1)
/* idle priority limits */
#define SCHEDIDLEPRIOMIN    (SCHEDIDLE * SCHEDNCLASSPRIO)
#define SCHEDIDLEPRIOMAX    (SCHEDIDLEPRIOMIN + SCHEDNCLASSQUEUE - 1)
#define SCHEDIDLERANGE      (SCHEDIDLEPRIOMAX - SCHEDIDLEPRIOMIN + 1)
/* nice limits */
#define SCHEDNICEMIN        (-(SCHEDNCLASSQUEUE << 1))
#define SCHEDNICEMAX        ((SCHEDNCLASSQUEUE << 1) - 1)
#define SCHEDNICERANGE      (SCHEDNICEMAX - SCHEDNICEMIN + 1)
#define SCHEDNICEHALF       (SCHEDNICERANGE >> 1)
/* highest and lowest priorities are reserved for nice */
/* we allow negative nice values to map to classes SCHEDREALTIME..SCHEDSYSTEM */
#define SCHEDPRIOMIN        SCHEDUSERPRIOMIN
#define SCHEDPRIOMAX        SCHEDUSERPRIOMAX
#define SCHEDPRIORANGE      (SCHEDPRIOMAX - SCHEDPRIOMIN)
/* interactive priority limits */
#define SCHEDINTPRIOMIN     SCHEDUSERPRIOMIN
//#define SCHEDINTPRIOMAX     (SCHEDBATCHPRIOMIN + SCHEDBATCHPRIOMAX - 1)
#define SCHEDINTPRIOMAX     SCHEDUSERPRIOMAX
#define SCHEDINTRANGE       (SCHEDINTPRIOMAX - SCHEDINTPRIOMIN + 1)

/* interactivity scoring */
/* interactivity scores are in the range [0, 128] */
#define SCHEDSCOREMAX       128
/* minimum score to mark thread as interactive */
#define SCHEDSCOREINTLIM    32
/* half of maximum interactivity score */
#define SCHEDSCOREHALF      (SCHEDSCOREMAX >> 1)
/* number of seconds to keep cpu stats around */
#define SCHEDHISTORYNSEC    8
//#define SCHEDHISTORYSIZE    (SCHEDHISTORYMAX * (HZ << SCHEDTICKSHIFT))
/* number of ticks to keep cpu stats around */
#define SCHEDHISTORYNTICK   (SCHEDHISTORYNSEC * kgethz())
/* maximum number of ticks before scaling back */
#define SCHEDHISTORYSIZE    (SCHEDHISTORYNTICK + kgethz())
//#define SCHEDRECTIMEINCR       ((HZ << SCHEDTICKSHIFT) / HZ)
#define SCHEDTICKSHIFT      10
/* maximum number of sleep time + run time stored */
#define SCHEDRECTIMEMAX     ((kgethz() << 2) << SCHEDTICKSHIFT)
#define SCHEDRECTIMEFORKMAX ((kgethz() << 1) << SCHEDTICKSHIFT)

#if defined(__KERNEL__)
#include <kern/bits/sched.h>
#endif

#endif /* defined(ZEROSCHED) */

#endif /* __KERN_SCHED_H__ */

