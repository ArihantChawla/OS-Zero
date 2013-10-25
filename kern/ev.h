#ifndef __KERN_EV_H__
#define __KERN_EV_H__

#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>

/* event header structure */

/* user input events */

/* masks for choosing events */
#define EVKEYDOWNMASK    (1UL << EVKEYDOWN)
#define EVKEYUPMASK      (1UL << EVKEYUP)
#define EVBUTTONDOWNMASK (1UL << EVBUTTONDOWN)
#define EVBUTTONUPMASK   (1UL << EVBUTTONUP)
#define EVPNTMOTIONMASK  (1UL << EVPNTMOTION)
#define EVIPCMASK        ((1UL << EVCMD) | (1UL << EVMSG) | (1UL << EVDATA))
#define EVFSCREATMASK    (1UL << EVFSCREAT)
#define EVFUNLINKMASK    (1UL << EVFSUNLINK)
#define EVFSMKDIRMASK    (1UL << EVFSMKDIR)
#define EVFSRMDIRMASK    (1UL << EVFSRMDIR)

/* keyboard events */
#define EVKEYDOWN        0x01   // keyboard key down event
#define EVKEYUP          0x02   // keyboard key up event
/* pointer events */
#define EVBUTTONDOWN     0x03   // mouse/pointer button down event
#define EVBUTTONUP       0x04   // mouse/pointer button up event
#define EVPNTMOTION      0x05
/* IPC events */
#define EVMSG            0x06   // message events such as errors
#define EVCMD            0x07   // RPC commands
#define EVDATA           0x08   // data transfer
/* filesystem events */
#define EVFSCREAT        0x07   // file creation event
#define EVFSUNLINK       0x08   // file unlink event
#define EVFSMKDIR        0x09   // add directory
#define EVFSRMDIR        0x0a   // remove directory

/* keyboard events */

/*
 * Unicode specifies 0x10ffff as maximum value, leaving us with 11 high bits to
 * be used as flags if need be
 */

/* mask-bits for modifier keys */
#define EVKBDSTATE       0x80000000     // 1 if event has state member set
#define EVKBDSHIFT       0x00000001     // Shift
#define EVKBDCAPSLK      0x00000002     // Caps Lock
#define EVKBDCTRL        0x00000004     // Ctrl
#define EVKBDMETA        0x00000008     // Meta
#define EVKBDCOMPOSE     0x00000010     // Compose
#define EVKBDALT         0x00000020     // Alt
#define EVKBDALTGR       0x00000040     // AltGr
#define EVKBDSCRLOCK     0x00000080     // Scroll Lock
#define EVNUMLOCK        0x00000100     // Num Lock
#define EVKBDNMODBIT     9
#define EVNBUTTON        (32 - EVKBNMODBIT)
#define kbdevlen64(ev)   ((ev)->sym & EVKBDSTATE)
#define kbducval(ev)     ((ev)->sym)    // extract Unicode value
#define kbdbutton(ev, b) ((ev)->state & (1L << ((b) + EVKBDNMODBIT)))
#define kbdmod(ev, mod)  ((ev)->state & (mod))
struct evkbd {
    int32_t sym;                        // Unicode key symbol + flags
    int32_t state;                      // button and modifier state if present
} PACK();

/* ring-buffer event queue for 8-bit character keyboard events */
#define EVKBDQNCHAR (PAGESIZE - 8 * LONGSIZE)
struct evkbdqchar {
    volatile long lk;                   // queue lock mutex
    int32_t       out;                  // index of next character to write
    int32_t       in;                   // index of next character to read
    /* PADDING */
#if (LONGSIZE == 4)
    int32_t       pad[5];
#elif (LONGSIZE == 8)
    int32_t       pad[6];
#endif
    /* character data */
    unsigned char ctab[EVKBDQNCHAR];
} PACK() ALIGNED(PAGESIZE);

/* pointer such as mouse device events */

#define pntbutton(ev, b) (ev->state & (1 << (b)))
/* pointer device, e.g. mouse event */
struct evpnt {
    uint32_t button;                    // button ID
    uint32_t state;                     // state bits for buttons; 1 -> down
    int32_t x;                          // screen X coordinate
    int32_t y;                          // screen Y coordinate
    int32_t z;                          // screen Z coordinate
} PACK();

/* IPC events */

/* message packet */
struct evmsg {
    uint32_t nbyte;                     // number of octets
    uint8_t  data[EMPTY];               // data (place-holder)
} PACK();

/* command packet */
struct evcmd {
    uint32_t cmd;                       // RPC command
    uint32_t src;                       // src object
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
    uint32_t dev;                       // device ID
    uint32_t node;                      // node (file, directory) ID
    uint32_t datalen;                   // length of data in bytes
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

unsigned char evdeqkbdchar(struct evkbdqchar *queue);
void          evqkbdchar(struct evkbdqchar *queue, unsigned char ch);

#endif /* __KERN_EV_H__ */

