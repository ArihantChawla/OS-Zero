#ifndef __ZERO_TAGPTR_H__
#define __ZERO_TAGPTR_H__

/* tagged pointers for libzero */

#include <stdint.h>

#if ((defined(__i386__) || defined(__i486__)                            \
      || defined(__i586__) || defined(__i686__))                        \
     && (!defined(__x86_64__) && !defined(__amd64__) && !defined(SSE)))

#include <zero/ia32/tagptr.h>

#elif defined(__x86_64__) || defined(__amd64__)

#include <zero/x86-64/tagptr.h>

#endif

#endif /* __ZERO_TAGPTR_H__ */

