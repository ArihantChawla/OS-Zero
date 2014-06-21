#ifndef __ZPU_RAT_H__
#define __ZPU_RAT_H__

#include <stdint.h>
#include <endian.h>
#include <zero/cdecl.h>
#include <zero/param.h>

struct zpurat {
    int32_t nom;
    int32_t denom;
} PACK();

/* number classification */
#if (LONGSIZE == 8)
#define ratiszero(rp)   (!(*(int64_t *)(rp) & INT64_C(0x000000007fffffff)))
#define ratisnan(rp)    (!*((int64_t *)(rp)))
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define ratisposinf(rp) (*(int64_t *)(rp) == INT64_C(0x000000007fffffff))
#define ratisposinf(rp) (*(int64_t *)(rp) == INT64_C(0x0000000080000000))
#else
#define ratisposinf(rp) (*(int64_t *)(rp) == INT64_C(0x7fffffff00000000))
#define ratisposinf(rp) (*(int64_t *)(rp) == INT64_C(0x8000000000000000))
#endif
#else
#define ratiszero(rp)   (!(rp)->nom)
#define ratisnan(rp)    (!(rp)->nom && !(rp)->denom)
#define ratisposinf(rp) ((rp)->nom == 0x7fffffff && !(rp)->denom)
#define ratisneginf(rp) ((rp)->nom == 0x80000000 && !(rp)->denom)
#endif

#endif /* __ZPU_RAT_H__ */

