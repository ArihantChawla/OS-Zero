#ifndef ___MALLOC_H__
#define ___MALLOC_H__

#include <limits.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/mem.h>

/* bits for the flg-argument of _malloc() */
#define MALLOCINITBIT (1L << (sizeof(long) * CHAR_BIT - 1))
#define MALLOCZEROBIT (1L << 0)

#if 0
/* flg-member bits */
struct malloc {
    long           flg;
};
#endif

struct sysalloc {
    void * (*realloc)(void *ptr, size_t size);
    void   (*free)(void *ptr);
};

#endif /* ___MALLOC_H__ */

