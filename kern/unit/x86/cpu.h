#ifndef __KERN_UNIT_X86_CPU_H__
#define __KERN_UNIT_X86_CPU_H__

#include <kern/proc/proc.h>
#include <kern/proc/task.h>
#include <kern/unit/x86/cpuid.h>

//#define NCPUWORD     6 /* cpu, proc, task, pdir, pid, info */
/* CPU flg-values */
#define CPUSTARTED (1L << 0)
#define CPURESCHED (1L << 1)
#define CPUINITBIT (1L << 2)
#define CPUHASINFO (1L << 3)
struct cpu {
    /* cpu-local variables */
    struct cpu       *cpu;
    long              unit;
    struct task      *task;
    long              pid;
    struct proc      *proc;
    uintptr_t         pdir;
    long              flg;
    /* scheduler parameters */
    unsigned long     ntick;    // tick count
    long              nicemin;  // minimum nice in effect
    long              loaduser; // user/timeshare load
    long              load;     // load
    /* info about cpu cache and features */
    struct m_cpuinfo  info;
};

#endif /* __KERN_UNIT_X86_CPU_H__ */

