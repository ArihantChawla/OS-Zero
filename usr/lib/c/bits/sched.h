#ifndef __ZERO_BITS_SCHED_H__
#define __ZERO_BITS_SCHED_H__

#include <features.h>
#include <limits.h>
#include <zero/trix.h>

/* __policy-field */
#define __SCHED_MIN_POLICY     SCHED_OTHER
#define SCHED_OTHER            0
#define SCHED_FIFO             1
#define SCHED_RR               2
#if defined(USEGNU) && (USEGNU)
#define SCHED_BATCH            3
#define SCHED_IDLE             4
#define __SCHED_NPOLICY        5
#define SCHED_RESET_ON_FORK    (1 << 31)
#else
#define __SCHED_NPOLICY        3
#endif
#define __SCHED_MAX_POLICY     (__SCHED_NPOLICY - 1)
/* __flg-field */
#define __SCHED_RT             (1 << 0) // realtime scheduling
#define __SCHED_FIXED_PRIORITY (1 << 1) // SYNC, INPUT, AUDIO, VIDEO
#define __SCHED_PROC_SCOPE     (1 << 2) // otherwise, system scope
struct sched_param {
    int  sched_priority;
    long __policy;
    long __nice;
    long __flg;
};

#define __CPU_SETSIZE 1024
typedef struct {
    unsigned char __bitmap[__CPU_SETSIZE / CHAR_BIT];
} cpuset_t;

#endif /* __ZERO_BITS_SCHED_H__ */

