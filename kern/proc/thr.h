#ifndef __KERN_THR_H__
#define __KERN_THR_H__

#include <kern/conf.h>
#include <stddef.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/types.h>
#include <kern/unit/x86/cpu.h>
#if defined(__x86_64__)
#include <kern/unit/x86-64/asm.h>
#elif defined(__i386__)
#include <kern/unit/ia32/thr.h>
#elif defined(__arm__)
#include <kern/unit/arm/asm.h>
#elif defined(__ppc__)
#include <kern/unit/ppc/asm.h>
#endif

//extern struct m_cpuinfo cpuinfo;

#if (ZEROSCHED)
void thryield(void);
#endif

#define __KERNEL__ 1
#include <zero/mtx.h>

struct thrq {
    volatile long  lk;
    struct thr    *head;
    struct thr    *tail;
};

#if (LONGSIZE == 8)
#define NLVLTHRLOG2 16
#elif (LONGSIZE == 4)
#define NLVLTHRLOG2 8
#endif
#define NLVL0THR    (1 << NLVLTHRLOG2)
#define NLVL1THR    (1 << NLVLTHRLOG2)
#define NLVL2THR    (1 << NLVLTHRLOG2)
#define NLVL3THR    (1 << NLVLTHRLOG2)
#define THRNKEY     4

#define thrwaitkey0(wc) (((wc) >> 3 * NLVLTHRLOG2) & ((1UL << NLVLTHRLOG2) - 1))
#define thrwaitkey1(wc) (((wc) >> 2 * NLVLTHRLOG2) & ((1UL << NLVLTHRLOG2) - 1))
#define thrwaitkey2(wc) (((wc) >> 1 * NLVLTHRLOG2) & ((1UL << NLVLTHRLOG2) - 1))
#define thrwaitkey3(wc) ((wc) & ((1UL << NLVLTHRLOG2) - 1))

struct thrwait {
    volatile long  lk;
    long           nref;
    void          *ptr;
};

#if 0
#if (!QEMU)
#define FPUCTX 1
#endif
#endif

#endif /* __KERN_THR_H__ */

