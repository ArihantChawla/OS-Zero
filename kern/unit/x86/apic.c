#include <kern/conf.h>

#if (APIC) || (SMP)

#include <stdint.h>
#include <sys/io.h>
#include <zero/trix.h>
#include <kern/asm.h>
#include <kern/util.h>
#include <kern/cpu.h>
#include <kern/mem/vm.h>
#include <kern/unit/x86/bios.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/x86/pic.h>
#include <kern/unit/x86/pit.h>
#include <kern/unit/x86/apic.h>
#include <kern/unit/x86/link.h>

extern void      irqtmr(void);
extern void      irqtmrcnt(void);
extern uint64_t  kernidt[NINTR];
extern void     *irqvec[NINTR];

volatile struct cpu *mpbootcpu;
volatile uint32_t   *mpapic;
static uint32_t      apicrate;

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
    uint64_t  val = APICBASE | APICENABLE;
    uint32_t *ptr;

    k_writemsr(APICMSR, val);
    val = k_readmsr(APICMSR);
    val &= 0xfffff000;
    ptr = (uint32_t *)(uint32_t)val;

    return ptr;
}

void
apicstarttmr(volatile uint32_t *base)
{
    uint32_t val;
    //    uint32_t spur = apicread(volatile uint32_t *base, APICSPURIOUS) | APICSWENABLE;

    apicwrite(base, 0, APICEOI);
    /*
     * timer counts down at bus frequency from apicrate and issues
     * the interrupt trapirqid(Irqtmr)
     */
    apicwrite(base, apicrate, APICTMRINITCNT);
    //    apicwrite(base, 0x03, APICTMRDIVCONF);
    //    apicwrite(base, trapirqid(IRQTMR) | APICPERIODIC, APICTMR);
    k_intron();

    return;
}

void
apicinittmr(volatile uint32_t *base)
{
    uint64_t cnt;
    uint32_t ntick;
    uint32_t rate;
    uint32_t freq;
    uint32_t tmrcnt;
    uint8_t  tmp8;

    /* initialise APIC */
#if 0
    apicwrite(base, 0xffffffff, APICDFR);
    apicwrite(base, (apicread(base, (APICLDR) & 0x00ffffff)) | 1, APICLDR);
#endif
    /* enable APIC */
    //    apicwrite((uint32_t)apic | APICENABLE, APICSPURIOUS);
    apicwrite(base, IRQSPURIOUS | APICSWENABLE, APICSPURIOUS);
    apicwrite(base, 0x03, APICTMRDIVCONF);
    /* initialise PIT channel 2 in one-shot mode, 100 Hz frequency */
    apicwrite(base, 0xffffffff, APICTMRINITCNT);
    trapsetintrgate(&kernidt[trapirqid(IRQTMR)], irqtmrcnt, TRAPUSER);
    tmp8 = inb(PITCTRL2) & 0xfd;
    tmp8 |= PITONESHOT;
    outb(tmp8, PITCTRL2);
    apicwrite(base, trapirqid(IRQTMR) | APICPERIODIC, APICTMR);
    pitsethz(100, PITCHAN2);
    while (!(inb(PITCTRL2) & 0x20)) {
        ;
    }
    k_introff();
    //    apicwrite(base, APICMASKED, APICTMR);
    tmp8 = inb(PITCTRL2) & 0xfe;
    outb(tmp8, PITCTRL2);
    outb(tmp8 | 1, PITCTRL2);
    /* number of ticks in 10 milliseconds */
    ntick = 0xffffffff - apicread(base, APICTMRCURCNT);
    ntick >> 2; // divide by 4 to get rate for 250 Hz
    rate = ntick / 250;
    kprintf("APIC ticks per slice: %lu\n", (unsigned long)rate);
//    kprintf("APIC interrupt frequency: %ld MHz\n", divu1000000(freq));
    trapsetintrgate(&kernidt[trapirqid(IRQTMR)], irqtmr, TRAPUSER);
    apicrate = rate;
    //    apicwrite(base, 0, APICEOI);

    return;
}

void
apicinit(long id)
{
    volatile uint32_t   *base;
    volatile struct cpu *cpu = &k_cputab[id];
    static long          first = 1;
    uint32_t             tmrcnt;

#if (!SMP)
    mpbootcpu = cpu;
#endif
    if (!mpapic) {
        mpapic = apicprobe();
    }
    if (!mpapic) {

        return;
    }
    base = mpapic;
    if (first) {
        /* identity-map APIC to kernel virtual address space */
        first = 0;
        kprintf("local APIC @ 0x%p\n", mpapic);
        /* identity-map MP APIC table */
        kprintf("APIC: map MP APIC (%ld bytes) @ 0x%lx\n",
                PAGESIZE, (long)mpapic);
        vmmapseg((uint32_t)mpapic, (uint32_t)mpapic,
                 (uint32_t)((uint8_t *)mpapic + PAGESIZE),
                 PAGEPRES | PAGEWRITE | PAGENOCACHE);
        irqvec[IRQERROR] = irqerror;
        irqvec[IRQSPURIOUS] = irqspurious;
    }
    /* enable local APIC; set spurious interrupt vector */
    apicwrite(base, IRQSPURIOUS | APICSWENABLE, APICSPURIOUS);
    /* initialise timer, mask interrupts, set divisor */
    apicwrite(base, 0x03, APICTMRDIVCONF);
    apicwrite(base, trapirqid(IRQTMR) | APICPERIODIC, APICTMR);
    apicwrite(base, 10000000, APICTMRINITCNT);
    apicwrite(base, APICMASKED, APICTMR);
    /* disable logical interrupt lines */
    apicwrite(base, APICMASKED, APICINTR0);
    apicwrite(base, APICMASKED, APICINTR1);
    /* disable performance counter overflow interrupts where available */
    if (apicread(base, (APICVER >> 16) & 0xff) >= 4) {
        apicwrite(base, APICMASKED, APICPERFINTR);
    }
    /* map error interrupt to IRQERROR */
    apicwrite(base, IRQERROR, APICERROR);
    /* clear error status registers */
    apicwrite(base, 0, APICERRSTAT);
    apicwrite(base, 0, APICERRSTAT);
    /* acknowledge outstanding interrupts */
    apicwrite(base, 0, APICEOI);
    if (cpu != mpbootcpu) {
        /* send init level deassert to synchronise arbitration IDs */
        apicsendirq(base, 0, APICBCAST | APICINIT | APICLEVEL, 0);
        while (apicread(base, APICINTRLO) & APICDELIVS) {
            ;
        }
    }
    apicinittmr(base);

    return;
}

void
apicstart(uint8_t cpuid, uint32_t adr)
{
    uint16_t *warmreset = (uint16_t *)BIOSWRV;

    outb(0x0f, RTCBASE);
    outb(0x0a, RTCBASE + 1);
    warmreset[0] = 0;
    warmreset[1] = adr >> 4;
    /* INIT IPI */
    apicsendirq(mpapic, cpuid << 24, APICINIT | APICLEVEL | APICASSERT, 200);
    while (apicread(mpapic, (APICINTRLO) & APICDELIVS)) {
        ;
    }
    apicsendirq(mpapic, cpuid << 24, APICINIT | APICLEVEL, 0);
    while (apicread(mpapic, APICINTRLO) & APICDELIVS) {
        ;
    }
    apicsendirq(mpapic, cpuid << 24, APICASSERT | APICSTART | adr >> 12, 200);
    apicsendirq(mpapic, cpuid << 24, APICASSERT | APICSTART | adr >> 12, 200);

    return;
}

#endif /* APIC || SMP */

