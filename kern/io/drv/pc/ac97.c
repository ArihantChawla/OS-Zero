#include <stdint.h>

#include <kern/util.h>
#include <kern/mem.h>
#include <kern/io/drv/pc/pci.h>
#include <kern/io/drv/pc/ac97.h>

extern void *irqvec[];

/* TODO: add microphone support */
static struct ac97bufdesc _ac97inbuftab[AC97NBUF];
static struct ac97bufdesc _ac97outbuftab[AC97NBUF];
static long               _ac97irq;
static long               _ac97dma;
static long               _ac97inbufhalf = 0;
static long               _ac97outbufhalf = 0;

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
    struct ac97bufdesc *inbuf = _ac97inbuftab;
    struct ac97bufdesc *outbuf = _ac97outbuftab;
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

long
ac97probe(void)
{
    struct pcidev *dev;
    long           ndev;

    dev = &pcidevtab[0];
    ndev = pcindev;
    if (ndev) {
        while (ndev--) {
            if (ac97chkdev(dev)) {

                return 1;
            }
            dev++;
        }
    }
}

void
ac97init(void)
{
    if (!ac97probe() || !ac97initbuf()) {
        kprintf("AC97: failed to initialise audio buffers\n");

        return;
    } else {
        kprintf("AC97: audio controller found\n");
    }
}

