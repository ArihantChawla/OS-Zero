#ifndef __ZPC_ZPC_H__
#define __ZPC_ZPC_H__

#include <zpc/conf.h>
#include <zero/cdefs.h>

#if defined(ZPC32BIT)
#include <zpc/zpc32.h>
#endif

/* mregs-indices */
#define ZPCMREGPC 0
#define ZPCMREGSW 1
#define ZPCMREGFP 2
#define ZPCMREGSP 3
#define ZPCMREGS  4
/* MSW-bits */
#define ZPCMSWZF  (1 << 0)      // zero-flag
#define ZPCMSWOF  (1 << 1)      // overflow-flag
#define ZPCMSWCF  (1 << 2)      // carry-flag
#define ZPCMSWIF  (1 << 3)      // interrupts enabled
#define ZPCMSWUF  (1 << 4)      // user-mode execution
struct zpcmach {
    zpcfreg_t  fregs[ZPCREGS];
    zpcreg_t   regs[ZPCREGS];
    zpcreg_t   vregs[ZPCREGS];
    void      *stk[ZPCREGS];
    zpcreg_t   mregs[ZPCMREGS];
};

#endif /* __ZPC_ZPC_H__ */

