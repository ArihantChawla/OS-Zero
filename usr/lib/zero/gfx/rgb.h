#ifndef __ZERO_GFX_RGB_H__
#define __ZERO_GFX_RGB_H__

#include <stdint.h>
#include <endian.h>
#include <zero/param.h>
#include <zero/cdecl.h>

#define RGBARGB32 0
#define RGBRGB888 1
#define RGBRGB555 2
#define RGBRGB565 3

#define RGBBLACK  0x00000000
#define RGBWHITE  0x00ffffff
#define RGBGREEN  0x0000bf00

#if (__BYTE_ORDER == __LITTLE_ENDIAN)

#define RGBALPHAOFS 24
#define RGBREDOFS   16
#define RGBGREENOFS 8
#define RGBBLUEOFS  0

struct argb32 {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
} PACK();

#elif (__BYTE_ORDER == __BIG_ENDIAN)

#define RGBALPHAOFS 0
#define RGBREDOFS   8
#define RGBGREENOFS 16
#define RGBBLUEOFS  24

struct argb32 {
    uint8_t alpha;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} PACK();

#endif

typedef int32_t argb32_t;
typedef int16_t rgb555_t;
typedef int16_t rgb565_t;

/* pix is 32-bit word */
#define gfxgetalpha(pix) ((pix) >> RGBALPHAOFS)		 // alpha component
#define gfxgetred(pix)   (((pix) >> RGBREDOFS) & 0xff)	 // red component
#define gfxgetgreen(pix) (((pix) >> RGBGREENOFS) & 0xff) // green component
#define gfxgetblue(pix)  (((pix) >> RGBBLUEOFS) & 0xff)	 // blue component

/* pointer version; faster byte-fetches from memory */
#define gfxgetalpha_p(p) (((struct argb32 *)(p))->alpha)
#define gfxgetred_p(p)   (((struct argb32 *)(p))->red)
#define gfxgetgreen_p(p) (((struct argb32 *)(p))->green)
#define gfxgetblue_p(p)  (((struct argb32 *)(p))->blue)

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
    (((a) << RGBALPHAOFS)                                               \
     | ((r) << RGBREDOFS)                                               \
     | ((g) << RGBGREENOFS)                                             \
     | ((b) << RGBBLUEOFS))
#define gfxmkpix_p(dest, a, r, g, b)                                    \
    ((dest) = gfxmkpix(a, r, g, b))
#define gfxsetpix_p(p, a, r, g, b)                                      \
    (((struct argb32 *)(p))->alpha = (a),                               \
     ((struct argb32 *)(p))->red = (r),                                 \
     ((struct argb32 *)(p))->green = (g),                               \
     ((struct argb32 *)(p))->blue = (b))

#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define RGB_ARGB32_RED_SHIFT       16
#define RGB_ARGB32_GREEN_SHIFT     8
#define RGB_ARGB32_BLUE_SHIFT      0
#define RGB_RGB888_RED_OFS         2
#define RGB_RGB888_GREEN_OFS       1
#define RGB_RGB888_BLUE_OFS        0
#else
#define RGB_RGB888_RED_OFS         0
#define RGB_RGB888_GREEN_OFS       1
#define RGB_RGB888_BLUE_OFS        2
#endif

#define RGB_RGB555_RED_MASK        0x00007c00
#define RGB_RGB555_GREEN_MASK      0x000003e0
#define RGB_RGB555_BLUE_MASK       0x0000001f
//#define RGB_RGB555_MASK            0x00007fff

#define RGB_RGB555_RED_SHIFT       7
#define RGB_RGB555_GREEN_SHIFT     2
#define RGB_RGB555_BLUE_SHIFT     -3

#define RGB_RGB565_RED_MASK        0x0000f800
#define RGB_RGB565_GREEN_MASK      0x000007e0
#define RGB_RGB565_BLUE_MASK       0x0000001f
//#define RGB_RGB565_MASK            0x0000ffff

#define RGB_RGB565_RED_SHIFT       8
#define RGB_RGB565_GREEN_SHIFT     3
#define RGB_RGB565_BLUE_SHIFT     -3

#define gfxtopix(dst, u) (gfxto##dst(u))
#define gfxtoargb32(u)                                                  \
    (u)

#define gfxtorgb555(u)                                                  \
    (gfxtoc(gfxgetred(u),                                               \
            RGB_RGB555_RED_MASK,                                        \
            RGB_RGB555_RED_SHIFT)                                       \
     | gfxtoc(gfxgetgreen(u),                                           \
              RGB_RGB555_GREEN_MASK,                                    \
              RGB_RGB555_GREEN_SHIFT)                                   \
     | gfxtoc(gfxgetblue(u),                                            \
              RGB_RGB555_BLUE_MASK,                                     \
              RGB_RGB555_BLUE_SHIFT))

#define gfxtorgb565(u)                                                  \
    (gfxtoc(gfxgetred(u),                                               \
            RGB_RGB565_RED_MASK,                                        \
            RGB_RGB565_RED_SHIFT)                                       \
     | gfxtoc(gfxgetgreen(u),                                           \
              RGB_RGB565_GREEN_MASK,                                    \
              RGB_RGB565_GREEN_SHIFT)                                   \
     | gfxtoc(gfxgetblue(u),                                            \
              RGB_RGB565_BLUE_MASK,                                     \
              RGB_RGB565_BLUE_SHIFT))

#define gfxsetrgb888_p(u, p)                                            \
    do {                                                                \
        argb32_t       _pix = (u);                                      \
        struct argb32 *_src = (struct argb32 *)&_pix;                   \
        struct argb32 *_dest = (struct argb32 *)p;                      \
                                                                        \
        _dest->red = gfxgetred_p(_src);                                 \
        _dest->green = gfxgetgreen_p(_src);                             \
        _dest->blue = gfxgetblue_p(_src);                               \
    } while (0)

#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define gfxsetrgb888(u, p)                                              \
    (((uint8_t *)(p))[2] = gfxgetred(u),                                \
     ((uint8_t *)(p))[1] = gfxgetgreen(u),                              \
     ((uint8_t *)(p))[0] = gfxgetblue(u))
#else
#define gfxsetrgb888(u, p)                                              \
    (((uint8_t *)(p))[0] = gfxgetred(u),                                \
     ((uint8_t *)(p))[1] = gfxgetgreen(u),                              \
     ((uint8_t *)(p))[2] = gfxgetblue(u))
#endif
#define gfxtoc(u, m, s)                                                 \
    ((s) > 0 ? (((u) >> (s)) & (m)) : (((u) << -(s)) & (m)))

#endif /* __ZERO_GFX_RGB_H__ */

