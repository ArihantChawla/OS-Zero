#ifndef __ZERO_BITS_EV_H__
#define __ZERO_BITS_EV_H__

#include <stdint.h>
#include <zero/cdefs.h>

#if (EVTIMESIZE == 64)
typedef uint64_t evtime_t;
#elif (EVTIMESIZE == 32)
typedef uint32_t evtime_t;
#endif

typedef int32_t  evkeycode_t;

#if (EVWORDSIZE == 32)
typedef int32_t  evword_t;
typedef uint32_t evuword_t;
#elif (EVWORDSIZE == 64)
typedef int64_t  evword_t;
typedef uint64_t evuword_t;
#endif

#if (EVNODEIDSIZE == 64)
typedef int64_t  evnodeid_t;
#endif

/* event names */

/*
 * NOTES
 * -----
 * - response event ID of 0 is protocol messages, event ID 1 for protocol errors
 */

/* flag bits for type-field */
#define EVSYS             0x80000000
/* events internal for event and response management */
#define EVPROTOMSG        0
#define EVERRORMSG        1
/* system events */
/* EVSHUTDOWN is delivered no matter what events have been selected */
#define EVSHUTDOWN        2     // system is being shut down or rebooted
/* hardware/driver events */
#define EVLOAD            3     // kernel module loaded
#define EVUNLOAD          4     // kernel module unloaded
#define EVPLUG            5     // new device plugged
#define EVUNPLUG          6     // device unplugged
#define NSYSEV            7     // # of system-space event types

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
#define EVKEYDOWN         1     // keyboard press event
#define EVKEYUP           2     // keyboard release event
/* pointer events */
#define EVBUTTONDOWN      3    // mouse/pointer button press event
#define EVBUTTONUP        4    // mouse/pointer button release event
#define EVPNTMOTION       5    // pointer motion reported
/* IPC events */
#define EVMSG             6  // custom protocol messages
#define EVCMD             7  // RPC commands
#define EVDATA            8  // data transfer
/* filesystem events */
#define EVFSMOUNT         9  // filesystem mount event
#define EVFSUNMOUNT       10  // filesystem unmount event
#define EVFSCREAT         11  // file creation event
#define EVFSUNLINK        12  // file unlink event
#define EVFSMKDIR         13  // add directory
#define EVFSRMDIR         14  // remove directory
#define NUSREV            15  // # of user-space event types

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
 * be used as flags if need be; let's start using them from the highest ones to
 * be safer, though
 */

/* flag-bits in code-member */
#define EVKBDRELEASE      0x80000000    // keyboard release event
#define EVKBDSTATE        0x40000000    // code followed by state word
#define EVKBDEXTSTATE     0x20000000    // state word followed by extra one
#define _EVKBDRESBIT      0x10000000
#define EVKBDSYMMASK      0x0fffffff
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
/* button masks/IDs */
#define BUTTON1               (1 << 0)
#define BUTTON2               (1 << 1)
#define BUTTON3               (1 << 2)
#define BUTTON4               (1 << 3)
#define BUTTON5               (1 << 4)
#define BUTTON6               (1 << 5)
#define BUTTON7               (1 << 6)
#define BUTTON8               (1 << 7)
#define BUTTON9               (1 << 8)
#define BUTTON10              (1 << 9)
#define BUTTON11              (1 << 10)
#define BUTTON12              (1 << 11)
#define BUTTON13              (1 << 12)
#define BUTTON14              (1 << 13)
#define BUTTON15              (1 << 14)
#define BUTTON16              (1 << 15)
#define EVNBUTTON             16
#define _kbdevhasstate(ev)    ((ev)->code & EVKBDSTATE)
#define _kbdevhasextstate(ev) ((ev)->code & EVKBDEXTSTATE)
#define _kbdupevent(ev)       ((ev)->code & EVKBDRELEASE)
#define _kbddownevent(ev)     (!kbdupevent(ev))
#define _kbdbutton(ev, b)     ((ev)->state & (1L << (b)))
#define _kbdmod(ev, mod)      ((ev)->state & (mod))
#define _kbdextbutton(ev, b)  ((ev)->extstate & (1L << ((b) - EVNBUTTON)))
/* keyboard event size in octets */
#define _kbdevsize(ev)        (!((ev)->code & EVKBDSTATE) ? 8 : 12)
struct evkbd {
    evkeycode_t code;           // Unicode-value with a few high flag bits
    /* state may not be present in protocol packets */
    evuword_t   state;          // modifier flags in high bits, buttons in low
    /* extended state for button IDs 17..48 when present */
    evuword_t   extstate;       // button-state bits
};

/* pointer such as mouse device events */

#define _pntbutton(ev, b) ((ev)->state & (1 << (b)))
/* pointer device, e.g. mouse event */
struct evpnt {
    evuword_t button;           // button ID
    evuword_t state;            // state bits for buttons; 1 -> down
    evword_t  x;                // screen X coordinate
    evword_t  y;                // screen Y coordinate
    evword_t  z;                // screen Z coordinate
};

/* IPC events */

/*
 * data transfers
 * --------------
 * - reply will be 32-bit object ID or 0 on failure
 */
/*
 * FIXME
 * -----
 * - endianess should [probably] be determined at connection time
 *   - use server/native endianess
 */

/*
 * message packet
 * --------------
 * - reply is 32-bit object ID or 0 on failure
 */
struct evmsg {
    evuword_t nbyte;            // number of octets
    evuword_t mq;               // message queue ID
    uint8_t   data[EMPTY];      // message data (evdata structure + data)
};
#define _evmsgsize(ev) (ev->nbyte + offsetof(struct evmsg, data)

struct evdata {
    evuword_t flg;              // flag-bits
    evuword_t fmt;              // data format
    evuword_t wsize;            // data-word size in bytes
    evuword_t nitem;            // number of items to follow
    uint8_t   msg[EMPTY];       // message data
};

/* command packet */
/*
 * message packet
 * --------------
 * - reply is 0 on success, 32-bit error ID on failure
 * - can be done asynchronously with no reply checks
 */
/* flg-field bits */
#define EVCMDSYNC 0x00000001U
struct evcmd {
    evuword_t cmd;              // RPC command
    evuword_t src;              // source object ID
    evuword_t dest;             // destination object ID
    evuword_t flg;              // EVCMDSYNC, ...
    uint8_t   msg[EMPTY];       // command data
};

/* message events */

/* flg-field bits */
#define EVDATAMESSAGE   0x00000001U     // error event if bit not set
/* fmt-field formats */
#define EVDATARAW       0x00000000U     // binary
#define EVDATAASCII     0x00000001U     // ASCII-text
#define EVDATAISO88591  0x00000002U     // 8-bit ISO-8859 text formats
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
#define EVDATAUTF8      0x00000012U     // Unicode in UTF8-encoding
#define EVDATAUTF16     0x00000013U     // Unicode in UTF16-encoding
#define EVDATAUCS16     0x00000014U     // 16-bit UCS-2
#define EVDATAUCS32     0x00000015U     // 32-bit UCS-4

/* file system events; EVFSCREAT, EVFSUNLINK, EVFSMKDIR, EVFSRMDIR */
struct evfs {
    evnodeid_t node;            // node (file, directory) ID
    evuword_t  dev;             // device ID
    evuword_t  flg;             // event flags
};

#endif /* __ZERO_BITS_EV_H__ */

