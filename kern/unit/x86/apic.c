#include <kern/conf.h>

#if (SMP) || (APIC)

#include <stdint.h>
#include <sys/io.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/x86/apic.h>
#include <kern/unit/ia32/link.h>
#include <kern/unit/ia32/vm.h>
#if 0
#include <kern/unit/ia32/pit.h>
#endif

extern void                    irqtimer(void);
extern void                  (*irqerror)(void);
extern void                  (*irqspurious)(void);
extern void                  (*mpspurint)(void);
extern uint64_t                kernidt[NINTR];
extern void                   *irqvec[];
extern volatile uint32_t      *mpapic;
extern volatile struct m_cpu   mpcputab[NCPU];
extern volatile struct m_cpu  *mpbootcpu;

void
usleep(unsigned long nusec)
{
    nusec <<= 16;
    while (nusec--) {
        ;
    }
}

void
apicinit(long id)
{
    uint64_t              *idt = kernidt;
    static long            first = 1;
    volatile struct m_cpu *cpu = &mpcputab[id];

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
    cpu->id = id;
    /* enable local APIC; set spurious interrupt vector */
    apicwrite(APICENABLE | (IRQTIMER + IRQSPURIOUS), APICSPURIOUS);
    /*
     * timer counts down at bus frequency from APICTMRINITCNT and issues
     * the interrupt IRQBASE + IRQTIMER
     */
    apicwrite(APICBASEDIV, APICTMRDIVCONF);
    apicwrite(APICPERIODIC | (IRQBASE + IRQTIMER), APICTIMER);
//    apicwrite(10000000, APICTMRINITCNT);
    apicwrite(1000, APICTMRINITCNT);
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
    if (flg & APICONESHOT) {
        /* one-shot timer */
    } else {
        /* periodic timer */
    }
}

#endif /* SMP || APIC */

