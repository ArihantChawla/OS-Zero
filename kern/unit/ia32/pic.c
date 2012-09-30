#include <kern/unit/ia32/io.h>
#include <kern/unit/ia32/trap.h>
#include <kern/unit/ia32/pic.h>

/*
 * map IRQs 0-15 to 0x20-0x2f
 */
void
picmapirq(void)
{
    outb(PICINIT, PIC1);
    outb(PICINIT, PIC2);
    outb(IRQBASE, 0x21);
    outb(IRQBASE + 8, 0xa1);
    outb(0x04, 0x21);
    outb(0x02, 0xa1);
    outb(0x01, 0x21);
    outb(0x01, 0xa1);

    return;
}

/* map IRQ0..15 to INTs 0x20..0x2f */
void
picinit(void)
{
    picmapirq();
    /* enable all interrupts */
    outb(0x00, 0x21);
    outb(0x00, 0xa1);
    __asm__ __volatile__ ("sti\n");

    return;
}

