#ifndef __KERN_EV_H__
#define __KERN_EV_H__

#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/deck.h>
#include <gfx/rgb.h>

/* keyboard events */

/*
 * - the kernel passes raw keyboard scan-codes to userland as 64-bit integers
 * - the longest scan-codes I'm aware on PC keyboards are 6-byte
 * - these scan-codes are read from the keyboard byte by byte
 * - we leave the translation to userland according to locale and such settings
 * - userland processing of scan-codes keeps our interrupt handler minimal
 */

/*
 * TODO: move this comment elsewhere...
 * Unicode specifies 0x10ffff as maximum value, leaving us with 11 high bits to
 * be used as flags if need be
 */
/* flag-bits in scan member */
#define EVKBDSTATE        0x80000000
#define EVKBDRELEASE      0x40000000
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
#define EVKBDNMODBIT      16
#define EVNBUTTON         (32 - EVKBNMODBIT)
#define kbdevhasstate(ev) ((ev)->code & EVKBDSTATE)
#define kbdupevent(ev)    ((ev)->code & EVKBDRELEASE)
#define kbdbutton(ev, b)  ((ev)->state & (1L << (b)))
#define kbdmod(ev, mod)   ((ev)->state & (mod))
/* keyboard event size in octets */
#define kbdevsize(ev)     (((ev)->code & EVKBDSTATE) ? 8 : 4)
struct evkbd {
    uint32_t code;      // keyboard scan-code or something similar
    uint32_t state;     // modifier flags in high bits, buttons in low
} PACK();

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

/*
 * message packet
 * --------------
 * - reply is 32-bit object ID or 0 on failure
 */
struct evmsg {
    uint32_t nbyte;                     // number of octets
    uint32_t mq;                        // message queue ID
    uint8_t  data[EMPTY];               // message data
} PACK();

/* command packet */
/*
 * message packet
 * --------------
 * - reply is 0 on success, 32-bit error ID on failure
 * - can be done asynchronously with no reply checks
 */
#define EVCMDASYNC 0x01U
struct evcmd {
    uint32_t cmd;                       // RPC command
    uint32_t src;                       // source object
    uint32_t dest;                      // destination object
    uint32_t flg;                       // ASYNC, ...
} PACK();

/*
 * data transfer
 * -------------
 * - reply will be 32-bit object ID or 0 on failure
 */
/* flg-field bits */
#define EVDATA_BIGENDIAN  0x00000001U   // big-endian vs. little-endian words
#define EVDATA_MESSAGE    0x00000002U   // otherwise error
/* fmt-field formats */
#define EVDATA_BINARY     0x00000000U
#define EVDATA_ASCII      0x00000001U
#define EVDATA_ISO8859_1  0x00000002U
#define EVDATA_ISO8859_2  0x00000003U
#define EVDATA_ISO8859_3  0x00000004U
#define EVDATA_ISO8859_4  0x00000005U
#define EVDATA_ISO8859_5  0x00000006U
#define EVDATA_ISO8859_6  0x00000007U
#define EVDATA_ISO8859_7  0x00000008U
#define EVDATA_ISO8859_8  0x00000009U
#define EVDATA_ISO8859_9  0x0000000aU
#define EVDATA_ISO8859_10 0x0000000bU
#define EVDATA_ISO8859_11 0x0000000cU
#define EVDATA_ISO8859_12 0x0000000dU
#define EVDATA_ISO8859_13 0x0000000eU
#define EVDATA_ISO8859_14 0x0000000fU
#define EVDATA_ISO8859_15 0x00000010U
#define EVDATA_ISO8859_16 0x00000011U
#define EVDATA_UTF_8      0x00000012U
#define EVDATA_UTF_16     0x00000013U
#define EVDATA_UCS_16     0x00000014U
#define EVDATA_UCS_32     0x00000015U
struct evdata {
    uint32_t flg;                       // data/flags such as BIGENDIAN
    uint32_t fmt;                       // data format
    uint32_t wsize;                     // data-word size in bytes
    uint32_t nitem;                     // number of items to follow
    uint8_t  data[EMPTY];               // data message
} PACK();

/* file system events; EVFSCREAT, EVFSUNLINK, EVFSMKDIR, EVFSRMDIR */
struct evfs {
    uint64_t node;                      // node (file, directory) ID
    uint32_t dev;                       // device ID
    uint32_t flg;                       // event flags
} PACK();

#define evgettype(ev)    ((ev)->hdr.type)
#define evsettype(ev, t) ((ev)->hdr.type = (t))
#define evgettime(ev)    ((ev)->hdr.tm)
#define evsettime(ev, t) ((ev)->hdr.tm = (t))
struct evhdr {
    uint32_t tm;                        // timestamp
    uint32_t type;                      // event type such as KEYUP, FSCREAT
};

/* event structure */
struct ev {
    struct evhdr hdr;
    union {
        /* actual event message */
        struct evkbd  kbd;
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
/* read next event from queue */
long   evpeek(struct ev *ev, long mask);
/*
 * NOTE: the flg-bit values below are guaranteed not to overlap with event
 * masks
 */
/*
'* flg-value bits for evget() and evput()
 * --------------------------------------
 * - EVGET, EVPUT: flush queue unless flg has the EVNOFLUSH-bit set
 * - EVGET:        remove from queue unless flg has the EVNOREMOVE-bit set
 */
#define EVNOFLUSH         0x01  // check queue; do not flush connection
#define EVNOREMOVE        0x02  // do not remove event from queue
/* flg-argument bits for evsync() */
#define EV_SYNC           0x00000001L   // otherwise asynchronous
#define EV_TOSS_USERINPUT 0x00000002L   // discard pending user input

#define evpeek(ev, flg)   evget((ev), ((flg) | EVNOREMOVE))
void    evget(struct ev *ev, long flg);
long    evput(struct ev *ev, long flg);
void    evsync(struct deck *deck, long flg);

/*
 * NOTES
 * -----
 * - event ID of 0 is protocol messages, event ID 1 for protocol errors
 */

/* events internal for event and reply management */
#define EVPROTOMSG        0x00
#define EVERRMSG          0x01
/* system events */
/* EVSHUTDOWN is delivered no matter what events have been selected */
#define EVSHUTDOWN        0x02  // system is being shut down or rebooted
/* hardware/driver events */
#define EVLOAD            0x03  // kernel module loaded
#define EVUNLOAD          0x04  // kernel module unloaded
#define EVPLUG            0x05  // new device plugged
#define EVUNPLUG          0x06  // device unplugged
#define NSYSEV            0x07  // # of system-space event types

/* userland events */

/* masks for choosing events to listen */

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
/* filesystem events */
#define EVFSMASK          ((EVFSMOUNTMASK                               \
                            | EVFSUNMOUNTMASK                           \
                            | EVFSCREATMASK                             \
                            | EVFSUNLINKMASK                            \
                            | EVFSMKDIRMASK                             \
                            | EVFSRMDIRMASK))
#define EVFSMOUNTMASK     (1UL << EVFSMOUNT)
#define EVFSUNMOUNTMASK   (1UL << EVFSUNMOUNT)
#define EVFSCREATMASK     (1UL << EVFSCREAT)
#define EVFSUNLINKMASK    (1UL << EVFSUNLINK)
#define EVFSMKDIRMASK     (1UL << EVFSMKDIR)
#define EVFSRMDIRMASK     (1UL << EVFSRMDIR)

/* event IDs */

/* keyboard events */
#define EVKEYDOWN         0x01  // keyboard key down event
#define EVKEYUP           0x02  // keyboard key up event
/* pointer events */
#define EVBUTTONDOWN      0x03  // mouse/pointer button down event
#define EVBUTTONUP        0x04  // mouse/pointer button up event
#define EVPNTMOTION       0x05  // pointer motion reported
/* IPC events */
#define EVMSG             0x06  // custom protocol messages
#define EVCMD             0x07  // RPC commands
#define EVDATA            0x08  // data transfer
/* filesystem events */
#define EVFSMOUNT         0x09
#define EVFSUNMOUNT       0x0a
#define EVFSCREAT         0x0b  // file creation event
#define EVFSUNLINK        0x0c  // file unlink event
#define EVFSMKDIR         0x0d  // add directory
#define EVFSRMDIR         0x0e  // remove directory
#define NUSREV            0x0f  // # of user-space event types

/* queue events */
#define EVQUEUE           0x01
#define EVDEQUEUE         0x02

#endif /* __KERN_EV_H__ */

 
