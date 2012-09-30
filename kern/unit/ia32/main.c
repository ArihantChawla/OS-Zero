#include <kern/conf.h>
#include <stdint.h>
#include <zero/param.h>
#include <zero/types.h>
#include <zero/trix.h>
#include <kern/util.h>
#include <kern/proc.h>
#include <kern/io/drv/pc/vga.h>
#include <kern/unit/ia32/kern.h>
#include <kern/unit/ia32/link.h>
#include <kern/unit/ia32/boot.h>
#include <kern/unit/ia32/cpu.h>
#include <kern/unit/ia32/thr.h>
#include <kern/unit/ia32/vm.h>
#include <kern/unit/ia32/pit.h>
#include <kern/unit/ia32/dma.h>

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
kmain(void)
{
    struct mboothdr *boothdr;
    unsigned long    pmemsz;

//    __asm__ __volatile__ ("sti\n");
    __asm__ __volatile__ ("cli\n");
    /* determine amount of RAM */
    /* boot.S leaves the multiboot header address in %ebx */
    __asm__ __volatile__ ("movl %%ebx, %0\n" : "=rm" (boothdr));
    /* multiprocessor probe */
    /* bootstrap kernel */
#if (SMP)
    mpinit();
    if (mpmultiproc) {
//        mpstart();
    }
#if 0
    if (mpncpu == 1) {
        kprintf("found %ld processor\n", mpncpu);
    } else {
        kprintf("found %ld processors\n", mpncpu);
    }
    if (mpapic) {
        kprintf("local APIC @ 0x%p\n", mpapic);
    }
#else
    curcpu = &cputab[0];
#endif
#endif
    trapinit();                         // interrupt management
    seginit(0);                         // memory segments
    vminit((uint32_t *)&_pagetab);      // virtual memory
    __asm__ __volatile__ ("sti\n");
    bzero(&_bssvirt, (uint32_t)&_ebss - (uint32_t)&_bss);
    curproc = &proctab[0];
    pmemsz = grubmemsz(boothdr);
//    meminit(vmphysadr(&_ebssvirt), pmemsz);
    meminit(vmphysadr(&_ebssvirt), max(pmemsz, 3584UL * 1024 * 1024));
    bfill(&kerniomap, 0xff, sizeof(kerniomap));
    /* consoles */
    vgainitcon(80, 25);
    logoprint();
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
    thrinit(0, SCHEDPRIO);
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
    /* pseudo-scheduler loop; interrupted by timer [and other] interrupts */
    kprintf("pitinit()\n");
    pitinit();
    while (1) {
        m_waitint();
    }
}

