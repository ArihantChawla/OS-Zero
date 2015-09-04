#ifndef __ZERO_BAR_H__
#define __ZERO_BAR_H__

#include <stdint.h>
#include <zero/param.h>
#include <zero/mtx.h>
#include <zero/cond.h>

#if (LONGSIZE == 8)
#define BARFLAGBIT   (UINT64_C(1) << 63)
#elif (LONGSIZE == 4)
#define BARFLAGBIT   (UINT32_C(1) << 31)
#endif
#define BARSERIALTHR (-1L)

typedef struct {
#if defined(ZEROMTX)
    volatile long lk;
#elif defined(PTHREAD)
    zeromtx       lk;
#endif
    unsigned long num;
    unsigned long cnt;
    zerocond      cond;
} zerobar;

typedef struct {
    volatile long                   lk;
    volatile long                   nref;
    unsigned long                   num;
    union {
        struct {
            volatile unsigned int   seq;
            volatile unsigned int   cnt;
        };
        volatile unsigned long long rst;
    };
} zerobarpool;

#endif /* __ZERO_BAR_H__ */

