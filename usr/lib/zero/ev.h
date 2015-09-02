#ifndef __ZERO_EV_H__
#define __ZERO_EV_H__

#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#if 0
#include <zero/cdecl.h>
#include <zero/param.h>
#endif
#include <deck/deck.h>
#include <gfx/rgb.h>

#define EVWORDSIZE   32
#define EVNODEIDSIZE 64

#include <zero/bits/ev.h>

#define evgettype(ev)    ((ev)->hdr.type)
#define evsettype(ev, t) ((ev)->hdr.type = (t))
#define evgettime(ev)    ((ev)->hdr.tm)
#define evsettime(ev, t) ((ev)->hdr.tm = (t))
/* FIXME: tm may need to be 64-bit */
struct evhdr {
    evuword_t type;             // event type such as KEYUP, FSCREAT
    evuword_t tm;               // timestamp
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
};

#if (!__KERNEL__)

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

#endif /* !__KERNEL__ */

#endif /* __ZERO_EV_H__ */

