#ifndef __ZERO_GFX_XFADE_H__
#define __ZERO_GFX_XFADE_H__

#include <stdint.h>
#include <zero/gfx/rgb.h>
#include <zero/gfx/alpha.h>

#if 0
#define gfxxfade1(src1, src2, dest, val)                                \
    do {                                                                \
        gfxalphablend_hiq_const(src1, dest, 0xff - (val));              \
        gfxalphablend_hiq_const(src2, dest, (val));                     \
    } while (0)
#endif

#define gfxxfade2(src1, src2, dest, val)                                \
    do {                                                                \
        gfxalphablendfast_const2(src1, dest, 0xff - (val));             \
        gfxalphablendfast_const2(src2, dest, (val));                    \
    } while (0)

#define gfxxfade1_mmx(src1, src2, dest, val)                            \
    do {                                                                \
        gfxalphablendloq_asm_mmx(src1, dest, 0xff - (val));             \
        gfxalphablendloq_asm_mmx(src2, dest, (val));                    \
    } while (0)

#if (__INTEL_MMX__)
#define gfxxfade2_mmx(src1, src2, dest, val)                            \
    do {                                                                \
        gfxalphablendloq_intel_mmx(src1, dest, 0xff - (val));           \
        gfxalphablendloq_intel_mmx(src2, dest, (val));                  \
    } while (0)

#endif

/* tested OK */
#define gfxxfade1_jose(src1, src2, dest, val)                           \
    do {                                                                \
        argb32_t _tmp;                                                  \
                                                                        \
        _tmp = INTERP_256(0xff - (val), src1, dest);                    \
        (dest) = INTERP_256((val), src2, _tmp);                         \
    } while (0)
#define gfxxfade2_jose(src1, src2, dest, val)                           \
    do {                                                                \
        gfxalphablend_nodiv(src1, dest, 0xff - (val));                  \
        gfxalphablend_nodiv(src2, dest, (val));                         \
    } while (0)

#endif /* __ZERO_GFX_XFADE_H__ */

