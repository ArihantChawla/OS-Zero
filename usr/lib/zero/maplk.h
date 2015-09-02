#ifndef __ZERO_MAPLK_H__
#define __ZERO_MAPLK_H__

#include <stdlib.h>
#include <zero/mtx.h>

#define ZEROMAPLKINIT { ZEROMTXINITVAL, 0, NULL }

typedef struct {
    volatile long  mtx;
    long           nbit;
    long          *bits;
} zeromaplk;

long maplkinit(long n, zeromaplk *maplk);

#endif /* __ZERO_MAPLK_H__ */

