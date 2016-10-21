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

#if (VBE)
extern void trapinitprot(void);
#endif
extern void cpuinit(long id);
extern long sysinit(long id);
extern long ioinitbuf(void);

#if (HPET)
extern void hpetinit(void);
#endif
#if (SMP)
extern void mpstart(void);
#endif
#if (VBE)
extern void idtinit(uint64_t *idt);
extern void vbeinitscr(void);
extern void vbeprintinfo(void);
#endif
#if (SMBIOS)
extern void smbiosinit(void);
#endif
#if (PS2DRV)
extern void ps2init(void);
#endif
#if (PLASMA)
extern void plasmaloop(long nsec);
#endif
#if (PCI)
extern void pciinit(void);
#endif
#if (ATA)
extern void atainit(void);
#endif
#if (AC97)
extern long ac97init(void);
#endif
#if (ACPI)
extern void acpiinit(void);
#endif
#if (SB16)
extern void sb16init(void);
#endif
#if (APIC)
extern void apicinit(long id);
extern void apicstarttmr(void);
#endif
extern void taskinitenv(void);
#if (USERMODE)
extern FASTCALL void m_jmpusr(long id, void *func);
#endif
extern void schedloop(void);

extern uint8_t                   kerniomap[8192] ALIGNED(PAGESIZE);
extern uint8_t                   kernsysstktab[NCPU * KERNSTKSIZE];
extern uint8_t                   kernusrstktab[NCPU * KERNSTKSIZE];
extern struct proc               proctab[NTASK];
extern struct cpu                cputab[NCPU];
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

void
kinitprot(unsigned long pmemsz)
{
    uint32_t lim = min(pmemsz, KERNVIRTBASE);
#if 0
    uint32_t sp = (uint32_t)kernusrstktab + KERNSTKSIZE;
#endif

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
#if (VBE)
    vbeinitscr();
#endif
#if (VBE) && (NEWFONT)
    consinit(768 / vbefontw, 1024 / vbefonth);
#elif (VBE)
    consinit(768 >> 3, 1024 >> 3);
#endif
    meminit(min(pmemsz, lim), min(KERNVIRTBASE, lim));
    tssinit(0);
    kprintf("%lu free physical pages @ 0x%p..0x%p\n",
            vmpagestat.nphys, vmpagestat.phys, vmpagestat.physend);
#if 0
    /* FIXME: map possible device memory */
    vmmapseg((uint32_t *)&_pagetab, DEVMEMBASE, DEVMEMBASE, 0xffffffffU,
             PAGEPRES | PAGEWRITE | PAGENOCACHE);
#endif
#if (SMBIOS)
    smbiosinit();
#endif
#if (VBE)
    vbeprintinfo();
#endif
    logoprint();
//    vminitphys((uintptr_t)&_ebss, pmemsz - (unsigned long)&_ebss);
#if (APIC)
    apicinit(0);
#endif
    /* HID devices */
#if (PCI)
    /* initialise PCI bus driver */
    pciinit();
#endif
#if (ATA)
    /* initialise ATA driver */
    atainit();
#endif
#if (SB16)
    /* initialise Soundblaster 16 driver */
    sb16init();
#endif
#if (AC97)
    /* initialise AC97 audio driver */
    ac97init();
#endif
#if (ACPI)
    /* initialise ACPI subsystem */
    acpiinit();
#endif
    /* initialise block I/O buffer cache */
    if (!ioinitbuf()) {
        kprintf("failed to allocate buffer cache\n");

        while (1) {
            ;
        }
    }
    kprintf("%lu kilobytes of buffer cache @ %p..%p\n",
            vmpagestat.nbuf << (PAGESIZELOG2 - 10),
            vmpagestat.buf, vmpagestat.bufend);
    /* allocate unused device regions (in 3.5G..4G) */
//    pageaddzone(DEVMEMBASE, &vmshmq, 0xffffffffU - DEVMEMBASE + 1);
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
    k_curunit = 0;
    /* CPU interface */
    cpuinit(0);
    sysinit(0);
    schedinit();
#if (HPET)
    /* initialise high precision event timers */
    hpetinit();
#endif
#if (IOAPIC)
    ioapicinit(0);
#endif
#endif /* SMP || APIC */
#if (SMP) && 0
    if (mpmultiproc) {
        mpstart();
    }
#endif
    taskinitenv();
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
#if (PS2DRV)
    ps2init();
#endif
    procinit(PROCKERN, SCHEDNOCLASS);
#if 0
    __asm__ __volatile__ ("movl %0, %%esp\n"
                          :
                          : "rm" (sp));
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

