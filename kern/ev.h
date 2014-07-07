#ifndef __KERN_EV_H__
#define __KERN_EV_H__

#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>

/* masks for choosing events */
/* user input events */
#define EVKEYMASK         (EVKEYDOWNMASK | EVKEYUPMASK)
#define EVKEYDOWNMASK     (1UL << EVKEYDOWN)
#define EVKEYUPMASK       (1UL << EVKEYUP)
#define EVBUTTONMASK      (EVBUTTONDOWNMASK | EVBUTTONUPMASK)
#define EVBUTTONDOWNMASK  (1UL << EVBUTTONDOWN)
#define EVBUTTONUPMASK    (1UL << EVBUTTONUP)
#define EVPNTMOTIONMASK   (1UL << EVPNTMOTION)
/* IPC events */
#define EVIPCMASK         ((1UL << EVCMD) | (1UL << EVMSG) | (1UL << EVDATA))
#define EVFSMASK          ((EVFSCREATMASK                                \
                            | EVFSUNLINKMASK                            \
                            | EVFSMKDIRMASK                             \
                            | EVFSRMDIRMASK))
/* filesystem events */
#define EVFSCREATMASK     (1UL << EVFSCREAT)
#define EVFSUNLINKMASK    (1UL << EVFSUNLINK)
#define EVFSMKDIRMASK     (1UL << EVFSMKDIR)
#define EVFSRMDIRMASK     (1UL << EVFSRMDIR)

/* keyboard events */
#define EVKEYDOWN         0x01   // keyboard key down event
#define EVKEYUP           0x02   // keyboard key up event
/* pointer events */
#define EVBUTTONDOWN      0x03   // mouse/pointer button down event
#define EVBUTTONUP        0x04   // mouse/pointer button up event
#define EVPNTMOTION       0x05
/* IPC events */
#define EVMSG             0x06   // message events such as errors
#define EVCMD             0x07   // RPC commands
#define EVDATA            0x08   // data transfer
/* filesystem events */
#define EVFSCREAT         0x07   // file creation event
#define EVFSUNLINK        0x08   // file unlink event
#define EVFSMKDIR         0x09   // add directory
#define EVFSRMDIR         0x0a   // remove directory

/* queue events */
#define EVQUEUE           0x01
#define EVDEQUEUE         0x02

/* keyboard events */

/*
 * Unicode specifies 0x10ffff as maximum value, leaving us with 11 high bits to
 * be used as flags if need be
 */
/* flag-bits in scan member */
#define EVKBDSTATE        UINT64_C(0x8000000000000000)
#define EVKBDRELEASE      UINT64_C(0x4000000000000000)
/* state-bits for modifier keys */
#define EVKBDSHIFT        0x80000000
#define EVKBDCAPSLK       0x40000000
#define EVKBDCTRL         0x20000000
#define EVKBDMETA         0x10000000
#define EVKBDCOMPOSE      0x08000000
#define EVKBDALT          0x04000000
#define EVKBDALTGR        0x02000000
#define EVKBDSCRLOCK      0x01000000
#define EVNUMLOCK         0x00800000
#define EVKBDNMODBIT      9
#define EVNBUTTON         (32 - EVKBNMODBIT - EVKBDNFLGBIT)
#define kbdevhasstate(ev) ((ev)->sym & EVKBDSTATE)
#define kbdupevent(ev)    ((ev)->scan & EVKBDRELEASE)
#define kbdbutton(ev, b)  ((ev)->state & (1L << (b)))
#define kbdmod(ev, mod)   ((ev)->state & (mod))
/* keyboard event size in octets */
#define kbdevsize(ev)     (((ev)->sym & EVKBDSTATE) ? 12 : 8)
struct kbdev {
    uint64_t scan;
    uint32_t state;
    uint32_t pad;
} PACK();

#if 0
/* TODO: this stuff belongs into the desktop environment; rework kernel
 * keyboard events to be 64-bit scan codes
 */

/* keyboard event state field existence */
#define EVKBDSTATE        0x80000000    // 1 if event has state member
#define EVKBDRELEASE      0x40000000    // 1 if keyrelease event
#define EVKBDNFLGBIT      2
struct evkbd {
    int32_t sym;                        // Unicode key symbol + flags
    int32_t state;                      // button and modifier state if present
} PACK();

#endif /* 0 */

/* pointer such as mouse device events */

#define pntbutton(ev, b) (ev->state & (1 << (b)))
/* pointer device, e.g. mouse event */
struct evpnt {
    uint32_t button;                    // button ID
    uint32_t state;                     // state bits for buttons; 1 -> down
    int32_t  x;                         // screen X coordinate
    int32_t  y;                         // screen Y coordinate
    int32_t  z;                         // screen Z coordinate
} PACK();

/* IPC events */

/* message packet */
struct evmsg {
    uint32_t nbyte;                     // number of octets
    uint32_t mqid;                      // message queue ID
} PACK();

/* command packet */
struct evcmd {
    uint32_t cmd;                       // RPC command
    uint32_t src;                       // source object
    uint32_t dest;                      // destination object
} PACK();

/* header + data */
struct evdata {
    uint32_t fmt;                       // data format
    uint32_t itemsz;                    // data-word size
    uint32_t nitem;                     // number of items to follow
    uint32_t obj;                       // data object ID
} PACK();

/* file system events */

struct evfs {
    uint64_t node;                      // node (file, directory) ID
    uint32_t dev;                       // device ID
    uint32_t datalen;                   // length of data in bytes
    uint8_t  data[EMPTY];               // optional event data
} PACK();

struct zevent {
    uint32_t type;
    union {
//        struct evkbd  kbd;
        uint64_t      key;
        struct evpnt  pnt;
        struct evcmd  cmd;
        struct evmsg  msg;
        struct evdata data;
        struct evfs   fs;
    } msg;
} PACK();

/* API */
/* TODO: implement ring-buffers for event queues */
/* - wired to physical memory permanently */
/* - event queue is mapped to both kernel and user space to avoid data copies */
/* - register to listen to ev with flg parameters */
/*   - evreg returns pointer to dual-mapped event queue (page) */
void * evreg(long mask, long flg);
long   evpeek(struct zevent *ev, long mask);
/* check current queue; don't flush the connection */
#define EVNOFLUSH       0x01
/* do not remove event from queue */
#define EVNOREMOVE      0x02
/* read next event from queue */
/* flush queue unless flg has the EVNOFLUSH-bit set */
/* remove from queue unless flg has the NOREMOVE-bit set */
void   evget(struct zevent *ev, long flg);

#if 0
unsigned char evdeqkbdchar(struct evkbdqchar *queue);
void          evqkbdchar(struct evkbdqchar *queue, unsigned char ch);
#endif

#endif /* __KERN_EV_H__ */

