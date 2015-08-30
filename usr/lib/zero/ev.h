#ifndef __ZERO_EV_H__
#define __ZERO_EV_H__

#include <stddef.h>
#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <deck/deck.h>
#include <gfx/rgb.h>

#define EVWORDSIZE   32
#define EVNODEIDSIZE 64

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
/* flag-bits in code-member */
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
#define EVNBUTTON         (EVWORDSIZE - EVKBNMODBIT)
#define kbdevhasstate(ev) ((ev)->code & EVKBDSTATE)
#define kbdupevent(ev)    ((ev)->code & EVKBDRELEASE)
#define kbddownevent(ev)  (!kbdupevent(ev))
#define kbdbutton(ev, b)  ((ev)->state & (1L << (b)))
#define kbdmod(ev, mod)   ((ev)->state & (mod))
/* keyboard event size in octets */
#define kbdevsize(ev)     (((ev)->code & EVKBDSTATE) ? 8 : 4)
struct evkbd {
    uint64_t code;      // keyboard scan-code or something similar
    /* state may not be present in protocol packets */
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
#define evmsgsize(ev) (ev->nbyte + offsetof(struct evmsg, data)

/* command packet */
/*
 * message packet
 * --------------
 * - reply is 0 on success, 32-bit error ID on failure
 * - can be done asynchronously with no reply checks
 */
/* flg-field bits */
#define EVCMDSYNC 0x01U
struct evcmd {
    uint32_t cmd;                       // RPC command
    uint32_t src;                       // source object ID
    uint32_t dest;                      // destination object ID
    uint32_t flg;                       // EVCMDSYNC, ...
} PACK();

/*
 * data transfer
 * -------------
 * - reply will be 32-bit object ID or 0 on failure
 */
/*
 * FIXME
 * -----
 * - endianess should [probably] be determined at connection time
 *   - use server/native endianess
 */
/* flg-field bits */
#define EVDATAMESSAGE   0x00000001U     // error event if bit not set
//#define EVDATABIGENDIAN 0x80000000U     // big-endian vs. little-endian words
/* fmt-field formats */
#define EVDATABINARY    0x00000000U
#define EVDATAASCII     0x00000001U
#define EVDATAISO88591  0x00000002U
#define EVDATAISO88592  0x00000003U
#define EVDATAISO88593  0x00000004U
#define EVDATAISO88594  0x00000005U
#define EVDATAISO88595  0x00000006U
#define EVDATAISO88596  0x00000007U
#define EVDATAISO88597  0x00000008U
#define EVDATAISO88598  0x00000009U
#define EVDATAISO88599  0x0000000aU
#define EVDATAISO885910 0x0000000bU
#define EVDATAISO885911 0x0000000cU
#define EVDATAISO885912 0x0000000dU
#define EVDATAISO885913 0x0000000eU
#define EVDATAISO885914 0x0000000fU
#define EVDATAISO885915 0x00000010U
#define EVDATAISO885916 0x00000011U
#define EVDATAUTF8      0x00000012U
#define EVDATAUTF16     0x00000013U
#define EVDATAUCS16     0x00000014U
#define EVDATAUCS32     0x00000015U
struct evdata {
    uint32_t flg;                       // flag-bits
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
    uint32_t type;                      // event type such as KEYUP, FSCREAT
    uint32_t tm;                        // timestamp
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

#if (!__KERNEL__)

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
#define EVNOFLUSH    0x01               // check queue; do not flush connection
#define EVNOREMOVE   0x02               // do not remove event from queue
/* flg-argument bits for evsync() */
#define EVSYNC       0x00000001L        // asynchronous if not set
#define EVTOSSINPUT  0x00000002L        // discard pending user input

#define evpeek(deck, ev, flg) evget((deck), (ev), (flg) | EVNOREMOVE)
void    evget(struct deck *deck, struct ev *ev, long flg);
long    evput(struct deck *deck, struct ev *ev, long flg);
void    evsync(struct deck *deck, long flg);

#endif /* !__KERNEL__ */

/*
 * NOTES
 * -----
 * - response event ID of 0 is protocol messages, event ID 1 for protocol errors
 */

/* events internal for event and response management */
#define EVPROTOMSG        0x00
#define EVERRORMSG        0x01
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
#define EVKEYDOWN         0x01  // keyboard press event
#define EVKEYUP           0x02  // keyboard release event
/* pointer events */
#define EVBUTTONDOWN      0x03  // mouse/pointer button press event
#define EVBUTTONUP        0x04  // mouse/pointer button release event
#define EVPNTMOTION       0x05  // pointer motion reported
/* IPC events */
#define EVMSG             0x06  // custom protocol messages
#define EVCMD             0x07  // RPC commands
#define EVDATA            0x08  // data transfer
/* filesystem events */
#define EVFSMOUNT         0x09  // filesystem mount event
#define EVFSUNMOUNT       0x0a  // filesystem unmount event
#define EVFSCREAT         0x0b  // file creation event
#define EVFSUNLINK        0x0c  // file unlink event
#define EVFSMKDIR         0x0d  // add directory
#define EVFSRMDIR         0x0e  // remove directory
#define NUSREV            0x0f  // # of user-space event types

/* queue events */
#define EVQUEUE           0x01
#define EVDEQUEUE         0x02

#endif /* __ZERO_EV_H__ */

 
