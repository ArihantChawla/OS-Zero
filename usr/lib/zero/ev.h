#ifndef __ZERO_EV_H__
#define __ZERO_EV_H__

#if defined(__KERNEL__)
#include <kern/malloc.h>
#endif

#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#if 0
#include <zero/cdefs.h>
#include <zero/param.h>
#endif
//#include <deck/deck.h>
#include <gfx/rgb.h>

#define EV_OBJ_BITS  64
#define EV_TIME_BITS 64
#define EV_WORD_BITS 32

#include <zero/bits/ev.h>

/* internal protocol events
 * - response event ID of 0 is protocol messages, event ID 1 for protocol errors
 */
#define EVPROTO            0  // protocol request or response
#define EVERROR            1  // protocol error [message]
#define EVSYSNOTIFY        2  // [operating] system event
#define EVHWNOTIFY         3  // hardware/driver event
#define EVFSNOTIFY         4  // filesystem event
/* struct evkbd */
#define EVKEYPRESS         5  // keypress event
#define EVKEYRELEASE       6  // keyrelease event
/* struct evpntr */
#define EVBUTTONPRESS      7  // buttonpress event
#define EVBUTTONRELEASE    8  // buttonrelease event
#define EVPOINTERMOTION    9  // pointer/cursor motion event
/* struct evcross */
#define EVENTER            10 // enter window event
#define EVLEAVE            11 // leave window event
#define EVFOCUSIN          12 // focus in event
#define EVFOCUSOUT         13 // focus out event
/* struct evnote */
#define EVKEYMAP           14 // keymap notification
#define EVVISIBILITY       15 // visibility notification
#define EVEXPOSE           16 // exposure notification
#define EVMAP              17 // map notification
#define EVUNMAP            18 // unmap notification
#define EVCREATE           19 // create notification
#define EVDESTROY          20 // destroy notification
#define EVCONFIGURE        21 // configure notfication
#define EVREPARENT         22 // reparent notification
#define EVCIRCULATE        23 // circulation notification
#define EVGRAVITY          24 // gravity notification
#define EVPROPERTY         25 // property notification
#define EVCOLORMAP         26 // colormap notification
#define EVCLIENTMESSAGE    27 // message from another client
/* events for window managers */
#define EVMAPREQUEST       28 // map window request
#define EVCONFIGUREREQUEST 29 // configure window request
#define EVRESIZEREQUEST    30 // resize window request
#define EVCIRCULATEREQUEST 31 // circulate window request
#define EVCOLORMAPREQUEST  32 // change colormap request
#define EVKILLREQUEST      33 // kill client request
#define EV_STATIC_EVENTS   35 // number of static events for lookup tables

/*
 * bitmask values used to choose input events
 */
#define NO_EVENT_BIT              0
#define KEY_PRESS_BIT             (1L << 0)
#define KEY_RELEASE_BIT           (1L << 1)
#define BUTTON_PRESS_BIT          (1L << 2)
#define BUTTON_RELEASE_BIT        (1L << 3)
#define POINTER_MOTION_BIT        (1L << 6)
#define POINTER_MOTION_HINT_BIT   (1L << 7)
#define BUTTON_MOTION_BIT         (1L << 8)
#define ENTER_WINDOW_BIT          (1L << 4)
#define LEAVE_WINDOW_BIT          (1L << 5)
#define KEYMAP_STATE_BIT          (1L << 9)
#define EXPOSURE_BIT              (1L << 10)
#define VISIBILITY_CHANGE_BIT     (1L << 11)
#define STRUCTURE_NOTIFY_BIT      (1L << 12)
#define RESIZE_REDIRECT_BIT       (1L << 13)
#define SUBSTRUCTURE_NOTIFY_BIT   (1L << 14)
#define SUBSTRUCTURE_REDIRECT_BIT (1L << 15)
#define FOCUS_CHANGE_BIT          (1L << 16)
#define PROPERTY_CHANGE_BIT       (1L << 17)
#define COLORMAP_CHANGE_BIT       (1L << 18)
#define OWNER_GRAB_BUTTON_BIT     (1L << 19)

#define evgettype(ev)    ((ev)->hdr.type)
#define evsettype(ev, t) ((ev)->hdr.type = (t))
#define evgettime(ev)    ((ev)->hdr.tm)
#define evsettime(ev, t) ((ev)->hdr.tm = (t))

/* API */
#define evmask(id) (1U << (id))
/* TODO: implement ring-buffers for event queues */
/* - wired to physical memory permanently */
/* - event queue is mapped to both kernel and user space to avoid data copies */
/* - register to listen to ev with flg parameters */
/*   - evreg returns pointer to dual-mapped event queue (set of pages) */
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
/* read events; evget() */
#define EVNOFLUSH   0x80000000  // check queue; do not flush connection
#define EVNOREMOVE  0x40000000  // do not remove event from queue
/* queue events; evput() */
#define EVQUEUE     0x20000000
#define EVDEQUEUE   0x10000000
/* flg-argument bits for evsync() */
#define EVSYNC      0x08000000  // asynchronous if not set
#define EVTOSSINPUT 0x04000000  // discard pending user input

#define evpeek(deck, ev, flg) evget((deck), (ev), (flg) | EVNOREMOVE)
void    evget(struct deck *deck, struct ev *ev, long flg);
long    evput(struct deck *deck, struct ev *ev, long flg);
void    evsync(struct deck *deck, long flg);

#if defined(__KERNEL__)

#if !defined(RING_ITEM)
#define RING_ITEM  struct ev
#endif
#if !defined(RING_INVAL)
#define RING_INVAL { 0 }
#endif
#if !defined(MALLOC)
#define MALLOC(sz) kmalloc(sz)
#endif

#else

#define RING_ITEM  struct ev
#define RING_INVAL NULL
#define MALLOC(sz) malloc(sz)

#include <zero/ring.h>

#endif /* defined(__KERNEL__) */

#endif /* __ZERO_EV_H__ */

