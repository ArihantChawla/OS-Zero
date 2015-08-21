#ifndef __ZERO_DECK_H__
#define __ZERO_DECK_H__

#include <stdint.h>
#include <endian.h>
#include <zero/ev.h>

/* endian-field values */
#define DECK_LITTLE_ENDIAN 0
#define DECK_BIG_ENDIAN    1
/* protocol-field values */
#define DECK_CONN_SHM      0
#define DECK_CONN_IP4      1
#define DECK_CONN_IP6      2
/* gfxtype-field values */
#define DECK_GFX_LFB       0  // use linear framebuffer for graphics
/* structure for functionality similar to X and such desktop/screen servers */
struct deck {
    /* screen ID (similar to Display */
    uint32_t  scrid;    // screen ID
    long      endian;   // server-machine word endianess
    long      conntype; // connection type
    long      gfxtype;  // graphics interface such as DECK_GFX_LFB
    void     *gfx;      // graphics interface
};

struct deckfb {
    uint32_t  depth;    // framebuffer pixel-size in bits
    uint32_t  width;    // framebuffer width in pixels
    uint32_t  height;   // framebuffer height in pixels
    void     *base;     // framebuffer base address
    void     *drawbuf;  // double-buffer for drawing
#if 0
    void     *scrbuf;   // screen buffer
#endif
};

/* flg-field bits */
#define DECK_GC_DRAW_BG 0x00000000U
struct deckgc {
    uint32_t scrid;     // screen ID
    uint32_t flg;       // flag-bits
    uint32_t font;      // font ID
    uint32_t func;      // raster-operation
    uint32_t mask;      // plane-mask
    uint32_t fgpix;     // foreground pixel value
    uint32_t bgpix;     // background pixel value
};

#endif /* __ZERO_DECK_H__ */

