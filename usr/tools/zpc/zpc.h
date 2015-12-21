#ifndef __ZPC_ZPC_H__
#define __ZPC_ZPC_H__

#include <zpc/conf.h>
#include <zero/cdefs.h>

#if defined(ZPC32BIT)
#include <zpc/zpc32.h>
#endif

#define ZPCMREGPC 0
#define ZPCMREGSW 1
#define ZPCMREGFP 2
#define ZPCMREGSP 3
#define ZPCMREGS  4
struct zpcmach {
    zpcfreg_t  fregs[ZPCREGS];
    zpcreg_t   regs[ZPCREGS];
    zpcreg_t   vregs[ZPCREGS];
    void      *stk[ZPCREGS];
    zpcreg_t   mregs[ZPCMREGS];
};

#endif /* __ZPC_ZPC_H__ */

