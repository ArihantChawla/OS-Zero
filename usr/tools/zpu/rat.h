#ifndef __ZPU_RAT_H__
#define __ZPU_RAT_H__

#include <zpu/conf.h>

#if (ZPURAT)

#include <stdint.h>
#include <endian.h>
#include <zero/cdecl.h>
#include <zero/param.h>

#define _RAT_PI_NOM   1570796327
#define _RAT_PI_DENOM  500000000
#define _RAT_E_NOM    1359140914
#define _RAT_E_DENOM   500000000

#define RAT_PI { _RAT_PI_NOM , _RAT_PI_DENOM }
#define RAT_E  { _RAT_E_NOM, _RAT_E_DENOM }
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define RAT_PI64 (INT64_C(_RAT_PI_NOM) | (INT64_C(_RAT_PI_DENOM) << 32))
#define RAT_E64  (INT64_C(_RAT_E_NOM) | (INT64_C(_RAT_E_DENOM) << 32))
#else
#define RAT_PI64 ((INT64_C(_RAT_PI_NOM) << 32) | INT64_C(_RAT_PI_DENOM))
#define RAT_E64  ((INT64_C(_RAT_E_NOM) << 32) | INT64_C(_RAT_E_DENOM))
#endif

#define zpugetnom64(i64)      (((struct zpurat *)&i64)->nom)
#define zpusetnom64(i64, n)   (((struct zpurat *)&i64)->nom = (n))
#define zpugetdenom64(i64)    (((struct zpurat *)&i64)->denom)
#define zpusetdenom64(i64, d) (((struct zpurat *)&i64)->denom = (d))
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
#define ratisneginf(rp) (*(int64_t *)(rp) == INT64_C(0x0000000080000000))
#else
#define ratisposinf(rp) (*(int64_t *)(rp) == INT64_C(0x7fffffff00000000))
#define ratisneginf(rp) (*(int64_t *)(rp) == INT64_C(0x8000000000000000))
#endif
#else
#define ratiszero(rp)   (!(rp)->nom)
#define ratisnan(rp)    (!(rp)->nom && !(rp)->denom)
#define ratisposinf(rp) ((rp)->nom == 0x7fffffff && !(rp)->denom)
#define ratisneginf(rp) ((rp)->nom == 0x80000000 && !(rp)->denom)
#endif

void radd(struct zpu *zpu, struct zpuop *op);
void rsub(struct zpu *zpu, struct zpuop *op);
void rmul(struct zpu *zpu, struct zpuop *op);
void rdiv(struct zpu *zpu, struct zpuop *op);

#endif /* ZPURAT */

#endif /* __ZPU_RAT_H__ */

