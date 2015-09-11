#include <stdint.h>
#include <sys/io.h>

#include <zero/param.h>
#include <zero/cdecl.h>

#include <kern/util.h>
#include <kern/malloc.h>
#include <kern/io/drv/pc/pci.h>
#include <kern/io/drv/pc/ac97.h>
#include <kern/unit/x86/asm.h>

extern void *irqvec[];

/* TODO: add microphone support */

static struct ac97drv ac97drv ALIGNED(PAGESIZE);

#if 0
#define ac97chkdev(dev)                                                 \
    ((dev)->vendor == 0x8086 && (dev)->id == 0x2415)
#endif

/*
 * - allocate the max of 32 * 4K buffers for both input and output
 * - receive interrupts on completion of every 16 buffers
 */
long
ac97initbuf(void)
{
    long                l = AC97NBUF;
    struct ac97bufdesc *inbuf = ac97drv.inbuftab;
    struct ac97bufdesc *outbuf = ac97drv.outbuftab;
    uint8_t            *inptr;
    uint8_t            *outptr;

    /* allocate input buffer */
    inptr = kwalloc(AC97NBUF * AC97BUFSIZE);
    if (!inptr) {
        kprintf("AC97: failed to allocate audio input buffer\n");

        return 0;
    }
    /* allocate output buffer */
    outptr = kwalloc(AC97NBUF * AC97BUFSIZE);
    if (!outptr) {
        kprintf("AC97: failed to allocate audio output buffer\n");

        kfree(inptr);

        return 0;
    }
    /* clear buffers */
    kbzero(inptr, AC97NBUF * AC97BUFSIZE);
    kbzero(outptr, AC97NBUF * AC97BUFSIZE);
    /* set up buffer descriptors */
    while (l--) {
        inbuf->adr = (uint32_t)inptr;
        inbuf->info = AC97BUFSIZE - 1;
        outbuf->adr = (uint32_t)outptr;
        outbuf->info = AC97BUFSIZE - 1;
        inbuf++;
        outbuf++;
        inptr += AC97BUFSIZE;
        outptr += AC97BUFSIZE;
    }

    return 1;
}

void
ac97initdev(struct pcidev *dev)
{
    uint16_t word;
    uint8_t  byte;

    /* perform register reset */
    pciwriteconf1(dev->bus,
                  dev->slot,
                  AC97AUDFUNC,
                  AC97MIXRESET,
                  0,
                  2);
    /* wait for codec #1 to become ready */
    do {
        word = pcireadconf1(dev->bus,
                            dev->slot,
                            AC97AUDFUNC,
                            AC97GLOBALSTAT,
                            2);
//                    kprintf("AC97STAT: %x\n", word);
    } while (word == 0xffff || !(word & AC97CODEC1READY));
    /* legacy PCI initialisation */
    /* initialise controller; bus-mastering, enable I/O space */
    pciwriteconf1(dev->bus,
                  dev->slot,
                  AC97AUDFUNC,
                  AC97GLOBALCTL,
                  AC97INIT,
                  4);
    /* wait for codec #1 to become ready */
    do {
        word = pcireadconf1(dev->bus,
                            dev->slot,
                            AC97AUDFUNC,
                            AC97GLOBALSTAT,
                            2);
//                    kprintf("AC97STAT: %x\n", word);
    } while (word == 0xffff || !(word & AC97CODEC1READY));
    /* 4-byte initialisation sequence */
    /* write ICW1 to primary PIC */
    pciwriteconf2(dev->bus,
                  dev->slot,
                  AC97AUDFUNC,
                  AC97PIC1BASE,
                  1,
                  AC97ICW1);
    /* write ICW2 to primary PIC */
    pciwriteconf2(dev->bus,
                  dev->slot,
                  AC97AUDFUNC,
                  AC97PIC1CMD,
                  1,
                  AC97ICW2);
    pciwriteconf2(dev->bus,
                  dev->slot,
                  AC97AUDFUNC,
                  AC97PIC1CMD,
                  1,
                  AC97ICW3);
    pciwriteconf2(dev->bus,
                  dev->slot,
                  AC97AUDFUNC,
                  AC97PIC1CMD,
                  1,
                  AC97ICW4);
    /* detect IRQ */
    byte = pcireadconf1(dev->bus,
                        dev->slot,
                        AC97AUDFUNC,
                        AC97INTRLINE,
                        1);
    /* FIXME: is this correct? */
    dev->irq = (byte >> 3) & 0x1f;

    return;
}

void
ac97init(struct pcidev *dev)
{
    ac97initdev(dev);
    if (!ac97initbuf()) {
        kprintf("AC97: failed to initialise audio buffers\n");

        return;
    }
    kprintf("AC97 audio controller: vendor == 0x%x, devid == 0x%x, bus == 0x%x, slot == 0x%x, irq == 0x%x\n",
            dev->vendor, dev->id, dev->bus, dev->slot, dev->irq);

    return;
}

