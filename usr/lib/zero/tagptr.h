#ifndef __ZERO_TAGPTR_H__
#define __ZERO_TAGPTR_H__

/* tagged pointers for libzero */

#include <stdint.h>

#define tagptrcmpswap(tp, want, src)                                    \
    m_cmpswapdbl((volatile long *)tp,                                   \
                 (volatile long *)want,                                 \
                 (volatile long *)src)

#if defined(__x86_64__) || defined(__amd64__)

#include <zero/x86-64/tagptr.h>

#endif

#endif /* __ZERO_TAGPTR_H__ */

