#include <stdint.h>

#include <kern/io/drv/pc/dma.h>

const uint8_t dmapageports[DMANCHAN]
= {
    DMAPAGE0,
    DMAPAGE1,
    DMAPAGE2,
    DMAPAGE3,
    0x00,
    DMAPAGE5,
    DMAPAGE6,
    DMAPAGE7
};

