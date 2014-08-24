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
volatile FASTCALL struct m_tcb * thrpick(void);
#endif
ASMLINK void                     thrsave(struct thr *thr, long retadr, long fp);
FASTCALL void                    thrjmp(struct thr *thr);

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

/* thread */
/* states */
#define THRNONE   0x00                 // undefined
#define THRINIT   0x01                 // being initialised
#define THRRUN    0x02                 // running
#define THRREADY  0x03                 // ready to run
#define THRWAIT   0x04                 // waiting on system descriptor
#define THRSTOP   0x05                 // stopped
#define THRZOMBIE 0x06                 // finished but not waited for
struct thr {
    /* thread control block */
    struct m_tcb   m_tcb;               // context
    /* thread stacks */
    uintptr_t      ustk;                // user-mode stack
    uintptr_t      kstk;                // kernel-mode stack
    /* state */
    long           state;               // thread state
    /* wait channel */
    uintptr_t      wchan;               // wait channel
    /* linkage */
    struct proc   *proc;                // owner process
    /* queue linkage */
    struct thr    *prev;                // previous in queue
    struct thr    *next;                // next in queue
    long           id;
    /* scheduler parameters */
    long           nice;                // priority adjustment
    long           class;               // thread class
    long           prio;                // priority
//    long           interact;
    long           runtime;             // run time
    /* system call context */
    struct syscall syscall;             // current system call
} PACK();

#endif /* __KERN_THR_H__ */

