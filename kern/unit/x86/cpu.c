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
extern uint8_t  kernusrstktab[NCPU * KERNSTKSIZE];

volatile struct m_cpu cputab[NCPU] ALIGNED(PAGESIZE);
struct m_cpuinfo      cpuinfotab[NCPU];

void
cpuinit(volatile struct m_cpu *cpu)
{
    long id = cpu->id;
#if 0
#if (SMP)
    void *kstk = (uint8_t *)MPENTRYSTK - id * MPSTKSIZE;
#else
//    void *kstk = (void *)(KERNVIRTBASE - id * KERNSTKSIZE);
    void *kstk = kernusrstktab + (id + 1) * KERNSTKSIZE;
#endif
#endif
    struct m_cpuinfo *info = &cpuinfotab[id];

    cpu->cpu = cpu;
//    cpu->kstk = kstk;
    cpuprobe(info);
    cpu->info = info;
    cpuprintinfo();

    return;
};

