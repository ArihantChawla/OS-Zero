#ifndef __ZERO_DECK_GC_H__
#define __ZERO_DECK_GC_H__

#include <stdint.h>

/* graphics context structure */
/* flg-field bits */
#define DECK_GC_DRAW_BG    0x00000001U
/* func values */
#define DECK_GC_COPY       1
#define DECK_GC_COPYINVERT 2
#define DECK_GC_CLEAR      3
#define DECK_GC_SET        4
#define DECK_GC_NOP        5
#define DECK_GC_INVERT     6
#define DECK_GC_AND        7
#define DECK_GC_ANDREVERSE 8
#define DECK_GC_ANDINVERT  9
#define DECK_GC_OR         10
#define DECK_GC_ORINVERT   11
#define DECK_GC_ORREVERSE  12
#define DECK_GC_XOR        13
#define DECK_GC_NOR        14
#define DECK_GC_EQ         15
#define DECK_GC_NAND       16
struct deckgc {
    uint32_t screen;    // screen ID
    uint32_t font;      // font ID
    uint32_t func;      // raster-operation
    uint32_t mask;      // plane-mask to choose bits from source pixel
    uint32_t fgpix;     // foreground pixel value
    uint32_t bgpix;     // background pixel value
};

/* flg-field bits */
#define DECK_GC_FONT  0x00000001U
#define DECK_GC_FUNC  0x00000002U
#define DECK_GC_MASK  0x00000004U
#define DECK_GC_FGPIX 0x00000008U
#define DECK_GC_BGPIX 0x00000010U
struct deckgcatr {
    uint32_t      flg;
    struct deckgc vals;
};

/* raster operations */
/*
 * EXAMPLES
 * --------
 * *dest = deckpixop(copy, src, dest);
 * *dest = deckmaskop(copy, src, dest, mask);
 */
#define __deckpixop(op, s, d)     (__deck##op##pix(s, d))
#define __deckmaskop(op, s, d, m)                                       \
    ((__deckpixop(op, (s), (d)) & (m)) | ((d) & ~(m)))
#define __deckcopypix(s, d)       (*(s))
#define __deckcopyinvpix(s, d)    (~(*(s)))
#define __deckclearpix(s, d)      ((uint32_t)0)
#define __decksetpix(s, d)        (~((uint32_t)0))
#define __decknoppix(s, d)        ((void)0)
#define __deckinvpix(s, d)        (~(*(d)))
#define __deckandpix(s, d)        ((*(s)) & (*(d)))
#define __deckandrevpix(s, d)     ((*(s)) & ~(*(d)))
#define __deckandinvpix(s, d)     (~(*(s)) & (*(d)))
#define __deckorpix(s, d)         ((*(s)) | (*(d)))
#define __deckorinvpix(s, d)      (~(*(s)) | (*(d)))
#define __deckorrevpix(s, d)      ((*(s)) | ~(*(d)))
#define __deckxorpix(s, d)        ((*(s)) ^ (*(d)))
#define __decknorpix(s, d)        (~(*(s)) & ~(*(d)))
#define __deckeqpix(s, d)         (~(*(s)) ^ (*(d)))
#define __decknandpix(s, d)       (~(*(s)) & (*(d)))

#endif /* __ZERO_DECK_GC_H__ */

