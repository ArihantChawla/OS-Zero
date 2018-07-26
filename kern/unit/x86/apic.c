
#include <kern/conf.h>

#if (SMP) || (APIC)

#include <stdint.h>
#include <sys/io.h>
#if 0
#include <zero/trix.h>
#include <kern/asm.h>
#include <kern/util.h>
#include <kern/unit/x86/link.h>
#endif
#include <kern/cpu.h>
//#include <kern/unit/x86/cpu.h>
#include <kern/mem/vm.h>
#include <kern/unit/x86/asm.h>
#include <kern/unit/x86/bios.h>
#include <kern/unit/x86/pit.h>
#include <kern/unit/x86/apic.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/x86/mp.h>

extern void                  irqtmr(void);
extern void                  irqtmrcnt(void);
extern void                (*irqerror)(void);
extern void                (*irqspurious)(void);
extern void                (*mpspurint)(void);
extern uint64_t              kernidt[NINTR];
extern void                 *irqvec[];
static uint32_t     apictmrcnt;

/* TODO: fix this kludge */
void
kusleep(unsigned long nusec)
{
    nusec <<= 16;
    while (nusec--) {
        ;
    }
}

uint32_t *
apicprobe(void)
{
    unsigned long  adr = k_readmsr(APICMSR);
    uint32_t      *ptr = (uint32_t *)adr;

    adr &= 0xfffff000;
    kprintf("local APIC @ %lx\n", adr);

    return ptr;
}

void
apicstarttmr(void)
{
    /*
     * timer counts down at bus frequency from APICTMRINITCNT and issues
     * the interrupt IRQAPICTMR
     */
    apicwrite(k_mp.apic, apictmrcnt, APICTMRINITCNT);
    apicwrite(k_mp.apic, 0x03, APICTMRDIVCONF);
    apicwrite(k_mp.apic, IRQAPICTMR | APICPERIODIC, APICTMR);
    apicwrite(k_mp.apic, 0, APICTASKPRIO);

    return;
}

void
apicinittmr(void)
{
    uint32_t *volatile apic = k_mp.apic;
    uint32_t                  freq;
    uint32_t                  tmrcnt;
    uint8_t                   tmp8;

    if (!apic) {
        apic = apicprobe();
        apic = apic;
    }
    trapsetintrgate(&kernidt[trapirqid(IRQTMR)], irqtmrcnt, TRAPUSER);
    /* initialise APIC */
    apicwrite(apic, 0xffffffff, APICDFR);
    apicwrite(apic,
              (apicread(apic, APICLDR) & 0x00ffffff) | 1,
              APICLDR);
    /* enable APIC */
    apicwrite(apic, (uint32_t)apic | APICGLOBENABLE, APICSPURIOUS);
    apicwrite(apic, APICSWENABLE | (uint32_t)apic, APICSPURIOUS);
    apicwrite(apic, IRQAPICTMR, APICTMR);
    apicwrite(apic, 0x03, APICTMRDIVCONF);
    /* initialise PIT channel 2 in one-shot mode */
    //    outb((inb(PITCTRL2) & 0xfd) | PITONESHOT, PITCTRL2);

    //    outb(PITCMD | PITONESHOT, PITCTRL);
    //    pitsethz(100, PITCHAN0);

    tmp8 = inb(PITCTRL2) & 0xfd;
    tmp8 |= PITONESHOT;
    outb(tmp8, PITCTRL2);

    //    outb(0xb2, PITCTRL);
    //    outb(PITCMD | PITONESHOT, PITCTRL2);
    apicwrite(apic, 0, APICTASKPRIO);
    pitsethz(100, PITCHAN2);
    /* wait until PIT  counter reaches zero */
    while (!(inb(PITCTRL2) & 0x20)) {
        ;
    }
    /* stop APIC timer */
    apicwrite(apic, APICMASKED, APICTMR);
    /* calculate APIC interrupt frequency */
    tmp8 = inb(PITCTRL2) & 0xfe;
    outb(tmp8, PITCTRL2);
    outb(tmp8 | 1, PITCTRL2);
    freq = ((0xffffffff - apicread(apic, APICTMRCURCNT) + 1) * 100) << 4;
    //    kprintf("APIC interrupt frequency: %ld MHz\n", divu1000000(freq));
    kprintf("APIC interrupt frequency: %ld MHz\n", freq / 1000000);
    tmrcnt = (freq / HZ) >> 4;
    trapsetintrgate(&kernidt[trapirqid(IRQTMR)], irqtmr, TRAPUSER);
    tmrcnt = max(tmrcnt, 16);
    apictmrcnt = tmrcnt;

    return;
}

void
apicinit(long id)
{
    uint32_t *volatile   apic = k_mp.apic;
    volatile struct cpu *cpu = &k_cputab[id];
    static long          first = 1;
    uint32_t             tmrcnt;

    if (!apic) {
        apic = apicprobe();
        k_mp.apic = apic;

        return;
    }
    if (first) {
        /* identity-map APIC to kernel virtual address space */
        first = 0;
        kprintf("local APIC @ 0x%p\n", apic);
        /* identity-map MP APIC table */
        kprintf("APIC: map MP APIC (%l bytes) @ 0x%lx\n",
                PAGESIZE, (long)apic);
        vmmapseg((uint32_t)apic, (uint32_t)apic,
                 (uint32_t)((uint8_t *)apic + PAGESIZE),
                 PAGEPRES | PAGEWRITE | PAGENOCACHE);
        irqvec[IRQERROR] = irqerror;
        irqvec[IRQSPURIOUS] = irqspurious;
    }
    /* enable local APIC; set spurious interrupt vector */
    apicwrite(apic, APICSWENABLE | IRQSPURIOUS, APICSPURIOUS);
    /* initialise timer, mask interrupts */
    apicwrite(apic, APICBASEDIV, APICTMRDIVCONF);
    apicwrite(apic, IRQAPICTMR, APICTMR);
    apicwrite(apic, 10000000, APICTMRINITCNT);
    apicwrite(apic, APICMASKED, APICTMR);
    /* disable logical interrupt lines */
    apicwrite(apic, APICMASKED, APICLINTR0);
    apicwrite(apic, APICMASKED, APICLINTR1);
    /* disable performance counter overflow interrupts */
    if (((apicread(apic, APICVER) >> 16) & 0xff) >= 4) {
        apicwrite(apic, APICMASKED, APICPERFINTR);
    }
    /* map error interrupt to IRQERROR */
    apicwrite(apic, IRQERROR, APICERROR);
    /* clear error status registers */
    apicwrite(apic, 0, APICERRSTAT);
    apicwrite(apic, 0, APICERRSTAT);
    /* acknowledge outstanding interrupts */
    apicwrite(apic, 0, APICEOI);
    if (cpu != k_mp.bootcpu) {
        /* send init level deassert to synchronise arbitration IDs */
        apicsendirq(apic, 0, APICBCAST | APICINIT | APICLEVEL, 0);
        while (apicread(apic, APICINTRLO) & APICDELIVS) {
            ;
        }
    }
    apicinittmr();

    return;
}

void
apicstart(uint8_t cpuid, uint32_t adr)
{
    uint32_t *volatile  apic = k_mp.apic;
    uint16_t           *warmreset = (uint16_t *)BIOSWRV;

    outb(0x0f, RTCBASE);
    outb(0x0a, RTCBASE + 1);
    warmreset[0] = 0;
    warmreset[1] = adr >> 4;
    /* INIT IPI */
    apicsendirq(apic, cpuid << 24, APICINIT | APICLEVEL | APICASSERT, 200);
    while (apicread(apic, APICINTRLO) & APICDELIVS) {
        ;
    }
    apicsendirq(apic, cpuid << 24, APICINIT | APICLEVEL, 0);
    while (apicread(apic, APICINTRLO) & APICDELIVS) {
        ;
    }
    apicsendirq(apic, cpuid << 24, APICASSERT | APICSTART | adr >> 12, 200);
    apicsendirq(apic, cpuid << 24, APICASSERT | APICSTART | adr >> 12, 200);

    return;
}

#endif /* SMP || APIC */

