#ifndef __KERN_EV_H__
#define __KERN_EV_H__

#include <stdint.h>
#include <wchar.h>
#include <zero/cdecl.h>

/* event header structure */

/* user input events */

/* keyboard events */
#define EVKEYDOWN       0x01
#define EVKEYUP         0x02
/* pointer events */
#define EVBUTTONDOWN    0x03
#define EVBUTTONUP      0x04
#define EVPNTMOTION     0x05
/* IPC events */
#define EVCMD           0x06
#define EVMSG           0x07
#define EVDATA          0x08
/* filesystem events */
#define EVFSCREAT       0x09
#define EVFSUNLINK      0x0a
#define EVFSMKDIR       0x0b
#define EVFSRMDIR       0x0c

/* keyboard events */

/*
 * Unicode specifies 0x10ffff as maximum value, leaving us with 11 high bits to
 * be used as flags if need be
 */

/* mask-bits for modifier keys */
#define EVKBDSHIFT       0x00000001     // Shift
#define EVKBDCAPSLK      0x00000002     // Caps Lock
#define EVKBDCTRL        0x00000004     // Ctrl
#define EVKBDMETA        0x00000008     // Meta
#define EVKBDCOMPOSE     0x00000010     // Compose
#define EVKBDALT         0x00000020     // Alt
#define EVKBDALTGR       0x00000040     // AltGr
#define EVKBDSCRLOCK     0x00000080     // Scroll Lock
#define EVNUMLOCK        0x00000100     // Num Lock
#define EVKBDNFLGBIT     9
#define kbducval(ev)     ((ev)->sym)    // extract Unicode value
#define kbdbutton(ev, b) ((ev)->state & (1L << ((b) + EVKBDNFLGBIT)))
#define kbdmod(ev, mod)  ((ev)->state & (mod))
struct evkbd {
    int32_t sym;                        // Unicode key symbol + flags
    int32_t state;                      // button state mask if present
} PACK();

/* pointer such as mouse device events */

#define pntbutton(ev, b) (ev->state & (1 << (b)))
/* pointer device, e.g. mouse event */
struct evpnt {
    uint32_t button;                    // button ID
    uint32_t state;                     // state bits for buttons; 1 -> down
    uint32_t x;                         // screen X coordinate
    uint32_t y;                         // screen Y coordinate
    uint32_t z;                         // screen Z coordinate
} PACK();

/* IPC events */

/* command packet */
struct evcmd {
    uint32_t cmd;                       // RPC command
    uint32_t src;                       // src object
    uint32_t dest;                      // destination object
} PACK();

/* message packet */
struct evmsg {
    uint32_t nbyte;                     // number of octets
    uint8_t  data[EMPTY];               // data (place-holder)
} PACK();

/* header + data */
struct evdata {
    uint32_t fmt;                       // data format
    uint32_t itemsz;                    // data-word size
    uint32_t nitem;                     // number of items to follow
    uint32_t obj;                       // data object ID
};

/* file system events */

struct evfs {
    uint32_t dev;                       // device ID
    uint64_t node;                      // node (file, directory) ID
    uint8_t  data[EMPTY];               // optional event data
} PACK();

struct zevent {
    uint32_t type;
    union {
        struct evkbd  kbd;
        struct evpnt  pnt;
        struct evcmd  cmd;
        struct evmsg  msg;
        struct evdata data;
        struct evfs   fs;
    };
} PACK();

/* API */
/* event-queue is mapped to both kernel and user space to avoid copying data */
/* register to listen to ev with flg parameters on event queue at qadr */
void        evreg(long ev, long flg, void *qadr);
/* check queue for event type ev; parameters in flg */
/* check current queue; don't flush server connection */
#define EVNOFLUSH       0x01
/* remove event from queue */
#define EVREMOVE        0x02
long        evchk(struct evp *ev, long mask, void *qadr);
/* read next event from queue */
/* flg bits */
/* leave copy of fetched event in the queue */
#define EVNOREMOVE      0x01
/* do not flush server connection before get */
#define EVNOFLUSH       0x02
void        evget(struct ev *evp, long flg, void *qadr);

#endif /* __KERN_EV_H__ */

