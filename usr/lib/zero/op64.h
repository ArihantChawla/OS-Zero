#ifndef __ZERO_OP64_H__
#define __ZERO_OP64_H__

#include <stdint.h>
#include <endian.h>

#if (defined(__i386__) || defined(__i486__)                             \
     || defined(__i586__) || defined(__i686__))
#define USEINT64 1
#define opdwload(ptr)       (*(int64_t *)(ptr))
#define opdwstore(val, ptr) ((val) = *(int64_t *)(ptr))
#endif

#if defined(USEINT64) && (USEINT64)
typedef int64_t            op64_t;
#define OP64_ZERO          INT64_C(0)
#else
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
typedef struct {
    uint64_t lo;
    int64_t  hi;
} op64_t;
#elif (__BYTE_ORDER == __BIG_ENDIAN)
typedef struct {
    int64_t  hi;
    uint64_t lo;
} op64_t;
#endif
#define OP64_ZERO          { 0, 0 }
#endif

#define op64zero(dest)     (*(op64_t *)dest = OP64_ZERO)
#define op64cpy(src, dest) (*(op64_t *)dest = *(op64_t *)src)

#endif /* __ZERO_OP64_H__ */

