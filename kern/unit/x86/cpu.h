#ifndef __KERN_UNIT_X86_CPU_H__
#define __KERN_UNIT_X86_CPU_H__

#include <kern/conf.h>
#include <stdint.h>
//#include <zero/cdecl.h>
#include <sys/types.h>
#include <zero/param.h>
#include <zero/types.h>
#include <kern/types.h>
#include <kern/unit/x86/bits/cpu.h>

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
extern struct m_cpu     * k_curcpu   __asm__ ("%gs:0");
extern struct proc      * k_curproc  __asm__ ("%gs:4");
extern struct task      * k_curtask  __asm__ ("%gs:8");
extern pde_t              k_curpdir  __asm__ ("%gs:12");
extern long               k_curpid   __asm__ ("%gs:16");
extern struct m_cpuinfo * k_cpuinfo  __asm__ ("%gs:20");
extern volatile long      k_cpustate __asm__ ("%gs:24");
extern long               k_cpuid    __asm__ ("%gs:28");
//extern pde_t        *k_cursigvec __asm__ ("%gs:16");
#elif (PTRBITS == 64)
extern struct m_cpu     * k_curcpu   __asm__ ("%gs:0");
extern struct proc      * k_curproc  __asm__ ("%gs:8");
extern struct task      * k_curtask  __asm__ ("%gs:16");
extern pde_t              k_curpdir  __asm__ ("%gs:24");
extern long               k_curpid   __asm__ ("%gs:32");
extern struct m_cpuinfo * k_cpuinfo  __asm__ ("%gs:40");
extern volatile long      k_cpustate __asm__ ("%gs:48");
extern long               k_cpuid    __asm__ ("%gs:56");
//extern pde_t        *k_cursigvec __asm__ ("%gs:32");
#endif

#define NCPUWORD     8 /* cpu, proc, task, pdir/pml4, pid, info, state, id */
/* CPU state values */
#define CPUSTARTED   1L
#define CPURESCHED   2L
struct m_cpu {
    /* cpu-local variables */
    struct m_cpu     *cpu;
    struct proc      *proc;
    struct task      *task;
    pde_t             pdir;
    long              pid;
    struct m_cpuinfo *info;
    /* end of cpu local variables */
    volatile long     state;
    long              id;
#if 0
    struct m_tcb     *schedtcb;
    uint8_t          *kstk;
    uint64_t         *gdt;
#if !defined(__x86_64__) && !defined(__amd64__)
    struct m_tss      ktss;
    struct m_tss      utss;
#endif
#endif
};

#endif /* defined(__KERNEL__) */

#endif /* __KERN_UNIT_X86_CPU_H__ */

