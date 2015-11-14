#ifndef __DECK_GC_H__
#define __DECK_GC_H__

#include <deck/conf.h>
#include <gfx/rgb.h>

/* graphics context structure */
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
    deckuword_t screen; // screen ID
    deckuword_t font;   // font ID
    deckuword_t charset; // character set for drawing text
    deckuword_t func;   // raster-operation
    deckpix_t   mask;   // plane-mask to choose bits from source pixel
    deckpix_t   fgpix;  // foreground pixel value
    deckpix_t   bgpix;  // background pixel value
    deckpix_t   _pad;
};

/* graphics context attributes for creating and changing GCs */
/* flg-field bits */
#define DECK_GC_FONT    (1U << 0)
#define DECK_GC_FUNC    (1U << 1)
#define DECK_GC_CHARSET (1U << 2)
#define DECK_GC_MASK    (1U << 3)
#define DECK_GC_FGPIX   (1U << 4)
#define DECK_GC_BGPIX   (1U << 5)
struct deckgcatr {
    deckuword_t   flg;  // bits to choose fields from vals
    struct deckgc vals; // attribute values
};

#endif /* __DECK_GC_H__ */

