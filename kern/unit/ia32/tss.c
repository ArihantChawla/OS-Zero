#include <stddef.h>
#include <kern/util.h>
//#include <mach/mach.h>
#include <zero/param.h>
#include <kern/malloc.h>
#include <kern/cpu.h>
#include <kern/mem/vm.h>
#include <kern/proc/task.h>
#include <kern/unit/x86/boot.h>
#include <kern/unit/ia32/seg.h>

uint8_t         kerntsstab[NCPU * TSSLEN];
extern uint8_t  kerniomap[8192];
extern uint64_t kerngdt[NCPU][NGDT];
/* FIXME: per-process kernel- and user-mode stacks */
extern uint8_t  kernsysstktab[NCPU * KERNSTKSIZE];
extern uint8_t  kernusrstktab[NCPU * KERNSTKSIZE];
//extern struct m_tss _kerntss[NCPU];

//extern void _tssinit(long);

void
tssinit(long id)
{
    struct m_tss *tss;
    uint64_t     *gdt;
    uint32_t      sel;
    uint32_t      pdbr;

//    tss = cpu->ktss = (struct m_tss *)(MPTSSBASE + 4 * PAGESIZE * core);
//    tss = &tsstab[id];
    tss = (struct m_tss *)(kerntsstab + id * TSSLEN);
    __asm__ __volatile__ ("movl %%cr3, %0" : "=r" (pdbr));
    tss->ss0 = tss->ss1 = tss->ss2 = DATASEL;
    tss->esp0 = (uint32_t)kernsysstktab + (NCPU - id) * KERNSTKSIZE;
    tss->cr3 = pdbr;
    tss->iomapofs = (uint16_t)((uint8_t *)kerniomap - (uint8_t *)tss);
    gdt = &kerngdt[id][0];
    segsetdesc(&gdt[TSSSEG], (uintptr_t)tss, sizeof(struct m_tss),
               SEGTSS);
    sel = TSSSEL;
    __asm__ __volatile__ ("ltr %w0" : : "r" (sel));

    return;
}

