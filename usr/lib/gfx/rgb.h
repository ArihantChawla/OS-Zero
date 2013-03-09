#ifndef __GFX_RGB_H__
#define __GFX_RGB_H__

#include <stdint.h>
#include <endian.h>

#define GFXARGB32 0
#define GFXRGB888 1
#define GFXRGB555 2
#define GFXRGB565 3

#define GFXALPHAOFS 24
#define GFXREDOFS   16
#define GFXGREENOFS 8
#define GFXBLUEOFS  0

typedef int32_t argb32_t;
typedef int16_t rgb555_t;
typedef int16_t rgb565_t;

struct argb32 {
    uint8_t bval;
    uint8_t gval;
    uint8_t rval;
    uint8_t aval;
};

/* pix is 32-bit word */
#define gfxalphaval(pix) ((pix) >> GFXALPHAOFS)		 // alpha component
#define gfxredval(pix)   (((pix) >> GFXREDOFS) & 0xff)	 // red component
#define gfxgreenval(pix) (((pix) >> GFXGREENOFS) & 0xff) // green component
#define gfxblueval(pix)  (((pix) >> GFXBLUEOFS) & 0xff)	 // blue component

/* pointer version; faster byte-fetches from memory */
#define gfxalphaval_p(p) (((struct argb32 *)(p))->aval)
#define gfxredval_p(p)   (((struct argb32 *)(p))->rval)
#define gfxgreenval_p(p) (((struct argb32 *)(p))->gval)
#define gfxblueval_p(p)  (((struct argb32 *)(p))->bval)

/* approximation for c / 0xff */
#define gfxdiv255(c)                                                    \
    ((((c) << 8) + (c) + 256) >> 16)
/* simple division per 256 by bitshift */
#define gfxdiv256(c)                                                    \
    ((c) >> 8)
#define gfxalphablendc(src, dest, a)                                    \
    ((dest) + gfxdiv255(((src) - (dest)) * (a)))
#define gfxalphablendc2(src, dest, a)                                   \
    ((dest) + gfxdiv256(((src) - (dest)) * (a)))
#define gfxalphablendcf(src, dest, a)                                   \
    ((dest) + (((src) - (dest)) * (a)) / 255.0)

/* compose pixel value from components */
#define gfxmkpix(a, r, g, b)                                            \
    (((a) << GFXALPHAOFS)                                               \
     | ((r) << GFXREDOFS)                                               \
     | ((g) << GFXGREENOFS)                                             \
     | ((b) << GFXBLUEOFS))
#define gfxmkpix_p(dest, a, r, g, b)                                    \
    ((dest) = gfxmkpix(a, r, g, b))
#define gfxsetpix_p(p, a, r, g, b)                                      \
    (((struct argb32 *)(p))->aval = (a),                                \
     ((struct argb32 *)(p))->rval = (r),                                \
     ((struct argb32 *)(p))->gval = (g),                                \
     ((struct argb32 *)(p))->bval = (b))

#if (_BYTE_ORDER == _LITTLE_ENDIAN)
#define GFX_ARGB32_RED_SHIFT       16
#define GFX_ARGB32_GREEN_SHIFT     8
#define GFX_ARGB32_BLUE_SHIFT      0
#define GFX_RGB888_RED_OFS         2
#define GFX_RGB888_GREEN_OFS       1
#define GFX_RGB888_BLUE_OFS        0
#else
#define GFX_RGB888_RED_OFS         0
#define GFX_RGB888_GREEN_OFS       1
#define GFX_RGB888_BLUE_OFS        2
#endif

#define GFX_RGB555_RED_MASK        0x00007c00
#define GFX_RGB555_GREEN_MASK      0x000003e0
#define GFX_RGB555_BLUE_MASK       0x0000001f
//#define GFX_RGB555_MASK            0x00007fff

#define GFX_RGB555_RED_SHIFT       9
#define GFX_RGB555_GREEN_SHIFT     6
#define GFX_RGB555_BLUE_SHIFT      3

#define GFX_RGB565_RED_MASK        0x0000f800
#define GFX_RGB565_GREEN_MASK      0x000007e0
#define GFX_RGB565_BLUE_MASK       0x0000001f
//#define GFX_RGB565_MASK            0x0000ffff

#define GFX_RGB565_RED_SHIFT       8
#define GFX_RGB565_GREEN_SHIFT     5
#define GFX_RGB565_BLUE_SHIFT      3

#define gfxtopix(dst, u)                                                \
    (_gfx_to##dst(u))

#define _gfxtonone(u)                                                   \
    (u)
#define _gfxtorgb555(u)                                                 \
    (_gfxtoc(gfxredval(u),                                              \
             GFX_TO_RGB555_RED_MASK,                                    \
             GFX_RGB555_RED_SHIFT)                                      \
     | _gfxtoc(gfxgreenval(u),                                          \
               GFX_TO_RGB555_GREEN_MASK,                                \
               GFX_RGB555_GREEN_SHIFT)                                  \
     | _gfxtoc(gfxblueval(u),                                           \
               GFX_TO_RGB555_BLUE_MASK,                                 \
               GFX_RGB555_BLUE_SHIFT))

#define _gfxtorgb565(u)                                                 \
    (_gfxtoc(gfxredval(u),                                              \
             GFX_TO_RGB565_RED_MASK,                                    \
             GFX_RGB565_RED_SHIFT)                                      \
     | _gfxtoc(gfxgreenval(u),                                          \
               GFX_TO_RGB565_GREEN_MASK,                                \
               GFX_RGB565_GREEN_SHIFT)                                  \
     | _gfxtoc(gfxblueval(u),                                           \
               GFX_TO_RGB565_BLUE_MASK,                                 \
               GFX_RGB565_BLUE_SHIFT))

#define gfxtorgb888(u, p)                                               \
    (((uint8_t *)(p))[GFXREDOFS] = gfxredval(u),                        \
     ((uint8_t *)(p))[GFXGREENOFS] = gfxgreenval(u),                    \
     ((uint8_t *)(p))[GFXBLUEOFS] = gfxblueval(u))
#define _gfxtoc(u, m, s)                                                \
    (((u) >> s) & (m))

#endif /* __GFX_RGB_H__ */

