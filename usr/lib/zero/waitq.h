#ifndef __ZERO_WAITQ_H__
#define __ZERO_WAITQ_H__

/* REFERENCE: https://arxiv.org/pdf/1112.1141.pdf */

#include <stdlib.h>
#include <zero/cdecl.h>
#include <zero/cond.h>

#define WAITQ_MAX  (1L << 16)
#define WAITQ_NONE (~0L)

#define WAITQ_SIGNAL_BIT 0x01L
#define WAITQ_WAITER_BIT 0x02L
struct waitq {
    volatile long  lk;           // mutual exclusion
    long           id;           // waitq ID
    long           flg;          // flag-bits
    long           status;       // application status
    long           event;        // event type
    long           prev;         // previous on queue
    long           next;         // next on queue
    COND_T         cond;         // condition variable
#if 0
    void          *signal(void);
    void          *wait(void);
    long          *poll(void);
#endif
};

#endif /* __ZERO_WAITQ_H__ */

