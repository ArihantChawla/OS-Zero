#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sched.h>
#if defined(_ZERO_SOURCE)
#include <sys/zero/sched.h>
#else
#error define priorities in <sched.h>
#endif

/* { min, max } */
#if defined(USEGNU) && (USEGNU)
static int  __schedpriolimtab[__SCHED_NPOLICY][2]
= {
    { __SCHED_OTHER_PRIO_MIN, __SCHED_OTHER_PRIO_MAX }, // SCHED_OTHER
    { __SCHED_FIFO_PRIO_MIN, __SCHED_FIFO_PRIO_MAX },   // SCHED_FIFO
    { __SCHED_RR_PRIO_MIN, __SCHED_RR_PRIO_MAX },       // SCHED_RR
    { __SCHED_BATCH_PRIO_MIN, __SCHED_BATCH_PRIO_MAX }, // SCHED_BATCH
    { __SCHED_IDLE_PRIO_MIN, __SCHED_IDLE_PRIO_MAX }   // SCHED_IDLE
};
#else
static int  __schedpriolimtab[__SCHED_NPOLICY][2]
= {
    { __SCHED_OTHER_PRIO_MIN, __SCHED_OTHER_PRIO_MAX }, // SCHED_OTHER
    { __SCHED_FIFO_PRIO_MIN, __SCHED_FIFO_PRIO_MAX },   // SCHED_FIFO
    { __SCHED_RR_PRIO_MIN, __SCHED_RR_PRIO_MAX }       // SCHED_RR
}
#endif

int
sched_get_priority_min(int policy)
{
    int retval;

#if defined(_ZERO_SOURCE)
    if (policy < __SCHED_MIN_POLICY || policy > __SCHED_MAX_POLICY) {
        errno = EINVAL;

        return -1;
    }
    retval = __schedpriolimtab[policy][0];
#endif

    return retval;
}

int
sched_get_priority_max(int policy)
{
    int retval;
    
#if defined(_ZERO_SOURCE)
    if (policy < __SCHED_MIN_POLICY || policy > __SCHED_MAX_POLICY) {
        errno = EINVAL;

        return -1;
    }
    retval = __schedpriolimtab[policy][1];
#endif

    return retval;
}

#if 0
int
sched_getscheduler(pid_t pid)
{
    int retval = -1;
    
    if (!pid) {
        pid = getpid();
    }

    return retval;
}
#endif

