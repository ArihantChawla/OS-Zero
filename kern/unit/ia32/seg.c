#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/types.h>
#include <kern/conf.h>
#include <kern/proc.h>
#include <kern/unit/x86/cpu.h>
#include <kern/unit/ia32/boot.h>
#include <kern/unit/ia32/seg.h>
#include <kern/unit/ia32/vm.h>

extern void gdtinit(void);

#if (SMP)
extern uint64_t        kerngdt[NCPU][NGDT];
#else
extern uint64_t        kerngdt[NGDT];
#endif
//extern struct m_tss    tsstab[NTHR];
#if (SMP)
extern struct m_cpu    mpcputab[NCPU];
#else
extern struct m_cpu    cputab[NCPU];
#endif
extern struct m_farptr gdtptr;

ASMLINK void
seginit(long id)
{
#if (SMP)
    struct m_cpu    *cpu = &mpcputab[curcpu->id];
#else
    struct m_cpu    *cpu = &cputab[curcpu->id];
#endif
    uint64_t        *gdt;

    /* set descriptors */
#if (SMP)
    gdt = &kerngdt[id][0];
#else
    gdt = kerngdt;
#endif
    segsetdesc(&gdt[TEXTSEG], 0, NPAGEMAX - 1,
               SEGCODE);
    segsetdesc(&gdt[DATASEG], 0, NPAGEMAX - 1,
               SEGDATA);
#if 0
    segsetdesc(&gdt[TSSSEG], &tsstab[id], sizeof(struct m_tss), SEGTSS);
#endif
    segsetdesc(&gdt[UTEXTSEG], 0, NPAGEMAX - 1, SEGCODE | SEGUSER);
    segsetdesc(&gdt[UDATASEG], 0, NPAGEMAX - 1, SEGDATA | SEGUSER);
    /* per-CPU data segment for curcpu etc. */
    segsetdesc(&gdt[CPUSEG], &cpu->cpu, 4 * sizeof(void *),
               SEGCPU);
    /* initialize segmentation */
    gdtptr.lim = NGDT * sizeof(uint64_t) - 1;
    gdtptr.adr = (uint32_t)gdt;
    gdtinit();
//    __asm__ __volatile__ ("movw %0, %%ax\n" : : "i" (CPUSEL));
//    __asm__ __volatile__ ("movw %ax, %gs\n");

    return;
}

