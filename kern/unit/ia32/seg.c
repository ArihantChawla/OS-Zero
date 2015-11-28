#include <kern/conf.h>
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

#if (REENTRANTGDTINIT)
extern FASTCALL void gdtinit(struct m_farptr *farptr);
#else
extern void gdtinit(void);
#endif

#if (SMP)
extern uint64_t        kerngdt[NCPU][NGDT];
#else
extern uint64_t        kerngdt[NGDT];
#endif
//extern struct m_tss    tsstab[NTHR];
extern struct m_cpu    cputab[NCPU];
#if (REENTRANTGDTINIT)
extern struct m_farptr gdtptrtab[NCPU];
#else
extern struct m_farptr gdtptr;
#endif

ASMLINK void
seginit(long id)
{
    struct m_cpu    *cpu = &cputab[id];
    uint64_t        *gdt;
#if (REENTRANTGDTINIT)
    struct m_farptr *farptr = &gdtptrtab[id];
#else
    struct m_farptr *farptr = &gdtptr;
#endif
    

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
    segsetdesc(&gdt[CPUSEG], &cpu->cpu, 4 * sizeof(void *) + sizeof(long),
               SEGCPU);
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

