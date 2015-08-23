#ifndef __GFX_PIXOP_H__
#define __GFX_PIXOP_H__

#include <gfx/rgb.h>

/* raster operations */
/*
 * EXAMPLES
 * --------
 * *dest = gfxpixop(copy, src, dest);
 * *dest = gfxmaskop(xor, src, dest, mask);
 */
#define gfxpixop(op, s, d)       (__gfx##op##pix(s, d))
#define gfxmaskop(op, s, d, m)                                          \
    ((__gfxpixop(op, (s), (d)) & (m)) | ((d) & ~(m)))
#define __gfxcopypix(s, d)       (*(s))
#define __gfxcopyinvpix(s, d)    (~(*(s)))
#define __gfxclearpix(s, d)      ((gfxargb32_t)0)
#define __gfxsetpix(s, d)        (~((gfxargb32_t)0))
#define __gfxnoppix(s, d)        ((void)0)
#define __gfxinvpix(s, d)        (~(*(d)))
#define __gfxandpix(s, d)        ((*(s)) & (*(d)))
#define __gfxandrevpix(s, d)     ((*(s)) & ~(*(d)))
#define __gfxandinvpix(s, d)     (~(*(s)) & (*(d)))
#define __gfxorpix(s, d)         ((*(s)) | (*(d)))
#define __gfxorinvpix(s, d)      (~(*(s)) | (*(d)))
#define __gfxorrevpix(s, d)      ((*(s)) | ~(*(d)))
#define __gfxxorpix(s, d)        ((*(s)) ^ (*(d)))
#define __gfxnorpix(s, d)        (~(*(s)) & ~(*(d)))
#define __gfxeqpix(s, d)         (~(*(s)) ^ (*(d)))
#define __gfxnandpix(s, d)       (~(*(s)) & (*(d)))

#endif /* __GFX_PIXOP_H__ */

