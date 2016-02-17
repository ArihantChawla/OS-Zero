#ifndef __KERN_UNIT_X86_CPU_H__
#define __KERN_UNIT_X86_CPU_H__

#include <kern/conf.h>
#include <stdint.h>
//#include <zero/cdecl.h>
//#include <sys/types.h>
#if defined(__KERNEL__)
#include <zero/param.h>
#include <kern/types.h>
//#include <kern/cpu.h>
#include <kern/proc/proc.h>
#include <kern/proc/task.h>
#include <kern/unit/x86/bits/cpu.h>
#endif
//#include <zero/types.h>

#define cpugetclsize(cpu)  ((cpu)->l1i.clsz)
#define cpugetntlb(cpu)    ((cpu)->l1i.ntlb)
#define cpugetl1isize(cpu) ((cpu)->l1i.size)
#define cpugetl1inway(cpu) ((cpu)->l1i.nway)
#define cpugetl1dsize(cpu) ((cpu)->l1d.size)
#define cpugetl1dnway(cpu) ((cpu)->l1d.nway)
#define cpugetl2size(cpu)  ((cpu)->l2.size)
#define cpugetl2nway(cpu)  ((cpu)->l2.nway)

struct m_cpucache {
    uint32_t size;
    uint32_t clsz;
    uint32_t nway;
    uint32_t ntlb;
};

struct m_cpuinfo {
    long              flags;
    struct m_cpucache l1i;
    struct m_cpucache l1d;
    struct m_cpucache l2;
};

void cpuprobe(struct m_cpuinfo *cpuinfo);

#if defined(__KERNEL__)

#if (PTRBITS == 32)
extern long               k_curunit __asm__ ("%gs:0");
extern struct cpu       * k_curcpu  __asm__ ("%gs:4");
extern struct proc      * k_curproc __asm__ ("%gs:8");
extern struct task      * k_curtask __asm__ ("%gs:12");
extern pde_t              k_curpdir __asm__ ("%gs:16");
extern long               k_curpid  __asm__ ("%gs:20");
extern struct m_cpuinfo * k_cpuinfo __asm__ ("%gs:24");
#elif (PTRBITS == 64)
extern long               k_curunit __asm__ ("%gs:0"
extern struct m_cpu     * k_curcpu  __asm__ ("%gs:8");
extern struct proc      * k_curproc __asm__ ("%gs:16");
extern struct task      * k_curtask __asm__ ("%gs:24");
extern pde_t              k_curpdir __asm__ ("%gs:32");
extern long               k_curpid  __asm__ ("%gs:40");
extern struct m_cpuinfo * k_cpuinfo __asm__ ("%gs:48");
#endif

#endif /* defined(__KERNEL__) */

//#define NCPUWORD     6 /* cpu, proc, task, pdir, pid, info */
struct m_cpu {
    /* cpu-local variables */
    long              unit;
    struct cpu       *cpu;
    struct proc      *proc;
    struct task      *task;
    pde_t             pdir;
    long              pid;
    struct m_cpuinfo *info;
};

#endif /* __KERN_UNIT_X86_CPU_H__ */

