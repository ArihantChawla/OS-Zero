#ifndef __MM_GFX_XFADE_H__
#define __MM_GFX_XFADE_H__

#include <stdint.h>
#include <gfx/rgb.h>
#include <gfx/alpha.h>

#define gfxxfade1(src1, src2, dest, val)                                \
    do {                                                                \
        gfxalphablend_hiq_const(src1, dest, 0xff - (val));              \
        gfxalphablend_hiq_const(src2, dest, (val));                     \
    } while (FALSE)

#define gfxxfade2(src1, src2, dest, val)                                \
    do {                                                                \
        gfxalphablendfast_const(src1, dest, 0xff - (val));              \
        gfxalphablendfast_const(src2, dest, (val));                     \
    } while (FALSE)

#define gfxxfade1_mmx(src1, src2, dest, val)                            \
    do {                                                                \
        gfxalphablendloq_asm_mmx(src1, dest, 0xff - (val));             \
        gfxalphablendloq_asm_mmx(src2, dest, (val));                    \
    } while (FALSE)

/* tested OK */
#define gfxxfade1_jose(src1, src2, dest, val)                           \
    do {                                                                \
        argb32_t _tmp;                                                  \
                                                                        \
        _tmp = INTERP_256(0xff - (val), src1, dest);                    \
        (dest) = INTERP_256((val), src2, _tmp);                         \
    } while (FALSE)
#define gfxxfade2_jose(src1, src2, dest, val)                           \
    do {                                                                \
        gfxalphablend_nodiv(src1, dest, 0xff - (val));                  \
        gfxalphablend_nodiv(src2, dest, (val));                         \
    } while (FALSE)

#endif /* __MM_GFX_XFADE_H__ */

