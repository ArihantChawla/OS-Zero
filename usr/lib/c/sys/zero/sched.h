#ifndef __SYS_ZERO_SCHED_H__
#define __SYS_ZERO_SCHED_H__

#include <features.h>
#include <kern/sched.h>

/* scheduler classes and their minimum and maximum priorities */
#define __SCHED_OTHER_PRIO_MIN 0
#define __SCHED_OTHER_PRIO_MAX SCHEDUSERPRIOMAX
#define __SCHED_FIFO_PRIO_MIN  0
#define __SCHED_FIFO_PRIO_MAX  SCHEDNCLASSPRIO
#define __SCHED_RR_PRIO_MIN    0
#define __SCHED_RR_PRIO_MAX    SCHEDNCLASSPRIO
#if defined(USEGNU) && (USEGNU)
#define __SCHED_BATCH_PRIO_MIN (SCHEDBATCH * SCHEDNCLASSPRIO)
#define __SCHED_BATCH_PRIO_MAX (__SCHED_BATCH_PRIO_MIN + SCHEDNCLASSPRIO - 1)
#define __SCHED_IDLE_PRIO_MIN  (SCHEDIDLE * SCHEDNCLASSPRIO)
#define __SCHED_IDLE_PRIO_MAX  (__SCHED_IDLE_PRIO_MIN + SCHEDNCLASSPRIO - 1)
#endif

#endif /* __SYS_ZERO_SCHED_H__ */

