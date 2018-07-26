#ifndef __KERN_TYPES_H__
#define __KERN_TYPES_H__

#include <stdint.h>
#include <time.h>
#include <mach/atomic.h>
/* byte-pointer (for arithmetics */
#define m_castadr(adr, type) ((type)(adr))

/* signed types to facilitate negative return values such as -1 */
typedef int32_t desc_t;

struct stkhdr {
    int8_t *top;
    int8_t *sp;
};

/* the structures we wait on are prefixed by this one */
/* for sleeping, waketm is wakeup-time (system tick ID) */
/* wait-channel is simply the address of this header as a uintptr_t */
struct waithdr {
    volatile m_atomic_t  lk;          // mutex for modifying the queue
    long                 cnt;         // # of items in queue
    time_t               waketm;      // wakeup-tick
    struct thr          *thrqueue;    // waiter queue
};

#endif /* __KERN_TYPES_H__ */

