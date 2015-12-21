#ifndef __ZPC_ZPC_H__
#define __ZPC_ZPC_H__

#include <zpc/conf.h>
#include <zero/cdefs.h>

#if defined(ZPC32BIT)
#include <zpc/zpc32.h>
#endif

struct zpcmach {
    zpcfreg_t  fregs[ZPCREGS];
    zpcreg_t   regs[ZPCREGS];
    zpcreg_t   vregs[ZPCREGS];
    void      *stk[ZPCREGS];
    zpcreg_t   pc;
    zpcreg_t   msw;
    zpcreg_t   fp;
    zpcreg_t   sp;
};

#endif /* __ZPC_ZPC_H__ */

