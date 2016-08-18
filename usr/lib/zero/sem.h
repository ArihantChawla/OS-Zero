#ifndef __ZERO_SEM_H__
#define __ZERO_SEM_H__

#include <limits.h>
#include <zero/mtx.h>

#define ZEROSEM_MAXVAL ULONG_MAX
typedef struct sem {
#if (ZEROFMTX)
    zerofmtx      lk;
#endif
    unsigned long val;
} zerosem;

#endif /* __ZERO_SEM_H__ */

