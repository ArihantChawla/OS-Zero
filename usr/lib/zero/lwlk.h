#ifndef __ZERO_LWLK_H__
#define __ZERO_LWLK_H__

/*
 * an implementation of "light-weight locks"
 *
 * REFERENCE: https://arxiv.org/pdf/1112.1141.pdf
 */

#include <stdint.h>
#include <zero/cdefs.h>
#include <mach/param.h>
#include <mach/asm.h>
#include <zero/cond.h>

#define LWLK_FAILURE 0L
#define LWLK_SUCCESS 1L

#define LWLK_DECL(name) THREADLOCAL struct lwlk *name

#define LWLK_WRLK       1
#define LWLK_WRLK_BIT   0x01L
#define LWLK_RDBIAS_BIT 0x02L
struct lwlk {
    volatile long flg;         // flag-bits
    volatile long nread;       // # of readers
    uintptr_t     waitq;       // waitq ID
    uint8_t       _pad[CLSIZE - 3 * sizeof(long)];
};

#endif /* __ZERO_LWLK_H__ */

