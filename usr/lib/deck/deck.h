#ifndef __DECK_DECK_H__
#define __DECK_DECK_H__

#include <deck/conf.h>
#include <deck/types.h>
#include <sys/socket.h>
#if (SUPPORT_POSIX_MQ)
#include <mqueue.h>
#endif
#if (SUPPORT_SYSV_MQ)
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#endif

/* endian-field values; byte-order */
#define DECK_LITTLE_ENDIAN 0
#define DECK_BIG_ENDIAN    1
/* protocol-field values */
#define DECK_CONN_SHM      0
#define DECK_CONN_MQ       1
#define DECK_CONN_IP4      2
#define DECK_CONN_IP6      3
/* gfxtype-field values */
#define DECK_GFX_LFB       0    // linear framebuffer

struct deckconnmq_posix {
    const char   *name;
    mqd_t         desc;
    unsigned int  prio;
};

struct deckconnmq_sysv {
    int qid;
};

struct deckconn {
    union {
        void                    *shm;   // shared memory region
        struct sockaddr_storage  sock;  // socket
        struct deckconnmq_posix  posix; // POSIX message queue
        struct deckconnmq_sysv   sysv;  // System V message queue
    } info;
    deckuword                    type;  // connection type
    deckuword                    endian; // remote byte-order
};

/* structure for functionality similar to X and such desktop/screen servers */
struct deck {
    struct deckconn  conn;      // local or remote connection attributes
    deckuword        screen;    // screen ID
    deckuword        endian;    // desktop-machine word endianess
    deckuword        gfxtype;   // graphics interface, e.g. DECK_GFX_LFB
    void            *gfxatr;    // graphics interface attributes
};

#endif /* __DECK_DECK_H__ */

