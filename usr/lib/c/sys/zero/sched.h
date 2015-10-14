#ifndef __SYS_ZERO_SCHED_H__
#define __SYS_ZERO_SCHED_H__

#include <features.h>
#include <kern/sched.h>

/* scheduler classes and their minimum and maximum priorities */
#define __SCHED_OTHER_PRIO_MIN (SCHEDNORMAL * SCHEDNPRIO)
#define __SCHED_OTHER_PRIO_MAX ((SCHEDNORMAL + 1) * SCHEDNPRIO - 1)
#define __SCHED_FIFO_PRIO_MIN  0
#define __SCHED_FIFO_PRIO_MAX  SCHEDNPRIO
#define __SCHED_RR_PRIO_MIN    0
#define __SCHED_RR_PRIO_MAX    SCHEDNPRIO
#if defined(USEGNU) && (USEGNU)
#define __SCHED_BATCH_PRIO_MIN (SCHEDBATCH * SCHEDNPRIO)
#define __SCHED_BATCH_PRIO_MAX ((SCHEDBATCH + 1) * SCHEDNPRIO - 1)
#define __SCHED_IDLE_PRIO_MIN  (SCHEDIDLE * SCHEDNPRIO)
#define __SCHED_IDLE_PRIO_MAX  ((SCHEDIDLE + 1) * SCHEDNPRIO - 1)
#endif

#endif /* __SYS_ZERO_SCHED_H__ */

