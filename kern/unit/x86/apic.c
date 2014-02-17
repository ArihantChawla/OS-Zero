#include <kern/conf.h>

#if (SMP)

#include <stdint.h>
#include <sys/io.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/x86/apic.h>
#include <kern/unit/ia32/link.h>
#include <kern/unit/ia32/vm.h>
#if 0
#include <kern/unit/ia32/pit.h>
#endif

extern volatile uint32_t     *mpapic;
extern volatile struct m_cpu  mpcputab[NCPU];
extern volatile struct m_cpu *mpbootcpu;

void
usleep(unsigned long nusec)
{
    nusec <<= 20;
    while (nusec--) {
        ;
    }
}

void
apicinit(long id)
{
    volatile struct m_cpu *cpu = &mpcputab[id];

    if (!mpapic) {

        return;
    }
    /* identity-map MP table */
    if (cpu == mpbootcpu) {
        vmmapseg((uint32_t *)&_pagetab, (uint32_t)mpapic, (uint32_t)mpapic,
                 (uint32_t)((uint8_t *)mpapic + PAGESIZE),
                 PAGEPRES | PAGEWRITE);
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
apicsethz(long hz)
{
}

#endif /* SMP */

