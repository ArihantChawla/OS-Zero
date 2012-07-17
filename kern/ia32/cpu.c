#include <kern/conf.h>
#include <zero/param.h>
#include <kern/ia32/cpu.h>
#if (SMP)
#include <stdint.h>
#include <kern/ia32/mp.h>
#else
#include <kern/ia32/boot.h>
#endif

struct m_cpu     cputab[NCPU] ALIGNED(PAGESIZE);
struct m_cpuinfo cpuinfo[NCPU];

void
cpuinit(struct m_cpu *cpu)
{
#if (SMP)
    void *kstk = (uint8_t *)MPSTKSIZE + cpu->id * MPSTKSIZE;
#else
    void *kstk = (void *)KERNSTKTOP;
#endif

    cpu->cpu = cpu;
    cpu->kstk = kstk;

    return;
};

