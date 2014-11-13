#ifndef __ZERO_FUTEX_H__
#define __ZERO_FUTEX_H__

#if defined(__linux__)
#include <linux/futex.h>
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#if 0
#include <unistd.h>
#endif
#include <sys/syscall.h>
#endif
#include <sys/time.h>
#include <zero/mtx.h>

#define MUTEX_INITIALIZER { 0 }
#define MUTEXUNLOCKED     ZEROMTXINITVAL
#define MUTEXLOCKED       ZEROMTXLKVAL
#define MUTEXCONTD        ZEROMTXCONTVAL

typedef long mutex_t;

#if defined(__linux__)
long syscall(long num, ...);
#endif

#endif /* __ZERO_FUTEX_H__ */

