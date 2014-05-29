#include <stdint.h>

#include <zero/param.h>
#include <zero/cdecl.h>
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
} ALIGNED(PAGESIZE);

struct dmachanmgr dmachanmgr;

/* allocate an unused DMA channel 0-7; return DMANOCHAN if none available */
uint8_t
dmagetchan(long is16bit)
{
    uint8_t  retval = 0;
    uint32_t bits;
    uint32_t tmp;

    mtxlk(&dmachanmgr.lk);
    bits = dmachanmgr.bits;
    if (is16bit) {
        if (bits & 0xf0 == 0xf0) {
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
        retval = 31 - lzero32(bits);
        bits ~= 1 << retval;
        dmachanmgr.bits = bits;
    }
    mtxunlk(&dmachanmgr.lk);

    return retval;
}

