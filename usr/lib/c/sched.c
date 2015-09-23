#include <limits.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sched.h>
#if defined(_ZERO_SOURCE)
#include <sys/zero/sched.h>
#else
#error define priorities in <sched.h>
#endif
#include <zero/trix.h>

/* { min, max } */
#if defined(USEGNU) && (USEGNU)
static int  __schedpriolimtab[__SCHED_NPOLICY][2]
= {
    { __SCHED_OTHER_PRIO_MIN, __SCHED_OTHER_PRIO_MAX }, // SCHED_OTHER
    { __SCHED_FIFO_PRIO_MIN, __SCHED_FIFO_PRIO_MAX },   // SCHED_FIFO
    { __SCHED_RR_PRIO_MIN, __SCHED_RR_PRIO_MAX },       // SCHED_RR
    { __SCHED_BATCH_PRIO_MIN, __SCHED_BATCH_PRIO_MAX }, // SCHED_BATCH
    { __SCHED_IDLE_PRIO_MIN, __SCHED_IDLE_PRIO_MAX }    // SCHED_IDLE
};
#else
static int  __schedpriolimtab[__SCHED_NPOLICY][2]
= {
    { __SCHED_OTHER_PRIO_MIN, __SCHED_OTHER_PRIO_MAX }, // SCHED_OTHER
    { __SCHED_FIFO_PRIO_MIN, __SCHED_FIFO_PRIO_MAX },   // SCHED_FIFO
    { __SCHED_RR_PRIO_MIN, __SCHED_RR_PRIO_MAX }        // SCHED_RR
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

int
__sched_count_cpus(cpu_set_t *set, size_t size)
{
    uint32_t *u32ptr = (uint32_t *)set->__bitmap;
    uint32_t  u32;
    size_t    n;
    int       tmp;
    int       ret = 0;

    n = size / (sizeof(uint32_t) * CHAR_BIT);
    while (n--) {
        u32 = u32ptr[n];
        onebits_32(u32, tmp);
        ret += tmp;
    }

    return ret;
}

int
__sched_cpusets_equal(size_t size, cpu_set_t *set1, cpu_set_t *set2)
{
    unsigned long  ndx;
    unsigned long  lim = size / (sizeof(long) * CHAR_BIT);
    unsigned long *set1ptr = (unsigned long *)(set1)->__bitmap;
    unsigned long *set2ptr = (unsigned long *)(set2)->__bitmap;
    
    for (ndx = 0 ; ndx < lim ; ndx++) {
        if (set1ptr[ndx] != set2ptr[ndx]) {

            return 0;
        }
    }

    return 1;
}

cpu_set_t *
__sched_alloc_cpuset(int ncpu)
{
    void *set = malloc(ncpu * sizeof(char) / CHAR_BIT);

    return set;
}

