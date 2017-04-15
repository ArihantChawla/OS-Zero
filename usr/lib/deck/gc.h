#ifndef __DECK_GC_H__
#define __DECK_GC_H__

#include <deck/types.h>
#include <gfx/rgb.h>

/* graphics context structure */
/* func values; initially, we use the same as X11/XOrg */
#define DECK_GC_CLEAR       3
#define DECK_GC_AND         7
#define DECK_GC_AND_REVERSE 8
#define DECK_GC_COPY        1
#define DECK_GC_AND_INVERT  9
#define DECK_GC_NOOP         5
#define DECK_GC_XOR         13
#define DECK_GC_OR          10
#define DECK_GC_NOR         14

#define DECK_GC_EQUIV       15
#define DECK_GC_INVERT      6
#define DECK_GC_OR_REVERSE  12
#define DECK_GC_COPY_INVERT 2
#define DECK_GC_OR_INVERT   11
#define DECK_GC_NAND        16
#define DECK_GC_SET         4
struct deckgc {
    deckuword screen;   // screen ID
    deckuword func;     // raster-operation
    deckpix   fgpix;    // foreground pixel value
    deckpix   bgpix;    // background pixel value
    deckuword font;     // font ID
    deckuword charset;  // character set for drawing text
    deckpix   mask;     // plane-mask to choose bits from source pixel
    deckpix   _pad;
};

/* graphics context attributes for creating and changing GCs */
/* flg-field bits */
#define DECK_GC_FUNC    (1U << 0)
#define DECK_GC_FGPIX   (1U << 1)
#define DECK_GC_BGPIX   (1U << 2)
#define DECK_GC_FONT    (1U << 3)
#define DECK_GC_CHARSET (1U << 4)
#define DECK_GC_MASK    (1U << 5)
struct deckgcatr {
    deckuword     flg;  // bits to choose fields from vals
    struct deckgc vals; // attribute values
};

#define _deckclear(s, d)         (0)
#define _deckand(s, d)           ((s) & (d))
#define _deckandreverse(s, d)    ((s) & (~(d)))
#define _deckcopy(s, d)          (s)
#define _deckandinvert(s, d)     ((~(s)) & (d))
#define _decknoop(s, d)          (d)
#define _deckxor(s, d)           ((s) ^ (d))
#define _deckor(s, d)            ((s) | (d))
#define _decknor(s, d)           ((~(s)) & (~(d)))
#define _deckequiv(s, d)         ((~(s)) ^ (d))
#define _deckinvert(s, d)        (~(d))
#define _deckorreverse(s, d)     ((s) | (~(d)))
#define _deckcopyinvert(s, d)    (~(s))
#define _deckorinvert(s, d)      ((~(s)) | (d))
#define _decknand(s, d)          (~((s) & (d)))
#define _deckset(s, d)           (0xffffffff)

#define deckclear32(s, ptr)      (*(ptr) = 0)
#define deckand32(s, ptr)        (*(ptr) = _deckand(s, *(ptr)))
#define deckandreverse32(s, ptr) (*(ptr) = _deckandreverse(s, *(ptr)))
#define deckcopy32(s, ptr)       (*(ptr) = _deckcopy(s, *(ptr)))
#define deckandinvert32(s, ptr)  (*(ptr) = _deckandinvert(s, *(ptr)))
#define decknoop32(s, ptr)       ((void)0)
#define deckxor32(s, ptr)        (*(ptr) = _deckxor(s, *(ptr)))
#define deckor32(s, ptr)         (*(ptr) = _deckor(s, *(ptr)))
#define decknor32(s, ptr)        (*(ptr) = _decknor(s, *(ptr)))
#define deckequiv32(s, ptr)      (*(ptr) = _deckequiv(s, *(ptr)))
#define deckinvert32(s, ptr)     (*(ptr) = _deckinvert(s, *(ptr)))
#define deckorreverse32(s, ptr)  (*(ptr) = _decorreverse(s, *(ptr)))
#define deckcopyinvert32(s, ptr) (*(ptr) = _deckcopyinvert(s, *(ptr)))
#define deckorinvert32(s, ptr)   (*(ptr) = _deckorinvert(s, *(ptr)))
#define decknand32(s, ptr)       (*(ptr) = _decknand(s, *(ptr)))
#define deckset32(s, ptr)        (*(ptr) = 0xffffffff)

#endif /* __DECK_GC_H__ */

