#include <kern/conf.h>

#if (SMP) || (APIC)

#include <stdint.h>
#include <kern/mem/vm.h>
#include <kern/unit/x86/kern.h>
#include <kern/unit/ia32/mp.h>

extern volatile struct cpu  cputab[NCPU];
extern volatile struct cpu *mpbootcpu;
extern volatile uint32_t   *mpioapic;
volatile struct ioapic     *ioapic;

void
ioapicinit(long id)
{
    volatile struct cpu *cpu = &cputab[id];
    long                 ntrap;
//    long id;
    long                 l;

    if (!mpmultiproc) {

        return;
    }
    ioapic = (volatile struct ioapic *)mpioapic;
    if (cpu == mpbootcpu) {
        vmmapseg((uint32_t)mpioapic, (uint32_t)mpioapic,
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

