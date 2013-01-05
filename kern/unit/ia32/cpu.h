#ifndef __UNIT_IA32_CPU_H__
#define __UNIT_IA32_CPU_H__

#include <stdint.h>
#include <zero/types.h>
#include <kern/conf.h>
#include <kern/types.h>

#if (SMP)
#define NCPU 8
#else
#define NCPU 1
#endif

#define CPUHASFXSR 0x00000001

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

extern struct m_cpu *curcpu  __asm__ ("%gs:0");
extern struct proc  *curproc __asm__ ("%gs:4");
extern struct thr   *curthr  __asm__ ("%gs:8");
extern pde_t        *curpdir __asm__ ("%gs:12");
struct m_cpu {
    /* cpu local variables */
    struct m_cpu  *cpu;
    struct proc   *proc;
    struct thr    *thr;
    pde_t          pdir;
    /* end of cpu local variables */
    long           id;
    volatile long  started;
    struct m_tcb  *schedtcb;
    uint8_t       *kstk;
    uint64_t      *gdt;
    struct m_tss   ktss;
    struct m_tss   utss;
};

#endif /* __UNIT_IA32_CPU_H__ */

