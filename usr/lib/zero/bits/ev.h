#ifndef __ZERO_BITS_EV_H__
#define __ZERO_BITS_EV_H__

/* libzero event interface */

#include <stdint.h>
#include <zero/cdefs.h>

#if (EV_TIME_BITS == 64)
typedef uint64_t evtime_t;
#elif (EV_TIME_BITS == 32)
typedef uint32_t evtime_t;
#endif

#define EV_KEYCODE_BITS 32 // number of bits for keys in event keycodes
#define EV_KEY_UP_BIT   (1L << 31)
#define EV_KEY_BITS     22 // space for up to 0x1fffff + EVKBDSTATEBIT
#define EV_KEY_NONE     0 // no key value provided
typedef int32_t         evkeycode_t; // protocol/transport keycode of 32 bits
typedef int32_t         evkeystate_t;
#define EV_KEY_INVAL    ((1U << EVKEYBITS) - 1) // invalid key/error return

/* library/user-level keycode type */
#if (EVKEYBITS <= 16)
typedef int16_t evkey_t;
#else
typedef int32_t evkey_t;
#endif

#if (EV_WORD_BITS == 32)
typedef int32_t  evword_t;
typedef uint32_t evuword_t;
#elif (EV_WORD_BITS == 64)
typedef int64_t  evword_t;
typedef uint64_t evuword_t;
#endif

typedef evword_t evobjid_t;

/* keyboard events */

/*
 * - the kernel passes raw keyboard scan-codes to userland as 64-bit integers
 * - the longest scan-codes I'm aware on PC keyboards are 6-byte
 * - these scan-codes are read from the keyboard byte by byte
 * - we leave the translation to userland according to locale and such settings
 * - userland processing of scan-codes keeps our interrupt handler minimal
 */

/* keycode special-bits */
#define EVKBDSTATEBIT     0x80000000 // modifier and button state present
/* modifier flags */
#define EV_MOD(x)         (1L << (32 - (x)))
#define EV_MOD_SHIFT      EV_MOD(EVMODSHIFT)
#define EV_MOD_CAPSLK     EV_MOD(EVMODCAPSLK)
#define EV_MOD_CTRL       EV_MOD(EVMODCTRL)
#define EV_MOD_META       EV_MOD(EVMODMETA)
#define EV_MOD_COMPOSE    EV_MOD(EVMODCOMPOSE)
#define EV_MOD_ALT        EV_MOD(EVMODALT)
#define EV_MOD_ALTGR      EV_MOD(EVMODALTGR)
#define EV_MOD_SCRLK      EV_MOD(EVMODSCRLK)
#define EV_MOD_NUMLK      EV_MOD(EVMODNUMLK)
/* button masks/IDs */
#define EV_BUTTON(i)      (1 << ((i) - 1)
#define EV_BUTTON_1       EV_BUTTON(1)
#define EV_BUTTON_2       EV_BUTTON(2)
#define EV_BUTTON_3       EV_BUTTON(3)
#define EV_BUTTON_4       EV_BUTTON(4)
#define EV_BUTTON_5       EV_BUTTON(5)
#define EV_BUTTON_6       EV_BUTTON(6)
#define EV_BUTTON_7       EV_BUTTON(7)
#define EV_BUTTON_8       EV_BUTTON(8)
#define EV_BUTTON_9       EV_BUTTON(9)
#define EV_BUTTON_10      EV_BUTTON(10)
#define EV_BUTTON_11      EV_BUTTON(11)
#define EV_BUTTON_12      EV_BUTTON(12)
#define EV_BUTTON_13      EV_BUTTON(13)
#define EV_BUTTON_14      EV_BUTTON(14)
#define EV_BUTTON_15      EV_BUTTON(15)
#define EV_BUTTON_16      EV_BUTTON(16)
#define KEY_STATE_BUTTONS 16
#define EVBUTTONMASK      ((1L << KEY_STATE_BUTTONS) - 1)
/* state-bits for modifier keys */
#define EVMODNONE         0
#define EVMODSHIFT        1
#define EVMODCAPSLK       2
#define EVMODCTRL         3
#define EVMODMETA         4
#define EVMODCOMPOSE      5
#define EVMODALT          6
#define EVMODALTGR        7
#define EVMODSCRLK        8
#define EVMODNUMLK        9
#define KEY_MOD_BITS      10
#define KEY_STATE_MODS    16
#define EVMODMASK         ((1L << KEY_STATE_BUTTONS) - 1)

/* keyboard event macros */
#define EVKEYMASK              0x001fffffL // Unicode range is 1..0x10fffff
#define evkeysym(key)          ((key) & EVKEYMASK)
#define evgetkeysym(ev)        (evkeysym((ev)->key))
#define evchkstate(key)        ((key) & EVKBDSTATEBIT) // have state?
#define evhasstate(ev)         (evkeystate((ev)->code)) // state present?
#define evgetbuttons(ev)       (kbdchkbuttons(EVBUTTONMASK)) // mask buttons
#define evbuttondown(id, b)    ((id) & EV_BUTTON(b)) // button pressed?
#define evhasbutton(ev, b)     (evbuttondown((id), (ev)->state))
#define evchkbuttons(ev, mask) ((ev)->state & (mask)) // buttons pressed?
#define evchkmods(ev, mask)    ((ev)->state & EVMODMASK) // modifiers active?
#define evgetmods(ev)          (evchkmods((ev), EVMODMASK))
#define evhasmod(ev, mod)      ((ev)->state & EV_MOD(mod)) // modifiers?
/* keyboard event size in octets */
#define _kbdevsize(ev)        (!((ev)->code & EVKBDSTATE) ? 8 : 12)
struct evkbd {
    evkeycode_t  code; // Unicode-value with possible EVKBDSTATEBIT
    /* state may not be present in protocol packets */
    evkeystate_t state; // modifier flags and pointer button state
};

/* pointer such as mouse device events */

#define pntrhasbutton(ev, b) ((ev)->state & (1 << (b)))
/* pointer device, e.g. mouse event */
struct evpntr {
    evuword_t button; // button ID
    evuword_t state;  // state bits for buttons; 1 -> down
    evword_t  x;      // screen X coordinate
    evword_t  y;      // screen Y coordinate
    evword_t  z;      // screen Z coordinate
};

#define EVNOTEMSGBIT    (1L << 31)

/* cmd-field for SYS */
/* EVSHUTDOWN is delivered no matter what events have been selected */
#define SYS_NO_OP       0
#define SYS_REBOOT      1 // reboot
#define SYS_SHUTDOWN    2 // shutdown
#define SYS_SLEEP       3 // power-saving sleep
#define SYS_WAKEUP      4 // ignite parts of system
#define SYS_HIBERNATE   5 // shutdown, store state for recovery
#define SYS_DEEP_SIX    6 // forceful shutdown
#define SYS_EVENT_TYPES 7

/* cmd-field for SYSHWNOTIFY
/* hardware/driver events */
#define HW_NO_OP       0
#define HW_LOAD        1 // kernel module loaded
#define HW_UNLOAD      3 // kernel module unloaded
#define HW_PLUG        3 // new device plugged
#define HW_UNPLUG      4 // device unplugged
#define HW_EVENT_TYPES 5

/* cmd-field for SYSFSNOTIFY */
/* event names */
#define FS_NO_OP       0
#define FS_MOUNT       1 // filesystem mount event
#define FS_UNMOUNT     2 // filesystem unmount event
#define FS_CREAT       3 // file creation event
#define FS_UNLINK      4 // file unlink event
#define FS_MKDIR       5 // add directory
#define FS_RMDIR       6 // remove directory
#define FS_EVENT_TYPES 7

struct evmsg {
    evword_t size; // # of bytes in msg
    uint8_t  data[EMPTY]; // message text
};

/* event structure for EVSYS, EVHW, and EVFS */
struct evnote {
    evword_t cmd; // command ID
    evword_t obj; // object such as file descriptor
    union {
        /* msg is present iff EVNOTEMSGBIT is set in cmd-member */
        evword_t dest;
        struct msg;
    }
};

struct evrequest {
    ;
};

/*
 * TODO: move this comment elsewhere...
 * Unicode specifies 0x10ffff as maximum value, leaving us with 11 high bits to
 * be used as flags if need be; let's start using them from the highest ones to
 * be safer, though
 */

/* FIXME: tm may need to be 64-bit for everyone */
struct evhdr {
    evtime_t  tm;               // timestamp; 32- or 64-bit
    evuword_t type;             // event type such as KEYUP, FSCREAT
};

/* event structure */
struct ev {
    struct evhdr       hdr;
    union {
        /* actual event message */
        struct evkbd   kbd; // EVKEYPRESS, EVKEYRELEASE
        /* EVBUTTONPRESS, EVBUTTONRELEASE, EVPOINTERMOTION, EVBUTTONMOTION */
        struct evpntr  pntr;
        struct evnote  note; // EVSYS, EVHW, EVFS
    } msg;
};

#endif /* __ZERO__BITS_EV_H__ */

