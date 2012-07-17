#include <stddef.h>
#include <kern/util.h>
//#include <mach/mach.h>
#include <zero/param.h>
#include <kern/task.h>
#include <kern/mem.h>
#include <kern/ia32/cpu.h>
#include <kern/ia32/seg.h>
#include <kern/ia32/vm.h>
#include <kern/ia32/boot.h>

extern uint8_t      kerniomap[8192];
extern uint64_t     kerngdt[NTHR][NGDT];
//extern struct m_tss _kerntss[NCPU];

//extern void _tssinit(long);

struct m_tss tsstab[NTHR];

void dummy(void)
{
    ;
}

void
tssinit(long id)
{
    struct m_tss *tss;
    uint64_t     *gdt;
    uint32_t      sel;
    uint32_t      pdbr;

//    tss = cpu->ktss = (struct m_tss *)(MPTSSBASE + 4 * PAGESIZE * core);
    tss = &tsstab[id];
    __asm__ __volatile__ ("movl %%cr3, %0" : "=r" (pdbr));
    tss->ss0 = tss->ss1 = tss->ss2 = DATASEL;
    tss->esp0 = tss->esp1 = tss->esp2 = (uint32_t)kwalloc(THRSTKSIZE);
    bzero((void *)tss->esp0, THRSTKSIZE);
    tss->cr3 = pdbr;
    tss->iomapofs = (uint16_t)((uint8_t *)kerniomap - (uint8_t *)tss);
    gdt = &kerngdt[id][0];
    segsetdesc(&gdt[TSSSEG], (uintptr_t)tss, sizeof(struct m_tss),
               SEGTSS);
    sel = TSSSEL;
    __asm__ __volatile__ ("ltr %w0" : : "r" (sel));

    return;
}

