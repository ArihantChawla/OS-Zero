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

/* flg-member bits */
struct malloc {
    long           flg;
};

#endif /* ___MALLOC_H__ */

