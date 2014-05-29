#include <stdint.h>

#include <zero/param.h>
#include <zero/cdecl.h>
#include <zero/trix.h>
#include <kern/io/drv/pc/dma.h>

const uint8_t dmapageports[DMANCHAN]
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

struct dmachanmgr dmachanmgr;

/* allocate an unused DMA channel 0-7; return DMANOCHAN if none available */
long
dmagetchan(long is16bit)
{
    long retval = 0;
    long bits;
    long tmp;

    mtxlk(&dmachanmgr.lk);
    bits = dmachanmgr.bits;
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
        dmachanmgr.bits = bits;
    }
    mtxunlk(&dmachanmgr.lk);

    return retval;
}

/* mark a DMA channel in-use; return DMANOCHAN on failure */
long
dmatakechan(unsigned long chan)
{
    long retval = DMANOCHAN;
    long bits;

    if (gtepow2(chan, 8)) {
        kprintf("invalid DMA channel: %ld\n", chan);

        return retval;
    }
    mtxlk(&dmachanmgr.lk);
    bits = dmachanmgr.bits;
    if (!(bits & (1 << chan))) {
        bits |= 1 << chan;
        dmachanmgr.bits = bits;
        retval = chan;
    } else {
        kprintf("DMA channel in use already: %ld\n", chan);
    }
    mtxunlk(&dmachanmgr.lk);

    return retval;
}

