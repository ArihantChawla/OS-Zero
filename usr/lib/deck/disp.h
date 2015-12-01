#ifndef __DECK_DISP_H__
#define __DECK_DISP_H__

#include <stdint.h>
#include <deck/deck.h>

struct deckdisp {
    deckuword_t  screen; // screen ID
    deckuword_t  pixfmt; // pixel-format
    deckuword_t  visual; // visual type
    deckuword_t  depth;  // pixel-size in bits
    deckuword_t  width;  // width in pixels
    deckuword_t  height; // height in pixels
    void        *gfx; // graphics interface
};

#endif /* __DECK_DISP_H__ */

