#ifndef __KERN_EV_H__
#define __KERN_EV_H__

#include <stdint.h>

/* user input events */

#define EVKEYDOWN     0x01
#define EVKEYUP       0x02
#define EVBUTTONDOWN  0x03
#define EVBUTTONUP    0x04
#define EVPNTMOTION   0x05

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
    uint32_t val;
};

#define EVPNTBUTTONUP 0x80000000U
/* pointer device, e.g. mouse event */
struct evpnt {
    uint32_t state;
    uint32_t x;
    uint32_t y;
};

struct uievent {
    uint32_t type;
    union {
        struct evkbd kbd;
        struct evpnt pnt;
    };
};

/* file system events */

#define EVCREAT       0x01
#define EVUNLINK      0x02
#define EVMKDIR       0x03
#define EVRMDIR       0x04

struct fsevent {
    uint32_t type;
    uint32_t dev;
    uint64_t node;
};

#endif /* __KERN_EV_H__ */

