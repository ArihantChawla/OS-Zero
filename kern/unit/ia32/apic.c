#include <stdint.h>
#include <kern/conf.h>

#if (SMP)

#include <kern/unit/x86/trap.h>
#include <kern/unit/ia32/link.h>
#include <kern/unit/ia32/vm.h>
#include <kern/unit/ia32/io.h>
#include <kern/unit/ia32/apic.h>
#if 0
#include <kern/unit/ia32/pit.h>
#endif

extern void segmap(uint32_t *pagetab, uint32_t virt, uint32_t phys,
                   uint32_t lim,
                   uint32_t flg);
extern volatile uint32_t     *mpapic;
extern volatile struct m_cpu  mpcputab[NCPU];
extern volatile struct m_cpu *mpbootcpu;

void
apicinit(long id)
{
    volatile struct m_cpu *cpu = &mpcputab[id];

    if (!mpapic) {

        return;
    }
    /* identity-map local APIC */
    if (cpu == mpbootcpu) {
        segmap((uint32_t *)&_pagetab, (uint32_t)mpapic, (uint32_t)mpapic,
               (uint32_t)((uint8_t *)mpapic + PAGESIZE),
               PAGEPRES | PAGEWRITE);
    }
    cpu->id = id;
    /* enable local APIC; set spurious interrupt vector */
    apicwrite(APICENABLE | (IRQTIMER + IRQSPURIOUS), APICSPURIOUS);
    /*
     * timer counts down at bus frequency from APICTMRINITCNT and issues an
     * an interrupt
     */
    apicwrite(APICBASEDIV, APICTMRDIVCONF);
    apicwrite(APICPERIODIC | (IRQBASE + IRQTIMER), APICTIMER);
    apicwrite(10000000, APICTMRINITCNT);
    /* disable logical interrupt lines */
    apicwrite(APICMASKED, APICLINT0);
    /* disable performance counter overflow interrupts */
    apicwrite(APICMASKED, APICLINT1);
    if (((apicread(APICVER) >> 16) & 0xff) >= 4) {
        apicwrite(APICMASKED, APICPERFINT);
    }
    /* map error interrupt */
    apicwrite(IRQERROR, APICERROR);
    /* clear error status registers */
    apicwrite(0, APICERRSTAT);
    apicwrite(0, APICERRSTAT);
    /* acknowledge outstanding interrupts */
    apicwrite(0, APICEOI);
    /* send init level deassert to synchronise arbitration IDs */
    apicsendirq(0, APICBCAST | APICINIT | APICLEVEL, 0);
    while (apicread(APICINTLO) & APICDELIVS) {
        ;
    }
    /* enable APIC (but not CPU) interrupts */
    apicwrite(0, APICTASKPRIO);

    return;
}

#define RTCBASE 0x70
#define BIOSWRV 0x467   // warm reset vector

void
usleep(long nusec)
{
    nusec <<= 1;
    while (nusec--) {
        nusec--;
    }
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
    apicsendirq(id << 24, APICINIT | APICLEVEL | APICASSERT, 100);
//    usleep(1000);
    while (apicread(APICINTLO) & APICDELIVS) {
        ;
    }
    apicsendirq(id << 24, APICINIT | APICLEVEL, 0);
    while (apicread(APICINTLO) & APICDELIVS) {
        ;
    }
    apicsendirq(id << 24, APICASSERT | APICSTART | adr >> 12, 0);
//    usleep(200);
    apicsendirq(id << 24, APICASSERT | APICSTART | adr >> 12, 0);
//    usleep(200);

    return;
}

#endif /* SMP */

