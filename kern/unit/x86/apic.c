#include <kern/conf.h>

#if (SMP) || (APIC)

#include <stdint.h>
#include <sys/io.h>
#include <zero/trix.h>
#include <kern/util.h>
#include <kern/unit/x86/asm.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/x86/apic.h>
#include <kern/unit/ia32/link.h>
#include <kern/unit/ia32/vm.h>

extern void                    irqtmr(void);
extern void                    irqtmrcnt(void);
extern void                  (*irqerror)(void);
extern void                  (*irqspurious)(void);
extern void                  (*mpspurint)(void);
extern uint64_t                kernidt[NINTR];
extern void                   *irqvec[];
extern volatile struct m_cpu   cputab[NCPU];
extern volatile uint32_t      *mpapic;
extern volatile struct m_cpu  *mpbootcpu;

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
    long adr = k_readmsr(APICMSR);

    adr &= 0xfffff000;
    kprintf("local APIC @ %lx\n", adr);

    return (uint32_t *)adr;
}

void
apicstarttmr(uint32_t tmrcnt)
{
    /*
     * timer counts down at bus frequency from APICTMRINITCNT and issues
     * the interrupt IRQAPICTMR
     */
    apicwrite(tmrcnt, APICTMRINITCNT);
    apicwrite(0x03, APICTMRDIVCONF);
    apicwrite(IRQAPICTMR | APICPERIODIC, APICTMR);
    apicwrite(0, APICTASKPRIO);

    return;
}

#if (NEWTMR)
uint32_t
#else
void
#endif
apicinittmr(void)
{
//    struct apic *apic = apicprobe();
    volatile uint32_t *apic = mpapic;
    uint32_t           freq;
    uint32_t           tmrcnt;
    uint8_t            tmp8;

    if (!apic) {
        apic = apicprobe();
        mpapic = apic;
    }
    trapsetintrgate(&kernidt[trapirqid(IRQTMR)], irqtmrcnt, TRAPUSER);
    /* initialise APIC */
    apicwrite(0xffffffff, APICDFR);
    apicwrite((apicread(APICLDR) & 0x00ffffff) | 1, APICLDR);
#if 0
    apicwrite(APICMASKED, APICTMR);
//    apicwrite(APICNMI, APICPERFINTR);
    apicwrite(APICMASKED, APICLINTR0);
    apicwrite(APICMASKED, APICLINTR1);
#endif
    /* enable APIC */
    apicwrite((uint32_t)apic | APICGLOBENABLE, APICSPURIOUS);
    apicwrite(APICSWENABLE | (uint32_t)apic, APICSPURIOUS);
    apicwrite(IRQAPICTMR, APICTMR);
    apicwrite(0x03, APICTMRDIVCONF);
    /* initialise PIT channel 2 in one-shot mode */
    outb((inb(PITCTRL2) & 0xfd) | PITONESHOT, PITCTRL2);
    outb(0xb2, PITCTRL);
    apicwrite(0, APICTASKPRIO);
    pitsethz(100, PITCHAN2);
    /* wait until PIT  counter reaches zero */
    while (!(inb(PITCTRL2) & 0x20)) {
        ;
    }
    /* stop APIC timer */
    apicwrite(APICMASKED, APICTMR);
    /* calculate APIC interrupt frequency */
    tmp8 = inb(PITCTRL2) & 0xfe;
    outb(tmp8, PITCTRL2);
    outb(tmp8 | 1, PITCTRL2);

//    freq = (0xffffffff - apicread(APICTMRCURCNT) + 1) * 16 * 100;
    freq = (0xffffffff - apicread(APICTMRCURCNT) + 1) * 16 * 100;
    kprintf("APIC interrupt frequency: %ld MHz\n", freq / 1000000);
    tmrcnt = freq / HZ / 16;

    trapsetintrgate(&kernidt[trapirqid(IRQTMR)], irqtmr, TRAPUSER);

    tmrcnt = max(tmrcnt, 16);
#if !(NEWTMR)
    /*
     * timer counts down at bus frequency from APICTMRINITCNT and issues
     * the interrupt IRQAPICTMR
     */
    apicwrite(tmrcnt, APICTMRINITCNT);
    apicwrite(0x03, APICTMRDIVCONF);
    apicwrite(IRQAPICTMR | APICPERIODIC, APICTMR);

    return;
#else

    return tmrcnt;
#endif
}

#if (NEWTMR)
uint32_t
#else
void
#endif
apicinitcpu(long id)
{
//    uint64_t              *idt = kernidt;
    static long            first = 1;
    volatile struct m_cpu *cpu = &cputab[id];
#if (NEWTMR)
    uint32_t               tmrcnt;
#endif

    if (!mpapic) {
        mpapic = (volatile uint32_t *)apicprobe();

        return 0;
    }
    if (!mpapic) {

        return 0;
    }

    if (first) {
        /* identity-map APIC to kernel virtual address space */
        first = 0;
        kprintf("local APIC @ 0x%p\n", mpapic);
        /* identity-map MP APIC table */
        kprintf("APIC: map MP APIC (%l bytes) @ 0x%lx\n",
                PAGESIZE, (long)mpapic);
        vmmapseg((uint32_t *)&_pagetab, (uint32_t)mpapic, (uint32_t)mpapic,
                 (uint32_t)((uint8_t *)mpapic + PAGESIZE),
                 PAGEPRES | PAGEWRITE | PAGENOCACHE);
        irqvec[IRQERROR] = irqerror;
        irqvec[IRQSPURIOUS] = irqspurious;
    }
    cpu->id = id;
    /* enable local APIC; set spurious interrupt vector */
    apicwrite(APICSWENABLE | IRQSPURIOUS, APICSPURIOUS);
    /* initialise timer, mask interrupts */
    apicwrite(APICBASEDIV, APICTMRDIVCONF);
//    apicwrite(APICPERIODIC | IRQAPICTMR, APICTMR);
    apicwrite(IRQAPICTMR, APICTMR);
    apicwrite(10000000, APICTMRINITCNT);
    apicwrite(APICMASKED, APICTMR);
    /* disable logical interrupt lines */
    apicwrite(APICMASKED, APICLINTR0);
    apicwrite(APICMASKED, APICLINTR1);
    /* disable performance counter overflow interrupts */
    if (((apicread(APICVER) >> 16) & 0xff) >= 4) {
        apicwrite(APICMASKED, APICPERFINTR);
    }
    /* map error interrupt to IRQERROR */
    apicwrite(IRQERROR, APICERROR);
    /* clear error status registers */
    apicwrite(0, APICERRSTAT);
    apicwrite(0, APICERRSTAT);
    /* acknowledge outstanding interrupts */
    apicwrite(0, APICEOI);
    if (cpu != mpbootcpu) {
        /* send init level deassert to synchronise arbitration IDs */
        apicsendirq(0, APICBCAST | APICINIT | APICLEVEL, 0);
        while (apicread(APICINTRLO) & APICDELIVS) {
            ;
        }
    }
#if (!NEWTMR)
    apicinittmr();
    /* enable APIC (but not CPU) interrupts */
//    apicwrite(0, APICTASKPRIO);

    return;
#else
    tmrcnt = apicinittmr();

    return tmrcnt;
#endif
}

void
apicstart(uint8_t id, uint32_t adr)
{
    uint16_t *warmreset = (uint16_t *)BIOSWRV;

    outb(0x0f, RTCBASE);
    outb(0x0a, RTCBASE + 1);
    warmreset[0] = 0;
    warmreset[1] = adr >> 4;

    /* INIT IPI */
    apicsendirq(id << 24, APICINIT | APICLEVEL | APICASSERT, 200);
    while (apicread(APICINTRLO) & APICDELIVS) {
        ;
    }
    apicsendirq(id << 24, APICINIT | APICLEVEL, 0);
    while (apicread(APICINTRLO) & APICDELIVS) {
        ;
    }
    apicsendirq(id << 24, APICASSERT | APICSTART | adr >> 12, 200);
    apicsendirq(id << 24, APICASSERT | APICSTART | adr >> 12, 200);

    return;
}

#endif /* SMP || APIC */

