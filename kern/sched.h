#ifndef __KERN_SCHED_H__
#define __KERN_SCHED_H__

#include <kern/conf.h>
#include <zero/trix.h>
#include <zero/fastidiv.h>

extern void schedinit(void);
extern void schedyield(void);

#include <zero/mtx.h>

#define schedlkcpuntick(cpu)     (mtxlk(&cpu->lk), (cpu)->ntick)
#define schedlkcpu(cpu)          (mtxlk(&cpu->lk))
#define schedunlkcpu(cpu)        (mtxunlk(&cpu->lk))
#define schedlktaskruntime(task) (mtxlk(&task->lk), (task)->runtime)
#define schedlktask(task)        (mtxlk(&task->lk))
#define schedunlktask(task)      (mtxunlk(&task->lk))

#if defined(ZEROSCHED)

#if !defined(HZ)
#define HZ                  250
#endif

/* macros */
#define schedcalctime(task) ((task)->ntick >> SCHEDTICKSHIFT)
#define schedestticks(task) (max((task)->lastrun - (task)->firstrun, kgethz()))
#define schedcalcnice(task) (tasknicetab[(task)->nice])
#define schedcalcbaseprio(task, sched)                                  \
    ((sched) * SCHEDNCLASSPRIO)
#if 0
#define schedsetprio(task, pri)                                         \
    ((task)->prio = schedprioqueueid(pri))
#define schedsetprioincr(task, pri, incr)                               \
    ((task)->prio = schedprioqueueid(pri) + (incr))
#endif
#if 0
#define schedcalcprio(task)                                             \
    (fastuldiv32(schedcalctime(task),                                   \
                 (roundup(schedcalcticks(task), SCHEDPRIORANGE)         \
                  / SCHEDPRIORANGE),                                    \
                 divultab))
#endif
/* timeshare-tasks have interactivity scores */
#define schedistimeshare(sched)                                         \
    ((sched) >= SCHEDRESPONSIVE || (sched) <= SCHEDBATCH)
#define schedisinteract(score)        ((score) < SCHEDSCORETHRESHOLD)
#define schedcalcqueueid(pri)         ((pri) >> 1)
#define schedcalcqueueidofs(pri, ofs) (schedcalcqueueid(pri) + (ofs))

/* task scheduler classes */
#define SCHEDNCLASSPRIO     64          // # of priorities per class
#define SCHEDNCLASSQUEUE    32          // # of priority queues per class
#define SCHEDNQUEUEPRIO     (SCHEDNCLASSPRIO / SCHEDNCLASSQUEUE)
/* 'system' classes */
#define SCHEDDEADLINE       (-1)        // deadline tasks
#define SCHEDINTERRUPT      0           // interrupt tasks
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
#define SCHEDNTABQUEUE      512
#define SCHEDNOCLASS        0xff

#if 0 /* FIXME: these will be handled in SCHEDINTERRUPT */
/* fixed priorities */
#define SCHEDHID            0           // human interface devices (kbd, mouse)
#define SCHEDAUDIO          1           // audio synchronisation
#define SCHEDVIDEO          2           // video synchronisation
#define SCHEDINIT           3           // init; creation of new processes
#define SCHEDFIXEDPRIOMIN   0
#endif

/* priority organisation */
#define SCHEDNIDLE          SCHEDNCLASSQUEUE
#define SCHEDSYSPRIOMIN     (SCHEDSYSTEM * SCHEDNCLASSPRIO)
/* interrupt priority limits */
#define SCHEDINTRPRIOMIN    (SCHEDINTERRUPT * SCHEDNCLASSPRIO)
#define SCHEDINTRPRIOMAX    (SCHEDINTRPRIOMIN + SCHEDNCLASSPRIO - 1)
/* realtime priority limits */
#define SCHEDRTPRIOBASE     (SCHEDREALTIME * SCHEDNCLASSPRIO)
/* SCHED_FIFO tasks store priorities as negative values */
#define SCHEDRTPRIOMIN      (-SCHEDNCLASSPRIO + 1)
#define SCHEDRTPRIOMAX      (SCHEDNCLASSPRIO - 1)
/* timeshare priority limits */
#define SCHEDUSERPRIOMIN    (SCHEDRESPONSIVE * SCHEDNCLASSPRIO)
#define SCHEDNORMALPRIOMIN  (SCHEDNORMAL * SCHEDNCLASSPRIO)
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
#define SCHEDNICEMIN        (-(SCHEDNCLASSQUEUE << 1))
#define SCHEDNICEMAX        (SCHEDNCLASSQUEUE << 1)
#define SCHEDNICERANGE      (SCHEDNICEMAX - SCHEDNICEMIN + 1)
#define SCHEDNICEHALF       (SCHEDNICERANGE >> 1)
/* highest and lowest priorities are reserved for nice */
//#define SCHEDPRIOMIN        (SCHEDUSERPRIOMIN + SCHEDNICEHALF)
//#define SCHEDPRIOMAX        (SCHEDUSERPRIOMAX - SCHEDNICEHALF)
/* we allow negative nice values to map to classes SCHEDREALTIME..SCHEDSYSTEM */
#define SCHEDPRIOMIN        (SCHEDUSERPRIOMIN)
/* positive nice values will stay out of the idle queue */
#define SCHEDPRIOMAX        (SCHEDUSERPRIOMAX - SCHEDNICEHALF)
#define SCHEDPRIORANGE      (SCHEDPRIOMAX - SCHEDPRIOMIN)
/* interactive priority limits */
#define SCHEDINTPRIOMIN     SCHEDUSERPRIOMIN
#define SCHEDINTPRIOMAX     (SCHEDBATCHPRIOMIN + SCHEDBATCHPRIOMAX - 1)
#define SCHEDINTRANGE       (SCHEDINTPRIOMAX - SCHEDINTPRIOMIN + 1)

/* interactivity scoring */
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
#define SCHEDHISTORYNTICK   (SCHEDHISTORYNSEC * kgethz())
/* maximum number of ticks before scaling back */
#define SCHEDHISTORYSIZE    (SCHEDHISTORYNTICK + kgethz())
//#define SCHEDTIMEINCR       ((HZ << SCHEDTICKSHIFT) / HZ)
#define SCHEDTICKSHIFT      10
/* maximum number of sleep time + run time stored */
#define SCHEDTIMEMAX        ((kgethz() << 2) << SCHEDTICKSHIFT)
#define SCHEDTIMEFORKMAX    ((kgethz() << 1) << SCHEDTICKSHIFT)

/* interrupt priorities */
#define schedintrsoftprio(id)                                           \
    (SCHEDINTRSOFTPRIO + (id) * SCHEDNQUEUEPRIO)
#define SCHEDINTRRTPRIO     SCHEDINTRPRIOMIN
#define SCHEDINTRAVPRIO     (SCHEDINTRPRIOMIN + SCHEDNQUEUEPRIO)
#define SCHEDINTRHIDPRIO    (SCHEDINTRPRIOMIN + 2 * SCHEDNQUEUEPRIO)
#define SCHEDINTRNETPRIO    (SCHEDINTRPRIOMIN + 3 * SCHEDNQUEUEPRIO)
#define SCHEDINTRDISKPRIO   (SCHEDINTRPRIOMIN + 4 * SCHEDNQUEUEPRIO)
#define SCHEDINTRMISCPRIO   (SCHEDINTRPRIOMIN + 5 * SCHEDNQUEUEPRIO)
#define SCHEDINTRSOFTPRIO   (SCHEDINTRPRIOMIN + 6 * SCHEDNQUEUEPRIO)

#endif /* defined(ZEROSCHED) */

#endif /* __KERN_SCHED_H__ */

