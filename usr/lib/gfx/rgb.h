#ifndef __GFX_RGB_H__
#define __GFX_RGB_H__

#include <stdint.h>
#include <endian.h>
#include <zero/cdecl.h>
#include <zero/param.h>

#define GFX_ARGB32 1
#define GFX_RGB888 2
#define GFX_RGB555 3
#define GFX_RGB565 4

#define GFX_BLACK  0x00000000
#define GFX_WHITE  0xffffffff
#define GFX_GREEN  0x0000bf00

#if (__BYTE_ORDER == __LITTLE_ENDIAN)

#define GFX_ALPHA_OFS 24
#define GFX_RED_OFS   16
#define GFX_GREEN_OFS 8
#define GFX_BLUE_OFS  0

struct gfxargb32 {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
};

#elif (__BYTE_ORDER == __BIG_ENDIAN)

#define GFX_ALPHA_OFS 0
#define GFX_RED_OFS   8
#define GFX_GREEN_OFS 16
#define GFX_BLUE_OFS  24

struct gfxargb32 {
    uint8_t alpha;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

#endif /* __BYTE_ORDER */

typedef int32_t gfxargb32_t;
typedef int16_t gfxrgb555_t;
typedef int16_t gfxrgb565_t;

/* pix is 32-bit word */
#define gfxgetalpha(pix) ((pix) >> GFX_ALPHA_OFS)          // alpha component
#define gfxgetred(pix)   (((pix) >> GFX_RED_OFS) & 0xff)   // red component
#define gfxgetgreen(pix) (((pix) >> GFX_GREEN_OFS) & 0xff) // green component
#define gfxgetblue(pix)  (((pix) >> GFX_BLUE_OFS) & 0xff)  // blue component

/* pointer version; faster byte-fetches from memory */
#define gfxgetalpha_p(ptr) (((struct gfxargb32 *)(ptr))->alpha)
#define gfxgetred_p(ptr)   (((struct gfxargb32 *)(ptr))->red)
#define gfxgetgreen_p(ptr) (((struct gfxargb32 *)(ptr))->green)
#define gfxgetblue_p(ptr)  (((struct gfxargb32 *)(ptr))->blue)

/* approximation for c / 0xff */
#define gfxdiv255(c)                                                    \
    ((((c) << 8) + (c) + 256) >> 16)
/* simple division by 256 by bitshift */
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
    (((a) << GFX_ALPHA_OFS)                                             \
     | ((r) << GFX_RED_OFS)                                             \
     | ((g) << GFX_GREEN_OFS)                                           \
     | ((b) << GFX_BLUE_OFS))
#define gfxmkpix_p(dest, a, r, g, b)                                    \
    ((dest) = gfxmkpix(a, r, g, b))
#define gfxsetpix_p(p, a, r, g, b)                                      \
    (((struct gfxargb32 *)(ptr))->alpha = (a),                          \
     ((struct gfxargb32 *)(ptr))->red = (r),                            \
     ((struct gfxargb32 *)(ptr))->green = (g),                          \
     ((struct gfxargb32 *)(ptr))->blue = (b))

#if (__BYTE_ORDER == __LITTLE_ENDIAN)

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

#define GFX_RGB555_RED_SHIFT       7
#define GFX_RGB555_GREEN_SHIFT     2
#define GFX_RGB555_BLUE_SHIFT     -3

#define GFX_RGB565_RED_MASK        0x0000f800
#define GFX_RGB565_GREEN_MASK      0x000007e0
#define GFX_RGB565_BLUE_MASK       0x0000001f
//#define GFX_RGB565_MASK            0x0000ffff

#define GFX_RGB565_RED_SHIFT       8
#define GFX_RGB565_GREEN_SHIFT     3
#define GFX_RGB565_BLUE_SHIFT     -3

#define gfxtopix(dst, pix) (gfxto##dst(pix))
#define gfxtoargb32(pix)   (pix)

#define gfxtorgb555(pix)                                                \
    (gfxtoc(gfxgetred(pix),                                             \
            GFX_RGB555_RED_MASK,                                        \
            GFX_RGB555_RED_SHIFT)                                       \
     | gfxtoc(gfxgetgreen(pix),                                         \
              GFX_RGB555_GREEN_MASK,                                    \
              GFX_RGB555_GREEN_SHIFT)                                   \
     | gfxtoc(gfxgetblue(pix),                                          \
              GFX_RGB555_BLUE_MASK,                                     \
              GFX_RGB555_BLUE_SHIFT))

#define gfxtorgb565(pix)                                                \
    (gfxtoc(gfxgetred(pix),                                             \
            GFX_RGB565_RED_MASK,                                        \
            GFX_RGB565_RED_SHIFT)                                       \
     | gfxtoc(gfxgetgreen(pix),                                         \
              GFX_RGB565_GREEN_MASK,                                    \
              GFX_RGB565_GREEN_SHIFT)                                   \
     | gfxtoc(gfxgetblue(pix),                                          \
              GFX_RGB565_BLUE_MASK,                                     \
              GFX_RGB565_BLUE_SHIFT))

#define gfxsetrgb888_p(pix, ptr)                                        \
    do {                                                                \
        gfxargb32_t       _pix = (pix);                                 \
        struct gfxargb32 *_src = (struct gfxargb32 *)&_pix;             \
        struct gfxargb32 *_dest = (struct gfxargb32 *)p;                \
                                                                        \
        _dest->red = gfxgetred_p(_src);                                 \
        _dest->green = gfxgetgreen_p(_src);                             \
        _dest->blue = gfxgetblue_p(_src);                               \
    } while (0)

#if (__BYTE_ORDER == __LITTLE_ENDIAN)

#define gfxsetrgb888(pix, ptr)                                          \
    (((uint8_t *)(ptr))[2] = gfxgetred(pix),                            \
     ((uint8_t *)(ptr))[1] = gfxgetgreen(pix),                          \
     ((uint8_t *)(ptr))[0] = gfxgetblue(pix))

#elif (__BYTE_ORDER == __BIG_ENDIAN)

#define gfxsetrgb888(pix, ptr)                                          \
    (((uint8_t *)(ptr))[0] = gfxgetred(pix),                            \
     ((uint8_t *)(ptr))[1] = gfxgetgreen(pix),                          \
     ((uint8_t *)(ptr))[2] = gfxgetblue(pix))

#endif /* __BYTE_ORDER */

#define gfxtoc(pix, m, s)                                               \
    ((s) > 0 ? (((pix) >> (s)) & (m)) : (((pix) << -(s)) & (m)))

#endif /* __GFX_RGB_H__ */

