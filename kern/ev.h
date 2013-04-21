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
#define EVRPC         0x06
#define EVDATA        0x07

/* keyboard events */

/* the highest bit indicates a control key mask */
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
    uint32_t val;                       //  control state or Unicode/ISO value

};

/* pointer such as mouse device events */

#define evpntisup(ev, button) (ev->state & (1 << (button)))
/* pointer device, e.g. mouse event */
struct evpnt {
    uint32_t state;                     // state bits for buttons; 1 -> up
    uint32_t x;                         // screen X coordinate
    uint32_t y;                         // screen Y coordinate
} PACK();

struct uievent {
    uint32_t type;                      // event type
    union {                             // union to store event
        struct evkbd kbd;
        struct evpnt pnt;
    } ev;
} PACK();

/* IPC events */

/* command packet */
struct evcmd {
    uint32_t cmd;                       // RPC command
    uint32_t dest;                      // destination descriptor
} PACK();

/* data packet */
struct evdata {
    uint32_t fmt;                       // data format
    uint32_t itemsz;                    // data-word size
    uint64_t nitem;                     // number of words
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

#endif /* __KERN_EV_H__ */

