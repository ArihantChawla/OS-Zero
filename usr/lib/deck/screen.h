#ifndef __DECK_SCREEN_H__
#define __DECK_SCREEN_H__

#include <deck/conf.h>
#include <deck/types.h>

#if (SUPPORT_DECK_GFX_LFB)
#include <deck/lfb.h>
#endif

struct deckscreen {
    deckuword  screen;  // screen ID
    deckuword  pixfmt;  // pixel-format
    deckuword  visual;  // visual type
    deckuword  depth;   // pixel-size in bits
    deckuword  width;   // width in pixels
    deckuword  height;  // height in pixels
    void      *gfx;     // graphics interface, e.g. struct decklfb
};

#endif /* __DECK_SCREEN_H__ */

