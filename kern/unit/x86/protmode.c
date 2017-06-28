#include <kern/conf.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/types.h>
#include <zero/trix.h>
#include <kern/util.h>
#include <kern/obj.h>
#include <kern/sched.h>
#include <kern/cpu.h>
#include <kern/proc/proc.h>
#include <kern/mem/vm.h>
#include <kern/mem/page.h>
#include <kern/io/drv/chr/cons.h>
//#include <kern/task.h>
#include <kern/io/drv/pc/dma.h>
#include <kern/io/drv/pc/vga.h>
#if (ACPI)
#include <kern/io/drv/pc/acpi.h>
#endif
#include <kern/unit/x86/boot.h>
#include <kern/unit/x86/pic.h>
#include <kern/unit/x86/pit.h>
#include <kern/unit/x86/kern.h>
#include <kern/unit/x86/link.h>
#if (SMP) || (APIC)
#include <kern/unit/ia32/mp.h>
#if (APIC)
#include <kern/unit/x86/apic.h>
#endif
#endif
#if (IOAPIC)
#include <kern/unit/x86/ioapic.h>
#endif
#if (VBE)
#include <kern/unit/x86/trap.h>
#endif
//#include <kern/unit/x86/asm.h>

extern uint8_t                   kerniomap[8192] ALIGNED(PAGESIZE);
extern uint8_t                   kernsysstktab[NCPU * KERNSTKSIZE];
extern uint8_t                   kernusrstktab[NCPU * KERNSTKSIZE];
extern volatile struct cpu       cputab[NCPU];
extern struct proc               proctab[NTASK];
#if (VBE)
extern uint64_t                  kernidt[NINTR];
extern long                      vbefontw;
extern long                      vbefonth;
#endif
extern volatile long             mpncpu;
extern struct vmpagestat         vmpagestat;
#if (SMP)
#if (ACPI)
extern volatile struct acpidesc *acpidesc;
#endif
#endif
extern struct pageq              vmphysq;
extern struct pageq              vmshmq;

FASTCALL
void
kinitprot(unsigned long pmemsz)
{
    uint32_t lim = min(pmemsz, KERNVIRTBASE);
    uint32_t sp = (uint32_t)kernusrstktab + NCPU * KERNSTKSIZE;

    /* initialise virtual memory */
    vminit((uint32_t *)&_pagetab);
//    vminit((uint32_t *)&_pagetab);
//    schedinit();
    /* zero kernel BSS segment */
    kbzero(&_bssvirt, (uint32_t)&_ebssvirt - (uint32_t)&_bssvirt);
    /* set kernel I/O permission bitmap to all 1-bits */
    kmemset(kerniomap, 0xff, sizeof(kerniomap));
    /* INITIALIZE CONSOLES AND SCREEN */
    /* TODO: use memory map from GRUB? */
    vminitphys((uintptr_t)&_epagetab, lim - (unsigned long)&_epagetab);
    __asm__ __volatile__ ("movl %0, %%esp\n"
                          "pushl %%ebp\n"
                          "movl %%esp, %%ebp\n"
                          :
                          : "rm" (sp));
    meminit(min(pmemsz, lim), min(KERNVIRTBASE, lim));
    cpuinit(0);
    procinit(PROCKERN, SCHEDNOCLASS);
    taskinitenv();
    tssinit(0);
#if (VBE)
    vbeinitscr();
#endif
#if (VBE) && (NEWFONT)
    consinit(768 / vbefontw, 1024 / vbefonth);
#elif (VBE)
    consinit(768 >> 3, 1024 >> 3);
#endif
#if (PLASMA)
    plasmainit();
#if (!PLASMAFOREVER)
    plasmaloop(1);
#endif
#endif
#if (PS2DRV)
    ps2init();
#endif
#if (SMP) || (APIC)
//#if (SMP)
    /* multiprocessor initialisation */
    mpinit();
//#endif
    if (mpncpu == 1) {
        kprintf("found %ld processor\n", mpncpu);
    } else {
        kprintf("found %ld processors\n", mpncpu);
    }
#if (APIC)
    apicinit();
#endif
#if (IOAPIC)
    ioapicinit();
#endif
#if 0
    if (mpmultiproc) {
        mpstart();
    }
#endif
#endif /* SMP || APIC */
//    procinit(PROCKERN, SCHEDNOCLASS);
    kprintf("%lu free physical pages @ 0x%p..0x%p\n",
            vmpagestat.nphys, vmpagestat.phys, vmpagestat.physend);
#if (SMBIOS)
    smbiosinit();
#endif
#if (VBE) && 0
    vbeprintinfo();
#endif
    logoprint();
//    vminitphys((uintptr_t)&_ebss, pmemsz - (unsigned long)&_ebss);
    /* HID devices */
#if (ATA)
    /* initialise ATA driver */
    atainit();
#endif
#if (PCI)
    /* initialise PCI bus driver */
    pciinit();
#endif
#if (AC97)
    /* initialise AC97 audio driver */
    ac97init();
#endif
#if (SB16)
    /* initialise Soundblaster 16 driver */
    sb16init();
#endif
#if (ACPI)
    /* initialise ACPI subsystem */
    acpiinit();
#endif
#if (IOBUF)
    /* initialise block I/O buffer cache */
    if (!ioinitbuf()) {
        kprintf("failed to allocate buffer cache\n");

        while (1) {
            ;
        }
    }
#if 0
    kprintf("%lu kilobytes of buffer cache @ %p..%p\n",
            vmpagestat.nbuf << (PAGESIZELOG2 - 10),
            vmpagestat.buf, vmpagestat.bufend);
#endif
    /* allocate unused device regions (in 3.5G..4G) */
//    pageaddzone(DEVMEMBASE, &vmshmq, 0xffffffffU - DEVMEMBASE + 1);
//    taskinitenv();
//    tssinit(0);
//    machinit();
    /* execution environment */
    kprintf("DMA buffers (%ul x %ul kilobytes) @ 0x%p\n",
            DMANCHAN, DMACHANBUFSIZE >> 10, DMABUFBASE);
    kprintf("VM page tables @ 0x%p\n", (unsigned long)&_pagetab);
//    kprintf("%ld kilobytes physical memory\n", pmemsz >> 10);
    kprintf("%ld kilobytes kernel memory\n", (uint32_t)&_ebss >> 10);
    kprintf("%ld kilobytes allocated physical memory (%ld wired, %ld total)\n",
            ((vmpagestat.nwire + vmpagestat.nmap + vmpagestat.nbuf)
             << (PAGESIZELOG2 - 10)),
            vmpagestat.nwire << (PAGESIZELOG2 - 10),
            vmpagestat.nphys << (PAGESIZELOG2 - 10));
#endif
    sysinit();
    schedinit();
#if (HPET)
    /* initialise high precision event timers */
    hpetinit();
#endif
#if (PLASMA) && 0
    plasmainit();
#endif
#if (APIC)
    apicstarttmr();
#else
    pitinit();
#endif
//    usrinit(&_usysinfo);
#if (PLASMAFOREVER)
    plasmaloop(-1);
#elif (USERMODE)
    m_jmpusr(0, schedloop);
#else    
    schedloop();
#endif

    /* NOTREACHED */
}

