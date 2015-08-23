#ifndef __DECK_DECK_H__
#define __DECK_DECK_H__

#include <deck/conf.h>
#include <stdint.h>
#include <endian.h>
#include <zero/ev.h>
#if (DECK_GFX_LFB)
#include <deck/lfb.h>
#endif

/* endian-field values; byte-order */
#define DECK_LITTLE_ENDIAN 0
#define DECK_BIG_ENDIAN    1
/* protocol-field values */
#define DECK_CONN_SHM      0
#define DECK_CONN_IP4      1
#define DECK_CONN_IP6      2
/* gfxtype-field values */

/* structure for functionality similar to X and such desktop/screen servers */
struct deck {
    uint32_t  screen;   // screen ID
    long      endian;   // desktop-machine word endianess
    long      conntype; // connection type
    long      gfxtype;  // graphics interface such as DECK_GFX_LFB
    void     *gfx;      // graphics interface
};

#endif /* __DECK_DECK_H__ */

