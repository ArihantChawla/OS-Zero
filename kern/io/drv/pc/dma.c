#include <stdint.h>

#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <zero/mtx.h>
#include <kern/util.h>
#include <kern/io/drv/pc/dma.h>

const uint8_t dmapageports[DMANCHAN] ALIGNED(PAGESIZE)
= {
    0x00,
    DMAPAGE1,
    DMAPAGE2,
    DMAPAGE3,
    0x00,
    DMAPAGE5,
    DMAPAGE6,
    DMAPAGE7
};

struct dmadrv dmadrv;

void
dmainit(void)
{
    dmadrv.ports = dmapageports;

    return;
}

/* allocate an unused DMA channel 0-7; return DMANOCHAN if none available */
long
dmagetchan(long is16bit)
{
    long retval = 0;
    long bits;
    long tmp;

    fmtxlk(&dmadrv.lk);
    bits = dmadrv.chanbits;
    if (is16bit) {
        if ((bits & 0xf0) == 0xf0) {
            retval = DMANOCHAN;
        } else {
            tmp = bits;
        }
    } else {
        tmp = bits & 0x0f;
        if (tmp == 0x0f) {
            retval = DMANOCHAN;
        }
    }
    if (retval != DMANOCHAN) {
        retval = 31 - lzerol(bits);
        bits |= 1 << retval;
        dmadrv.chanbits = bits;
    }
    fmtxunlk(&dmadrv.lk);

    return retval;
}

/* mark a DMA channel in-use; return DMANOCHAN on failure */
long
dmatakechan(unsigned long chan)
{
    long retval = DMANOCHAN;
    long bits;

    if (gtepow2(chan, 8)) {
        kprintf("%s: invalid DMA channel: %ld\n", __FUNCTION__, chan);

        return retval;
    }
    fmtxlk(&dmadrv.lk);
    bits = dmadrv.chanbits;
    if (!(bits & (1 << chan))) {
        bits |= 1 << chan;
        dmadrv.chanbits = bits;
        retval = chan;
    } else {
        kprintf("%s: DMA channel in use already: %ld\n", __FUNCTION__, chan);
    }
    fmtxunlk(&dmadrv.lk);

    return retval;
}

/* release DMA channel */
void
dmarelchan(unsigned long chan)
{
    long bits;

    if (gtepow2(chan, 8)) {
        kprintf("%s: invalid DMA channel: %ld\n", __FUNCTION__, chan);
    }
    fmtxlk(&dmadrv.lk);
    bits = dmadrv.chanbits;
    bits &= ~(1 << chan);
    dmadrv.chanbits = bits;
    fmtxunlk(&dmadrv.lk);

    return;
}
