#ifndef __ZERO_FUTEX_H__
#define __ZERO_FUTEX_H__

#include <features.h>
#if defined(__linux__)
#include <linux/futex.h>
#endif /* defined(__linux__) */
#include <sys/time.h>
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif /* !defined(_GNU_SOURCE) */
#if (_ZERO_SOURCE) || defined(__linux__)
#include <sys/syscall.h>
#endif /* ZERO || linux */
#if (_ZERO_SOURCE)
#include <zero/mtx.h>
#if !defined(__linux__)
#include <zero/bits/futex.h>
#endif /* !defined(__linux__) */
#endif

#define MUTEX_INITIALIZER { ZEROMTXINITVAL }
#define MUTEXUNLOCKED     ZEROMTXINITVAL
#define MUTEXLOCKED       ZEROMTXLKVAL
#define MUTEXCONTD        ZEROMTXCONTVAL

typedef struct {
    zeromtx lk;
} mutex_t;

#if defined(_ZERO_SOURCE) || defined(__linux__)
extern long syscall(long num, ...);
#endif

int futex(int *uadr, int op, int val,
          const struct timespec *timeout,
          int *uadr2, int val2);

#endif /* __ZERO_FUTEX_H__ */

