#include <kern/conf.h>

#if (SMP)

#include <stdint.h>
#include <kern/ia32/link.h>
#include <kern/ia32/trap.h>
#include <kern/ia32/mp.h>
#include <kern/ia32/vm.h>
#include <kern/ia32/ioapic.h>

extern void segmap(uint32_t *pagetab, uint32_t virt, uint32_t phys,
                   uint32_t lim,
                   uint32_t flg);

extern volatile struct m_cpu     mpcputab[NCPU];
extern volatile long             mpmultiproc;
extern volatile struct m_cpu    *mpbootcpu;
extern volatile struct mpioapic *mpioapic;
volatile struct ioapic          *ioapic;

void
ioapicinit(long id)
{
    struct m_cpu *cpu = &mpcputab[id];
    long ntrap;
//    long id;
    long l;

    if (!mpmultiproc) {

        return;
    }
    ioapic = (volatile struct ioapic *)mpioapic;
    if (cpu == mpbootcpu) {
        segmap((uint32_t *)&_pagetab, (uint32_t)mpioapic, (uint32_t)mpioapic,
               (uint32_t)((uint8_t *)mpioapic + PAGESIZE),
               PAGEPRES | PAGEWRITE);
    }
    ntrap = (ioapicread(IOAPICVER) >> 16) & 0xff;
//    id = ioapicread(IOAPICID) >> 24;
    for (l = 0 ; l < ntrap ; l++) {
        ioapicwrite(IOAPICDISABLED | (IRQTIMER + l), IOAPICTAB + 2 * l);
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
    ioapicwrite(IRQTIMER + irq, IOAPICTAB + 2 * irq);
    ioapicwrite(id << 24, IOAPICTAB + 2 * irq + 1);

    return;
}

#endif /* SMP */

