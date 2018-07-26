#include <kern/conf.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <mach/param.h>
#include <mach/types.h>
#include <kern/conf.h>
#include <kern/cpu.h>
#include <kern/mem/vm.h>
#include <kern/unit/x86/boot.h>
#include <kern/unit/ia32/seg.h>

FASTCALL void gdtinit(struct m_farptr *farptr);

#if (SMP)
extern uint64_t            kerngdt[NCPU][NGDT];
#else
extern uint64_t            kerngdt[NGDT];
#endif
//extern struct m_tss    tsstab[NTHR];
extern struct m_farptr     gdtptrtab[NCPU];

ASMLINK void
seginit(long id)
{
    volatile struct cpu *cpu = &k_cputab[id];
    struct m_farptr     *farptr = &gdtptrtab[id];
    uint64_t            *gdt;

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
    segsetdesc(&gdt[UTEXTSEG], 0, NPAGEMAX - 1,
               SEGCODE | SEGUSER);
    segsetdesc(&gdt[UDATASEG], 0, NPAGEMAX - 1,
               SEGDATA | SEGUSER);
    /* per-CPU data segment */
    segsetdesc(&gdt[CPUSEG], cpu, sizeof(struct cpu), SEGCPU);
#if (VBE)
    gdt[REALCODESEG] = UINT64_C(0x00009a000000ffff);
    gdt[REALDATASEG] = UINT64_C(0x000092000000ffff);
#endif
    /* initialize segmentation */
    farptr->lim = NGDT * sizeof(uint64_t) - 1;
    farptr->adr = (uint32_t)gdt;
    gdtinit(farptr);

    return;
}

