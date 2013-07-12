#ifndef __KERN_EV_H__
#define __KERN_EV_H__

#include <stdint.h>
#include <zero/cdecl.h>

/* user input events */

/* event types */
#define EVKEYDOWN     0x01
#define EVKEYUP       0x02
#define EVBUTTONDOWN  0x03
#define EVBUTTONUP    0x04
#define EVPNTMOTION   0x05
#define EVCMD         0x06
#define EVDATA        0x07

/* keyboard events */

/* the highest bit in sym indicates presence of a control key mask */
#define EVKBDCTRLBIT  0x80000000U
/* mask-bits for control keys */
#define EVKBDSHIFT    0x00000001U       // Shift
#define EVKBDCAPSLK   0x00000002U       // Caps Lock
#define EVKBDCTRL     0x00000004U       // Ctrl
#define EVKBDMETA     0x00000008U       // Meta
#define EVKBDCOMPOSE  0x00000010U       // Compose
#define EVKBDALT      0x00000020U       // Alt
#define EVKBDALTGR    0x00000040U       // AltGr
#define EVKBDSCRLOCK  0x00000080U       // Scroll lock
struct evkbd {
    uint32_t sym;                       // key symbol
    uint32_t state;                     // control key state mask if present
};

/* pointer such as mouse device events */

#define evbuttondown(ev, button) (ev->state & (1 << (button)))
/* pointer device, e.g. mouse event */
struct evpnt {
    uint32_t state;                     // state bits for buttons; 1 -> down
    uint32_t x;                         // screen X coordinate
    uint32_t y;                         // screen Y coordinate
    uint32_t z;                         // screen Z coordinate
} PACK();

struct uievent {
    uint32_t type;                      // event type
    union {                             // event data
        struct evkbd kbd;
        struct evpnt pnt;
    } ev;
} PACK();

/* IPC events */

/* command packet */
struct evcmd {
    uint64_t cmd;                       // RPC command
    uint64_t dest;                      // destination object
} PACK();

/* data packet */
struct evdata {
    uint32_t fmt;                       // data format
    uint32_t itemsz;                    // data-word size
    uint32_t nitem;                     // number of words
    uint8_t  data[EMPTY];               // data
} PACK();

struct ipcevent {
    uint32_t type;
    union {
        struct evcmd  cmd;
        struct evdata data;
    } ev;
} PACK();

/* file system events */

/* event types */
#define EVCREAT       0x01
#define EVUNLINK      0x02
#define EVMKDIR       0x03
#define EVRMDIR       0x04

struct fsevent {
    uint32_t type;                      // event type
    uint32_t dev;                       // device ID
    uint64_t node;                      // node (file, directory) ID
} PACK();

/* window events */

#define EVWINMK         0x01            // new window created
#define EVWINMAP        0x02            // window mapped
#define EVWINDESTROY    0x03            // window destroyd
#define EVWINUNMAP      0x04            // window unmapped
#define EVWINEXPOSE     0x05            // part of window exposed
#define EVWINVISIBILITY 0x06            // window visibility changed

/* API */
/* event-queue is mapped to both kernel and user space to avoid copying data */
/* register to listen to ev with flg parameters on event queue at qadr */
void        evreg(long ev, long flg, void *qadr);
/* check queue for event type ev; parameters in flg */
#define EVNOFLUSH       // check current queue; don't flush server connection
#define EVFETCH         // fetch/remove event from queue
struct ev * evchk(long ev, long flg, void *qadr);
/* read next event from queue */
#define EVNOREMOVE      // leave copy of fetched event in the queue
#define EVNOFLUSH       // do not flush server connection before get
void        evget(struct ev *evp, long flg, void *qadr);

#endif /* __KERN_EV_H__ */

