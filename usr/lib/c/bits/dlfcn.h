#ifndef __BIT_DLFCN_H__
#define __BIT_DLFCN_H__

#include <features.h>

#define RTLD_LAZY         (1 << 0)
#define RTLD_NOW          (1 << 1)
#define RTLD_BINDING_MASK (RTLD_LAZY | RTLD_NOW)
#define RTLD_NOLOAD       (1 << 2)
#define RTLD_DEEPBIND     (1 << 3)

#define RTLD_GLOBAL       (1 << 8)
#define RTLD_LOCAL        0
#define RTLD_NODELETE     (1 << 12)

#if defined(_GNU_SOURCE)

#define DL_CALL_FCT(fctp, args)                                         \
    (_dl_mcount_wrapper_check((void *)(fctp)), (*(fctp))args)
extern void _dl_mcount_wrapper_check(void *selfpc);

#endif

#endif /* __BIT_DLFCN_H__ */

