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
#include <kern/io/drv/chr/cons.h>
//#include <kern/thr.h>
#include <kern/io/drv/pc/dma.h>
#include <kern/io/drv/pc/vga.h>
#if (ACPI)
#include <kern/io/drv/pc/acpi.h>
#endif
#if (VBE)
#include <kern/io/drv/pc/vbe.h>
#endif
#include <kern/unit/x86/cpu.h>
#include <kern/unit/x86/pit.h>
#if (VBE)
#include <kern/unit/x86/trap.h>
#endif
#include <kern/unit/ia32/kern.h>
#include <kern/unit/ia32/link.h>
#include <kern/unit/ia32/boot.h>
#include <kern/unit/ia32/vm.h>
#if (SMP)
#include <kern/unit/ia32/mp.h>
#endif
#include <kern/unit/x86/asm.h>

extern long bufinit(void);

#if (HPET)
extern void hpetinit(void);
#endif
#if (PS2DRV)
extern void ps2init(void);
#endif
#if (SMP)
extern void mpstart(void);
#endif
#if (VBE2)
extern long vbe2init(struct mboothdr *hdr);
#endif
#if (VBE)
extern void idtinit(uint64_t *idt);
extern void vbeinit(void);
extern void vbeinitscr(void);
#if (PLASMA)
extern void plasmaloop(void);
#endif
#endif
#if (PCI)
extern void pciinit(void);
#endif
#if (ACPI)
extern void acpiinit(void);
#endif
#if (SB16)
extern void sb16init(void);
#endif

extern uint8_t                   kerniomap[8192] ALIGNED(PAGESIZE);
extern struct proc               proctab[NPROC];
extern struct m_cpu              mpcputab[NCPU];
#if (VBE)
extern uint64_t                  kernidt[NINTR];
#endif
extern struct vmpagestat         vmpagestat;
#if (SMP)
extern struct m_cpu              cputab[NCPU];
#if (ACPI)
extern volatile struct acpidesc *acpidesc;
#endif
extern volatile uint32_t        *mpapic;
extern volatile long             mpncpu;
extern volatile long             mpmultiproc;
#endif

ASMLINK
void
kmain(struct mboothdr *hdr, unsigned long pmemsz)
{
    seginit(0);                         // memory segments
#if (VBE)
//    idtinit(kernidt);
    vbeinit();
    trapinit();
#endif
    vminit((uint32_t *)&_pagetab);      // virtual memory
    kbzero(&_bssvirt, (uint32_t)&_ebss - (uint32_t)&_bss);
    consinit(768 >> 3, 1024 >> 3);
#if (VBE)
    vbeinitscr();
#endif
    curproc = &proctab[0];
    /* TODO: use memory map from GRUB */
    meminit(vmlinkadr(&_ebssvirt), pmemsz);
    vminitphys((uintptr_t)&_ebss, pmemsz - (unsigned long)&_ebss);
    kmemset(&kerniomap, 0xff, sizeof(kerniomap));
#if (VBE2)
    vbe2init(hdr);
    vbe2kludge();
#elif (VBE) && (PLASMA)
    plasmaloop();
#endif
    logoprint();
#if (PCI)
    pciinit();
#endif
#if (SB16)
    sb16init();
#endif
#if (ACPI)
    acpiinit();
#endif
#if (ACPI) && 0
    if (acpidesc) {
        kprintf("ACPI: RSDP found @ 0x%p\n", acpidesc);
    } else {
        kprintf("ACPI: RSDP not found\n");
    }
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
        mpstart();
    }
    if (mpncpu == 1) {
        kprintf("found %ld processor\n", mpncpu);
    } else {
        kprintf("found %ld processors\n", mpncpu);
    }
#if 0
    if (mpapic) {
        kprintf("local APIC @ 0x%p\n", mpapic);
    }
#endif
    curcpu = &cputab[0];
#endif
#if (HPET)
    hpetinit();
#endif
    /* CPU interface */
    taskinit();
    tssinit(0);
#if 0
    machinit();
#endif
    /* HID devices */
#if (PS2DRV)
    ps2init();
#endif
    /* execution environment */
    procinit(0);
    curthr = curproc->thr;
//    sysinit();
    kprintf("DMA buffers (%ul x %ul kilobytes) @ 0x%p\n",
            DMANCHAN, DMACHANBUFSIZE >> 10, DMABUFBASE);
    kprintf("VM page tables @ 0x%p\n", (unsigned long)&_pagetab);
//    kprintf("%ld kilobytes physical memory\n", pmemsz >> 10);
    kprintf("%ld kilobytes kernel memory\n", (uint32_t)&_ebss >> 10);
    kprintf("%ld kilobytes allocated physical memory (%ld wired, %ld total)\n",
            (vmpagestat.nwired + vmpagestat.nmapped + vmpagestat.nbuf) << (PAGESIZELOG2 - 10),
            vmpagestat.nwired << (PAGESIZELOG2 - 10),
            vmpagestat.nphys << (PAGESIZELOG2 - 10));
    schedinit();
    pitinit();
    /* pseudo-scheduler loop; interrupted by timer [and other] interrupts */
    while (1) {
        k_waitint();
    }
}

