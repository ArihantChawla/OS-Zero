#ifndef __KERN_EV_H__
#define __KERN_EV_H__

#include <stdint.h>
#include <wchar.h>
#include <zero/cdecl.h>

typedef uint32_t evid_t;

/* event header structure */

/* user input events */

/* event types */
#define EVKEYDOWN       0x01
#define EVKEYUP         0x02
#define EVBUTTONDOWN    0x03
#define EVBUTTONUP      0x04
#define EVPNTMOTION     0x05
#define EVCMD           0x06
#define EVDATA          0x07
/* filesystem events */
#define EVFSCREAT       0x08
#define EVFSUNLINK      0x09
#define EVFSMKDIR       0x0a
#define EVFSRMDIR       0x0b

/* keyboard events */

/* Unicode specifies 0x10ffff as maximum value, leaving us with 11 high bits to
 * be used as flags if need be
 */

/* the highest bit in sym indicates keyrelease event */
#define EVKBDUPBIT       0x80000000
/* mask-bits for modifier keys */
#define EVKBDSHIFT       0x00000001             // Shift
#define EVKBDCAPSLK      0x00000002             // Caps Lock
#define EVKBDCTRL        0x00000004             // Ctrl
#define EVKBDMETA        0x00000008             // Meta
#define EVKBDCOMPOSE     0x00000010             // Compose
#define EVKBDALT         0x00000020             // Alt
#define EVKBDALTGR       0x00000040             // AltGr
#define EVKBDSCRLOCK     0x00000080             // Scroll lock
#define EVKBDNFLGBIT     8
#define kbducval(ev)     ((ev)->sym & WCUCMASK) // extract Unicode value
#define kbdisup(ev)      ((ev)->state < 0)      // sign-bit indicates keypress
#define kbdbutton(ev, b) ((ev)->state & (1L << ((b) + EVKBDNFLGBIT)))
#define kbdmod(ev, mod)  ((ev)->state & (mod))
struct evkbd {
    wchar_t sym;                                // key symbol
    int32_t state;                              // button state mask if present
} PACK();

/* pointer such as mouse device events */

#define EVPNTUP          0x8000000
#define pntisup(ev)      ((ev)->button < 0)     // sign-bit means release
#define pntbutton(ev, b) (ev->state & (1 << (b)))
#define pnt
/* pointer device, e.g. mouse event */
struct evpnt {
    int32_t  button;                    // button ID; sign-bit indicates release
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

struct fsevent {
    uint32_t type;                      // event type
    uint32_t dev;                       // device ID
    int64_t  node;                      // node (file, directory) ID
} PACK();

/* API */
/* event-queue is mapped to both kernel and user space to avoid copying data */
/* register to listen to ev with flg parameters on event queue at qadr */
void        evreg(long ev, long flg, void *qadr);
/* check queue for event type ev; parameters in flg */
/* check current queue; don't flush server connection */
#define EVNOFLUSH       0x01
/* fetch/remove event from queue */
#define EVFETCH         0x02
struct ev * evchk(long ev, long flg, void *qadr);
/* read next event from queue */
/* leave copy of fetched event in the queue */
#define EVNOREMOVE      0x01
/* do not flush server connection before get */
#define EVNOFLUSH       0x02
void        evget(struct ev *evp, long flg, void *qadr);

#endif /* __KERN_EV_H__ */

