#ifndef __KERN_CPU_H__
#define __KERN_CPU_H__

#include <kern/conf.h>
#if (defined(__i386__) || defined(__i486__)                             \
     || defined(__i586__) || defined(__i686__)                          \
     || defined(__x86_64__) || defined(__amd64__))
#include <kern/unit/x86/cpuid.h>
#endif

//#define NCPUWORD     6 /* cpu, proc, task, pdir, pid, info */
/* CPU flg-values */
#define CPUSTARTED (1L << 0)
#define CPURESCHED (1L << 1)
#define CPUINITBIT (1L << 2)
#define CPUHASINFO (1L << 3)
struct cpu {
    /* cpu-local variables */
    struct cpu       *cpu;      // first item for k_getcurcpu/k_setcurcpu
    long              unit;     // system processor unit ID
    struct task      *task;     // current task data
    long              pid;      // ID of current process
    struct proc      *proc;     // current process data
    uintptr_t         pdir;     // page directory page address
    long              flg;      // flags as listed above
    volatile uint32_t *apic;    // local APIC base address
    /* scheduler parameters */
    uintmax_t         ntick;    // tick count
    long              nicemin;  // minimum nice in effect
    long              loaduser; // user/timeshare load
    long              load;     // load
    /* info about cpu cache and features */
    struct m_cpuinfo  info;
};

extern volatile struct cpu k_cputab[CPUSMAX];

#endif /* __KERN_CPU_H__ */

