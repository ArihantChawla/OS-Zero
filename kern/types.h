#ifndef __KERN_TYPES_H__
#define __KERN_TYPES_H__

#include <stdint.h>
#include <time.h>
//#include <sys/types.h>
#include <mach/param.h>
#include <zero/trix.h>
#if defined(__x86_64__) || defined(__amd64__)
#include <kern/unit/x86-64/types.h>
#elif defined(__i386__) && !defined(__x86_64__) && !defined(__amd64__)
#include <kern/unit/ia32/types.h>
#endif

/* machine [integer] register for better portability */
/* Windows compilers have 32-bit longs on 64-bit systems... */
#if (WORDSIZE == 4)
typedef int32_t   m_ireg_t;
typedef uint32_t  m_ureg_t;
#elif (WORDSIZE == 8)
typedef int64_t   m_ireg_t;
typedef uint64_t  m_ureg_t;
#endif
/* byte-pointer (for arithmetics */
#define m_castadr(adr, type) ((type)(adr))

/* signed types to facilitate negative return values such as -1 */
typedef intptr_t desc_t;

struct stkhdr {
    int8_t *top;
    int8_t *sp;
};

/* the structures we wait on are prefixed by this one */
/* for sleeping, waketm is wakeup-time (system tick ID) */
/* wait-channel is simply the address of this header as a uintptr_t */
#define __STRUCT_WAITHDR_SIZE                                           \
    (2 * sizeof(long) + sizeof(void *) + sizeof(time_t))
#define __STRUCT_WAITHDR_PAD                                            \
    (rounduppow2(__STRUCT_WAITHDR_SIZE, CLSIZE) - __STRUCT_WAITHDR_SIZE)
struct waithdr {
    volatile long  lk;          // mutex for modifying the queue
    long           cnt;         // # of items in queue
    time_t         waketm;      // wakeup-tick
    struct thr    *thrqueue;    // waiter queue
    uint8_t        _pad[__STRUCT_WAITHDR_PAD];
};

#endif /* __KERN_TYPES_H__ */

