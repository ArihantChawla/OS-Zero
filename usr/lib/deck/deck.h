#ifndef __DECK_DECK_H__
#define __DECK_DECK_H__

#include <deck/conf.h>
#include <stdint.h>
#include <endian.h>
#include <sys/socket.h>
//#include <zero/ev.h>

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
#define DECK_GFX_LFB       0

/* structure for functionality similar to X and such desktop/screen servers */
struct deck {
    deckuword_t              screen; // screen ID
    deckuword_t              endian; // desktop-machine word endianess
    deckuword_t              conntype; // connection type
    deckuword_t              gfxtype; // graphics interface such as DECK_GFX_LFB
    struct sockaddr_storage  sockaddr; // remote connection address
    void                    *gfx; // graphics interface
};

#endif /* __DECK_DECK_H__ */

