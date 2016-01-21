#include <kern/conf.h>
#include <zero/param.h>
#include <kern/cpu.h>
#include <kern/mem/vm.h>
#if (SMP)
#include <stdint.h>
#include <kern/unit/ia32/mp.h>
#else
#include <kern/unit/x86/boot.h>
#endif

extern void      cpuprintinfo(void);
extern uint8_t   kernusrstktab[NCPU * KERNSTKSIZE];

struct m_cpuinfo cpuinfotab[NCPU];

void
cpuinit(volatile struct cpu *cpu)
{
    long              id = cpu->id;
    struct m_cpuinfo *info = &cpuinfotab[id];
#if 0
#if (SMP)
    void *kstk = (uint8_t *)MPENTRYSTK - id * MPSTKSIZE;
#else
//    void *kstk = (void *)(KERNVIRTBASE - id * KERNSTKSIZE);
    void *kstk = kernusrstktab + (id + 1) * KERNSTKSIZE;
#endif
#endif

//    cpu->kstk = kstk;
    cpuprobe(info);
    cpu->m_cpu.info = info;
    cpuprintinfo();

    return;
};

