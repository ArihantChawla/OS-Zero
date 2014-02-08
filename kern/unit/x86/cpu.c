#include <kern/conf.h>
#include <zero/param.h>
#include <kern/unit/x86/cpu.h>
#if (SMP)
#include <stdint.h>
#include <kern/unit/ia32/mp.h>
#else
#include <kern/unit/ia32/boot.h>
#endif

struct m_cpu     cputab[NCPU] ALIGNED(PAGESIZE);
struct m_cpuinfo cpuinfotab[NCPU];

void
cpuinit(struct m_cpu *cpu)
{
    long core = cpu->id;
#if (SMP)
    void *kstk = (uint8_t *)MPENTRYSTK - cpu->id * MPSTKSIZE;
#else
    void *kstk = (void *)KERNSTKTOP;
#endif

    cpu->cpu = cpu;
    cpu->kstk = kstk;
    cpuprobe(&cpuinfotab[core]);

    return;
};

