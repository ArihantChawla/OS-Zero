#ifndef __ZVM_MACH_H__
#define __ZVM_MACH_H__

#include <zero/trix.h>

#define zvmalign(adr, x)                                                \
    (!((x) & (sizeof(x) - 1))                                           \
     ? (adr)                                                            \
     : rounduppow2(adr,sizeof(x)))
#define zvmalignop(adr)                                                 \
    zvmalign(adr, sizeof(struct zvmopcode))

#endif /* __ZVM_MACH_H__ */

