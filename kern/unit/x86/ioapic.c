#include <kern/conf.h>

#if (SMP) || (APIC)

#include <stdint.h>
#include <kern/mem/vm.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/x86/ioapic.h>
#include <kern/unit/x86/link.h>
#include <kern/unit/ia32/mp.h>

extern volatile struct cpu       cputab[NCPU];
extern volatile struct cpu      *mpbootcpu;
extern volatile struct mpioapic *mpioapic;
volatile struct ioapic          *ioapic;

void
ioapicinit(void)
{
    volatile struct cpu *cpu = k_curcpu;
    long                 ntrap;
//    long id;
    long                 l;

    if (!mpmultiproc) {

        return;
    }
    ioapic = (volatile struct ioapic *)mpioapic;
    if (cpu == mpbootcpu) {
        vmmapseg((uint32_t *)&_pagetab, (uint32_t)mpioapic, (uint32_t)mpioapic,
                 (uint32_t)((uint8_t *)mpioapic + PAGESIZE),
                 PAGEPRES | PAGEWRITE);
    }
    ntrap = (ioapicread(IOAPICVER) >> 16) & 0xff;
    for (l = 0 ; l < ntrap ; l++) {
        ioapicwrite(IOAPICDISABLED | (IRQTMR + l), IOAPICTAB + 2 * l);
        ioapicwrite(0, IOAPICTAB + 2 * l + 1);
    }

    return;
}

void
ioapicsetirq(long irq, long id)
{
    if (!mpmultiproc) {

        return;
    }
    ioapicwrite(IRQTMR + irq, IOAPICTAB + 2 * irq);
    ioapicwrite(id << 24, IOAPICTAB + 2 * irq + 1);

    return;
}

#endif /* SMP */

