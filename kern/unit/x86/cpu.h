#ifndef __KERN_UNIT_X86_CPU_H__
#define __KERN_UNIT_X86_CPU_H__

#include <kern/conf.h>
#include <stdint.h>
//#include <zero/cdecl.h>
#include <sys/types.h>
#include <zero/param.h>
#include <zero/types.h>
#include <kern/types.h>

#define CPUHASFXSR 0x00000001
#define CPUHASAPIC 0x00000002

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
extern struct m_cpu * k_curcpu  __asm__ ("%gs:0");
extern struct proc  * k_curproc __asm__ ("%gs:4");
extern struct task  * k_curtask __asm__ ("%gs:8");
extern pde_t        * k_curpdir __asm__ ("%gs:12");
extern pid_t          k_curpid  __asm__ ("%gs:16");
//extern pde_t        *k_cursigvec __asm__ ("%gs:16");
#elif (PTRBITS == 64)
extern struct m_cpu * k_curcpu  __asm__ ("%gs:0");
extern struct proc  * k_curproc __asm__ ("%gs:8");
extern struct task  * k_curtask __asm__ ("%gs:16");
extern uint64_t     * k_curpml4 __asm__ ("%gs:24");
extern pid_t          k_curpid  __asm__ ("%gs:32");
//extern pde_t        *k_cursigvec __asm__ ("%gs:32");
#endif

#define NCPUWORD     6 /* cpu, proc, task, info, pdir/pml4, started */
struct m_cpu {
    /* cpu-local variables */
    struct m_cpu     *cpu;
    struct proc      *proc;
    struct task      *task;
#if (PTRBITS == 32)
    uint32_t          pdir;
#elif (PTRBITS == 64)
    uint64_t          pml4;
#endif
    long              pid;
    /* end of cpu local variables */
    volatile long     started;
    struct m_cpuinfo *info;
    long              id;
    struct m_tcb     *schedtcb;
    uint8_t          *kstk;
    uint64_t         *gdt;
#if !defined(__x86_64__) && !defined(__amd64__)
    struct m_tss      ktss;
    struct m_tss      utss;
#endif
};

#endif /* defined(__KERNEL__) */

#endif /* __KERN_UNIT_X86_CPU_H__ */

