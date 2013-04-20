#include <kern/conf.h>
#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/types.h>
#include <zero/trix.h>
#include <kern/util.h>
#include <kern/proc.h>
#include <kern/thr.h>
#include <kern/io/drv/pc/vga.h>
#if (VBE2)
#include <kern/io/drv/pc/vbe2.h>
#endif
#include <kern/unit/x86/cpu.h>
#include <kern/unit/x86/pit.h>
#include <kern/unit/x86/dma.h>
#include <kern/unit/ia32/kern.h>
#include <kern/unit/ia32/link.h>
#include <kern/unit/ia32/boot.h>
#include <kern/unit/ia32/vm.h>

extern uint8_t            kerniomap[8192] ALIGNED(PAGESIZE);
extern struct proc        proctab[NPROC];
extern struct m_cpu       mpcputab[NCPU];
extern unsigned long      vmnphyspages;
extern unsigned long      vmnmappedpages;
extern unsigned long      vmnwiredpages;
extern unsigned long      vmnbufpages;
#if (SMP)
extern volatile uint32_t *mpapic;
extern volatile long      mpncpu;
extern volatile long      mpmultiproc;
#endif

ASMLINK
void
kmain(struct mboothdr *hdr, unsigned long pmemsz)
{
    seginit(0);                         // memory segments
    vminit((uint32_t *)&_pagetab);      // virtual memory
    kbzero(&_bssvirt, (uint32_t)&_ebss - (uint32_t)&_bss);
//    __asm__ __volatile__ ("sti\n");
    curproc = &proctab[0];
    vgainitcon(80, 25);
    meminit(vmphysadr(&_ebssvirt), pmemsz);
//    meminit(vmphysadr(&_ebssvirt), max(pmemsz, 3UL * 1024 * 1024 * 1024));
    kbfill(&kerniomap, 0xff, sizeof(kerniomap));
//    vgainitcon(80, 25);
#if (VBE2)
    vbe2init(hdr);
    {
        kbfill(vbe2screen.fbuf, 0xff, vbe2screen.w * vbe2screen.h * 3);
    }
#endif
#if (AC97)
    ac97init();
#endif
//    vgainitcon(80, 25);
#if (!VBE2)
    logoprint();
#endif
    /* multiprocessor probe */
    /* CPU interface */
    taskinit();
    tssinit(0);                        // initialize CPU TSS
#if 0
    machinit();                         // initialise machine
#endif
    /* HID devices */
    kprintf("kbdinit()\n");
    kbdinit();
    kprintf("mouseinit()\n");
    mouseinit();
    /* execution environment */
    kprintf("procinit()\n");
    procinit(0);
    kprintf("thrinit()\n");
    curthr = curproc->thr;
//    sysinit();
    kprintf("DMA buffers (%ul x %ul kilobytes) @ 0x%p\n",
            DMANCHAN, DMACHANBUFSIZE >> 10, DMABUFBASE);
    kprintf("VM page tables @ 0x%p\n", (unsigned long)&_pagetab);
    kprintf("%ld kilobytes physical memory\n", pmemsz >> 10);
    kprintf("%ld kilobytes kernel memory\n", (uint32_t)&_ebss >> 10);
    kprintf("%ld kilobytes allocated physical memory (%ld wired, %ld total)\n",
            (vmnwiredpages + vmnmappedpages + vmnbufpages) << (PAGESIZELOG2 - 10),
            vmnwiredpages << (PAGESIZELOG2 - 10),
            vmnphyspages << (PAGESIZELOG2 - 10));
    schedinit();
    kprintf("pitinit()\n");
    pitinit();
    /* pseudo-scheduler loop; interrupted by timer [and other] interrupts */
    while (1) {
        m_waitint();
    }
}

