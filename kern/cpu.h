#ifndef __KERN_CPU_H__
#define __KERN_CPU_H__

#include <stddef.h>
#include <stdint.h>
#include <sys/param.h>
#include <zero/param.h>
//#include <zero/trix.h>
#include <kern/types.h>
#if (defined(__i386__) || defined(__i486__)                             \
     || defined(__i586__) || defined(__i686__)                          \
     || defined(__x86_64__) || defined(__amd64__))
#include <kern/unit/x86/cpu.h>
#endif

//#define __STRUCT_CPU_SIZE (sizeof(struct m_cpu) + 7 * sizeof(long))
#define __STRUCT_CPU_SIZE                                               \
    (7 * sizeof(long) + sizeof(void *) + sizeof(struct m_cpu))
#define __STRUCT_CPU_PAD                                                \
    (roundup(__STRUCT_CPU_SIZE, CLSIZE) - __STRUCT_CPU_SIZE)
/* CPU statflg-values */
#define CPUSTARTED   (1L << 0)
#define CPURESCHED   (1L << 1)
struct cpu {
    struct m_cpu   m_cpu;       // machine-specific structure
    m_atomic_t     lk;          // mutual exclusion lock
    struct m_core *coretab;     // SMT
    m_atomic_t     statflg;     // status flags
    long           id;          // numerical ID
    unsigned long  ntick;       // tick count
    long           nicemin;     // minimum nice in effect
    long           loaduser;    // user/timeshare load
    long           load;        // load
    uint8_t        _pad[__STRUCT_CPU_PAD];
};

#endif /* __KERN_CPU_H__ */

