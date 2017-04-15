#ifndef __DECK_LFB_H__
#define __DECK_LFB_H__

/* linear [graphics] framebuffer structure */

/* pixfmt values */
#define DECK_PIXEL_BGRA 0 // little-endian (x86)
struct decklfb {
    deckuword  w;       // width
    deckuword  h;       // height
    deckuword  pixfmt;  // format of pixels
    void      *base;    // base address
    void      *drawbuf; // double-buffer for drawing
};

#endif /* __DECK_LFB_H__ */

