#include <kern/conf.h>
#include <zero/param.h>
#include <kern/mem/vm.h>
#include <kern/unit/x86/cpu.h>
#if (SMP)
#include <stdint.h>
#include <kern/unit/ia32/mp.h>
#else
#include <kern/unit/x86/boot.h>
#endif

extern void cpuprintinfo(void);

volatile struct m_cpu cputab[NCPU] ALIGNED(PAGESIZE);
struct m_cpuinfo      cpuinfotab[NCPU];

void
cpuinit(struct m_cpu *cpu)
{
    long id = cpu->id;
#if (SMP)
    void *kstk = (uint8_t *)MPENTRYSTK - id * MPSTKSIZE;
#else
    void *kstk = (void *)(KERNVIRTBASE - id * KERNSTKSIZE);
#endif
    struct m_cpuinfo *info = &cpuinfotab[id];

//    cpu->cpu = cpu;
    cpu->kstk = kstk;
    cpuprobe(info);
    cpu->info = info;
    cpuprintinfo();

    return;
};

