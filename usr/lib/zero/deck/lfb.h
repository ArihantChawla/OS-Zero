#ifndef __ZERO_DECK_LFB_H__
#define __ZERO_DECK_LFB_H__

#include <stdint.h>

/* linear [graphics] framebuffer structure */

/* pixfmt values */
#define DECK_PIXEL_BGRA 0       // little-endian (x86)
struct decklfb {
    uint32_t  screen;   // screen ID
    uint32_t  pixfmt;   // pixel-format
    uint32_t  depth;    // pixel-size in bits
    uint32_t  width;    // width in pixels
    uint32_t  height;   // height in pixels
    void     *base;     // base address
    void     *drawbuf;  // double-buffer for drawing
    void     *scrbuf;   // screen buffer
};

#endif /* __ZERO_DECK_LFB_H__ */

