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
    uint32_t mask;      // plane-mask
    uint32_t fgpix;     // foreground pixel value
    uint32_t bgpix;     // background pixel value
};

/* flg-field bits */
#define DECK_GC_FONT  0x00000001
#define DECK_GC_FUNC  0x00000002
#define DECK_GC_MASK  0x00000004
#define DECK_GC_FGPIX 0x00000008
#define DECK_GC_BGPIX 0x00000010
struct deckgcatr {
    uint32_t      flg;
    struct deckgc vals;
};

/* raster operation macros */
#define __deckcopyop(s, d)    (*(d) = *(s))
#define __deckcopyinvop(s, d) (*(d) = ~(*(s)))
#define __deckclearop(s, d)   (*(d) = (uint32_t)0)
#define __decksetop(s, d)     (*(d) = ~((uint32_t)0))
#define __decknopop(s, d)     ((void)0)
#define __deckinvop(s, d)     (*(d) = ~(*(d)))
#define __deckandop(s, d)     (*(d) = (*(s)) & (*(d)))
#define __deckandrevop(s, d)  (*(d) = (*(s)) & ~(*(d)))
#define __deckandinvop(s, d)  (*(d) = ~(*(s)) & (*(d)))
#define __deckorop(s, d)      (*(d) = (*(s)) | (*(d)))
#define __deckorinvop(s, d)   (*(d) = ~(*(s)) | (*(d)))
#define __deckorrevop(s, d)   (*(d) = (*(s)) | ~(*(d)))
#define __deckxorop(s, d)     (*(d) = (*(s)) ^ (*(d)))
#define __decknorop(s, d)     (*(d) = ~(*(s)) & ~(*(d)))
#define __deckeqop(s, d)      (*(d) = ~(*(s)) ^ (*(d)))
#define __decknandop(s, d)    (*(d) = ~(*(s)) & (*(d)))

#endif /* __ZERO_DECK_GC_H__ */

