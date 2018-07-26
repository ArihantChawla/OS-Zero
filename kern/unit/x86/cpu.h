#ifndef __KERN_UNIT_X86_CPU_H__
#define __KERN_UNIT_X86_CPU_H__

//#include <kern/proc/task.h>
#include <kern/unit/x86/cpuid.h>

#if (PTRBITS == 32)
extern volatile struct cpu  * k_curcpu  __asm__ ("%gs:0");
extern volatile long          k_curunit __asm__ ("%gs:4");
extern volatile struct task * k_curtask __asm__ ("%gs:8");
extern volatile long          k_curpid  __asm__ ("%gs:12");
#elif (PTRBITS == 64)
extern volatile struct cpu  * k_curcpu  __asm__ ("%gs:0");
extern volatile long          k_curunit __asm__ ("%gs:8");
extern volatile struct task * k_curtask __asm__ ("%gs:16");
extern volatile long          k_curpid  __asm__ ("%gs:24");
#endif

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
    pde_t             pdir;
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

