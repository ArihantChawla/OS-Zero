#ifndef __KERN_CPU_H__
#define __KERN_CPU_H__

#include <stdint.h>
#include <zero/param.h>
#include <zero/trix.h>

#if (defined(__i386__) || defined(__i486__)                             \
     || defined(__i586__) || defined(__i686__)                          \
     || defined(__x86_64__) || defined(__amd64__))
#include <kern/unit/x86/cpu.h>
#endif

#define __STRUCT_CPU_SIZE (sizeof(struct m_cpu) + 7 * sizeof(long))
/* CPU statflg values */
#define CPUSTARTED   (1L << 0)
#define CPURESCHED   (1L << 1)
struct cpu {
    struct m_cpu   m_cpu;       // machine-specific structure
    volatile long  lk;          // mutual exclusion lock
    volatile long  statflg;     // status flags
    long           id;          // numerical ID
    long           ntick;       // tick count
    long           nicemin;     // minimum nice in effect
    long           loadtmsh;    // timeshare load
    long           load;        // load
    uint8_t        _pad[rounduppow2(__STRUCT_CPU_SIZE, CLSIZE)];
};

#endif /* __KERN_CPU_H__ */

