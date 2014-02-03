#include <stdint.h>
#include <sys/io.h>

#include <zero/param.h>
#include <zero/cdecl.h>

#include <kern/util.h>
#include <kern/mem.h>
#include <kern/io/drv/pc/pci.h>
#include <kern/io/drv/pc/ac97.h>

extern void *irqvec[];

/* TODO: add microphone support */

static struct ac97drv ac97drv ALIGNED(PAGESIZE);

#define ac97chkdev(dev)                                                 \
    ((dev)->vendor == 0x8086 && (dev)->id == 0x2415)

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

struct pcidev *
ac97probe(void)
{
    struct pcidev *dev = NULL;
    long           ndev;
//    uint16_t       word;

    dev = &pcidevtab[0];
    ndev = pcindev;
    if (ndev) {
        while (ndev--) {
            kprintf("AC97DEBUG: %x, %x\n", dev->vendor, dev->id);
            if (ac97chkdev(dev)) {
                /* legacy PCI initialisation */
#if 0
                pciwriteconfl(AC97INIT,
                              dev->bus, dev->slot,
                              AC97AUDFUNC, PCICONFADR);
                do {
                    word = pcireadconfw(dev->bus,
                                        dev->slot,
                                        AC97AUDFUNC,
                                        AC97GLOBALSTAT1);
                    kprintf("AC97STAT: %x\n", word);
                } while (word == 0xffff || !(word & AC97CODEC1READY));
                dev->irq = pcireadconfw(dev->bus, dev->slot,
                                        AC97AUDFUNC, AC97INTLINE);
#endif
                
                return dev;
            }
            dev++;
        }
    }

    return NULL;
}

void
ac97init(void)
{
    struct pcidev *dev;

    dev = ac97probe();
    if (!dev) {
        kprintf("AC97 controller not found\n");
    }
    if (!ac97initbuf()) {
        kprintf("AC97: failed to initialise audio buffers\n");

        return;
    }
    kprintf("AC97 audio controller: v == %x, d == %x, b == %x, s == %x, i == %x, d == %x\n",
            dev->vendor, dev->id, dev->bus, dev->slot, dev->irq, dev->dma);

    return;
}

