#ifndef __ZERO_FUTEX_H__
#define __ZERO_FUTEX_H__

#if defined(__linux__)
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#include <unistd.h>
#include <sys/syscall.h>
#endif
#include <sys/time.h>
#include <zero/mtx.h>

#define MUTEX_INITIALIZER { 0 }
#define MUTEXUNLOCKED     ZEROMTXINITVAL
#define MUTEXLOCKED       ZEROMTXLKVAL
#define MUTEXCONTD        ZEROMTXCONTVAL

typedef volatile long mutex_t;

#if defined(__linux__)
long syscall(long num, ...);
#endif

#if defined(__linux__)
static __inline__ long
sys_futex(void *adr1, long op, long val1, struct timespec *timeout,
          void *adr2, int val2)
{
    long retval = syscall(SYS_futex, adr1, op, val1, timeout, adr2, val2);

    return retval;
}
#endif /* defined(__linux__) */

#endif /* __ZERO_FUTEX_H__ */

