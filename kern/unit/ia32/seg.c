#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/types.h>
#include <kern/conf.h>
//#include <kern/proc.h>
#include <kern/mem/vm.h>
#include <kern/unit/x86/boot.h>
#include <kern/unit/x86/cpu.h>
#include <kern/unit/ia32/seg.h>

extern void gdtinit(void);

#if (SMP)
extern uint64_t        kerngdt[NCPU][NGDT];
#else
extern uint64_t        kerngdt[NGDT];
#endif
//extern struct m_tss    tsstab[NTHR];
extern struct m_cpu    cputab[NCPU];
extern struct m_farptr gdtptr;

ASMLINK void
seginit(long id)
{
    struct m_cpu    *cpu = &cputab[id];
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
//    segsetdesc(&gdt[TSSSEG], &tsstab[id], sizeof(struct m_tss), SEGTSS);
    segsetdesc(&gdt[UTEXTSEG], 0, NPAGEMAX - 1, SEGCODE | SEGUSER);
    segsetdesc(&gdt[UDATASEG], 0, NPAGEMAX - 1, SEGDATA | SEGUSER);
    /* per-CPU data segment */
    segsetdesc(&gdt[CPUSEG], &cpu->cpu, 6 * sizeof(void *),
               SEGCPU);
#if (VBE)
    gdt[REALCODESEG] = UINT64_C(0x00009a000000ffff);
    gdt[REALDATASEG] = UINT64_C(0x000092000000ffff);
#endif
    /* initialize segmentation */
    gdtptr.lim = NGDT * sizeof(uint64_t) - 1;
    gdtptr.adr = (uint32_t)gdt;
    gdtinit();
//    __asm__ __volatile__ ("movw %0, %%ax\n" : : "i" (CPUSEL));
//    __asm__ __volatile__ ("movw %ax, %gs\n");

    return;
}

