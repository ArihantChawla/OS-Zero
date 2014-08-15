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
#if 0
#include <kern/unit/ia32/pit.h>
#endif

extern void                    irqtimer(void);
extern void                    irqtimercnt(void);
extern void                  (*irqerror)(void);
extern void                  (*irqspurious)(void);
extern void                  (*mpspurint)(void);
extern uint64_t                kernidt[NINTR];
extern void                   *irqvec[];
extern volatile uint32_t      *mpapic;
extern volatile struct m_cpu   mpcputab[NCPU];
extern volatile struct m_cpu  *mpbootcpu;
extern uint64_t                kernidt[NINTR];

/* TODO: fix this kludge */
void
usleep(unsigned long nusec)
{
    nusec <<= 16;
    while (nusec--) {
        ;
    }
}

struct apic *
apicprobe(void)
{
    long adr = k_readmsr(APICMSR);

    adr &= 0xfffff000;
    kprintf("local APIC @ %lx\n", adr);

    return (struct apic *)adr;
}

void
apicinit(void)
{
//    struct apic *apic = apicprobe();
#if (SMP)
    volatile struct apic *apic = (struct apic *)mpapic;
#else
    volatile struct apic *apic = apicprobe();
#endif
    uint32_t     freq;
    uint32_t     tmp;
    uint8_t      tmp8;

    trapsetintgate(&kernidt[trapirqid(IRQTIMER)], irqtimercnt, TRAPUSER);

    /* initialise APIC */
#if 0
   apic->destfmt.model = 0x0f;
    apic->logdest.val = 0x01;
    apic->lvttmr.mask = 1;
    apic->lvttmr.vec = IRQAPICTMR;
    apic->lvttherm.mask = 1;
    apic->lvtlint0.mask = 1;
    apic->lvtlint1.mask = 1;
    apic->lvterror.mask = 1;
    apic->spurint.vec = IRQSPURIOUS;
    apic->spurint.on = 1;
    apic->spurint.eoibcs = 1;
    apic->taskprio.lvl = 0;
    apic->divconf.div = 0x03;
    apic->tmrinit.cnt = 0xffffffffU;
#endif
    apicwrite(0xffffffff, APICDFR);
    apicwrite((apicread(APICLDR) & 0x00ffffff) | 1, APICLDR);
    apicwrite(APICDISABLE, APICTIMER);
    apicwrite(APICNMI, APICPERFINTR);
    apicwrite(APICDISABLE, APICLINTR0);
    apicwrite(APICDISABLE, APICLINTR1);
    apicwrite(0, APICTASKPRIO);

    /* enable APIC */
    k_writemsr((uint32_t)apic | APICGLOBENABLE, APICMSR);
    apicwrite(APICSWENABLE | (uint32_t)apic, APICSPURIOUS);
    apicwrite(IRQAPICTMR, APICTIMER);
    apicwrite(0x03, APICTMRDIVCONF);

    /* initialise PIT channel 2 in one-shot mode */
    outb((inb(0x61) & 0xfd) | 1, 0x61);
    outb(0xb2, PITCTRL);
    outb(0x9b, 0x42);
    inb(0x60);
    outb(0x2e, 0x42);

    /* wait until PIT  counter reaches zero */
    while (!(inb(0x61) & 0x20)) {
        ;
    }

    /* stop APIC timer */
#if 0
    apic->lvttmr.mask = 1;
#endif
    apicwrite(APICDISABLE, APICTIMER);

    /* start counting */
    tmp8 = inb(0x64) & 0xfe;
    outb(tmp8, 0x61);
    outb(tmp8 | 1, 0x61);

//    freq = (0xffffffff - apic->tmrcnt.cur + 1) * 16 * 100;
    freq = (0xffffffff - apicread(APICTMRCURCNT) + 1) * 16 * 100;
    tmp = freq / HZ / 16;
    kprintf("APIC interrupt frequency: %ld\n", (long)freq);

    trapsetintgate(&kernidt[trapirqid(IRQTIMER)], irqtimer, TRAPUSER);

#if 0
    apic->tmrinit.cnt = max(tmp, 16);
    apic->divconf.div = 0x03;
    apic->lvttmr.mode = APICMODEPERIODIC;
    apic->lvttmr.mask = 0;
#endif
    tmp = max(tmp, 16);
    apicwrite(tmp, APICTMRINITCNT);
    apicwrite(IRQAPICTMR | APICPERIODIC, APICTIMER);
    apicwrite(0x03, APICTMRDIVCONF);

    return;
}

void
apicinitcpu(long id)
{
    uint64_t              *idt = kernidt;
    static long            first = 1;
    volatile struct m_cpu *cpu = &mpcputab[id];

    if (!mpapic) {
        mpapic = (volatile uint32_t *)apicprobe();

        return;
    }
    if (!mpapic) {

        return;
    }

    kprintf("initialising timer interrupt to %d Hz\n", HZ);
    trapsetintgate(&idt[trapirqid(IRQTIMER)], irqtimer, TRAPUSER);

    if (first) {
        first = 0;
        kprintf("local APIC @ 0x%p\n", mpapic);
        /* identity-map MP APIC table */
        kprintf("APIC: map MP APIC (%l bytes) @ 0x%lx\n",
                PAGESIZE, (long)mpapic);
        vmmapseg((uint32_t *)&_pagetab, (uint32_t)mpapic, (uint32_t)mpapic,
                 (uint32_t)((uint8_t *)mpapic + PAGESIZE),
                 PAGEPRES | PAGEWRITE);
        irqvec[IRQERROR] = irqerror;
        irqvec[IRQSPURIOUS] = irqspurious;
    }
    if (cpu->id != 0) {
        apicinit();
    }
    cpu->id = id;
    /* enable local APIC; set spurious interrupt vector */
    apicwrite(APICSWENABLE | (IRQTIMER + IRQSPURIOUS), APICSPURIOUS);
    /*
     * timer counts down at bus frequency from APICTMRINITCNT and issues
     * the interrupt IRQBASE + IRQTIMER
     */
    apicwrite(APICBASEDIV, APICTMRDIVCONF);
    apicwrite(APICPERIODIC | (IRQBASE + IRQTIMER), APICTIMER);
//    apicwrite(10000000, APICTMRINITCNT);
    apicwrite(10000000, APICTMRINITCNT);
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
    /* send init level deassert to synchronise arbitration IDs */
    apicsendirq(0, APICBCAST | APICINIT | APICLEVEL, 0);
    while (apicread(APICINTRLO) & APICDELIVS) {
        ;
    }
    /* enable APIC (but not CPU) interrupts */
    apicwrite(0, APICTASKPRIO);

    return;
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

/* TODO */

/* set APIC interrupt frequency */
void
apicsettmr(long hz, long flg)
{
    if (flg & APICPERIODIC) {
        /* periodic timer */
    } else {
        /* one-shot timer */
    }
}

#endif /* SMP || APIC */

