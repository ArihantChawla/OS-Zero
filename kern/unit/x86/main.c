#include <kern/conf.h>
#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/types.h>
#include <zero/trix.h>
#include <kern/util.h>
#include <kern/obj.h>
#include <kern/proc/proc.h>
#include <kern/proc/sched.h>
//#include <kern/thr.h>
#include <kern/io/drv/pc/vga.h>
#if (VBE)
#include <kern/unit/ia32/vbe.h>
#endif
#include <kern/unit/x86/cpu.h>
#include <kern/unit/x86/pit.h>
#include <kern/unit/x86/dma.h>
#include <kern/unit/ia32/kern.h>
#include <kern/unit/ia32/link.h>
#include <kern/unit/ia32/boot.h>
#include <kern/unit/ia32/vm.h>
#if (SMP)
#include <kern/unit/ia32/mp.h>
#endif

extern long bufinit(void);

#if (HPET)
extern void hpetinit(void);
#endif
#if (SMP)
extern void mpstart(void);
#endif
#if (VBE2)
extern long vbe2init(struct mboothdr *hdr);
#endif
#if (VBE)
extern void vbeinit(void);
extern void vbeinitscr(void);
#endif

extern uint8_t            kerniomap[8192] ALIGNED(PAGESIZE);
extern struct proc        proctab[NPROC];
extern struct m_cpu       mpcputab[NCPU];
extern struct vmpagestat  vmpagestat;
#if (SMP)
extern struct m_cpu       cputab[NCPU];
extern volatile uint32_t *mpapic;
extern volatile long      mpncpu;
extern volatile long      mpmultiproc;
#endif

#if (VBE2) || (VBE)
void
vbe2kludge(void)
{
    kmemset(vbescreen.fbuf, 0xff, vbescreen.w * vbescreen.h * 3);
}
#endif

ASMLINK
void
kmain(struct mboothdr *hdr, unsigned long pmemsz)
{
    seginit(0);                         // memory segments
#if (VBE)
    vbeinit();
    trapinit();
#endif
    vminit((uint32_t *)&_pagetab);      // virtual memory
    kbzero(&_bssvirt, (uint32_t)&_ebss - (uint32_t)&_bss);
#if (VBE)
//    vbeinit();
    vbeinitscr();
    trapinit();
#endif
//    __asm__ __volatile__ ("sti\n");
    curproc = &proctab[0];
    meminit(vmphysadr(&_ebssvirt), pmemsz);
    /* TODO: use memory map from GRUB */
    vminitphys((uintptr_t)&_ebss, pmemsz - (unsigned long)&_ebss);
//    meminit(vmphysadr(&_ebssvirt), max(pmemsz, 3UL * 1024 * 1024 * 1024));
    kmemset(&kerniomap, 0xff, sizeof(kerniomap));
//    vgainitcon(80, 25);
    vgainitcon(80, 25);
#if (VBE2)
    vbe2init(hdr);
    vbe2kludge();
#elif (VBE)
    vbe2kludge();
//    vbeprintinfo();
#endif
    if (!bufinit()) {
        kprintf("failed to allocate buffer cache\n");

        while (1) {
            ;
        }
    }
#if (SMP)
    /* multiprocessor probe */
    mpinit();
    if (mpmultiproc) {
//        mpstart();
    }
    if (mpncpu == 1) {
        kprintf("found %ld processor\n", mpncpu);
    } else {
        kprintf("found %ld processors\n", mpncpu);
    }
    if (mpapic) {
        kprintf("local APIC @ 0x%p\n", mpapic);
    }
    curcpu = &cputab[0];
#endif
#if (HPET)
    hpetinit();
#endif
#if 0
#if (VBE2)
    vbe2init(hdr);
    vbe2kludge();
#endif
#endif
#if (AC97)
    ac97init();
#endif
//    vgainitcon(80, 25);
#if (!VBE2)
    logoprint();
#endif
    /* CPU interface */
    taskinit();
    tssinit(0);                        // initialize CPU TSS
#if 0
    machinit();                         // initialise machine
#endif
    /* HID devices */
    kbdinit();
    mouseinit();
    /* execution environment */
    procinit(0);
    curthr = curproc->thr;
//    sysinit();
    kprintf("DMA buffers (%ul x %ul kilobytes) @ 0x%p\n",
            DMANCHAN, DMACHANBUFSIZE >> 10, DMABUFBASE);
    kprintf("VM page tables @ 0x%p\n", (unsigned long)&_pagetab);
    kprintf("%ld kilobytes physical memory\n", pmemsz >> 10);
    kprintf("%ld kilobytes kernel memory\n", (uint32_t)&_ebss >> 10);
    kprintf("%ld kilobytes allocated physical memory (%ld wired, %ld total)\n",
            (vmpagestat.nwired + vmpagestat.nmapped + vmpagestat.nbuf) << (PAGESIZELOG2 - 10),
            vmpagestat.nwired << (PAGESIZELOG2 - 10),
            vmpagestat.nphys << (PAGESIZELOG2 - 10));
    schedinit();
    pitinit();
    /* pseudo-scheduler loop; interrupted by timer [and other] interrupts */
    while (1) {
        m_waitint();
    }
}

