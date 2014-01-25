#ifndef __KERN_IO_DRV_PC_AC97_H__
#define __KERN_IO_DRV_PC_AC97_H__

/* interface for PCI device 0x31, function 5 (audio controller) */

#define AC97NBUF          32

#define AC97BASE          
#define ac97getirq()                                                    \
    inb(AC97INTLINE)
/* registers */
#define AC97GLOBALCTLSTAT 0x60  // global control/status register
#define AC97CMD           0x04
#define AC97MIXERBASE     0x10
#define AC97BUSMASTERBASE 0x14
#define AC97INTLINE       0x3c
/* mixer registers */
#define AC97MIXRST        0x00
#define AC97MIXMASTVOL    0x02
#define AC97MIXAOUTVOL    0x04
#define AC97MIXMONOVOL    0x06
#define AC97MIXMASTTONE   0x08
#define AC97MIXBEEPVOL    0x0a
/* phone volume is 0x0c; audio only */
#define AC97MIXMICVOL     0x0e
#define AC97MIXLINEINVOL  0x10
#define AC97MIXCDVOL      0x12
#define AC97MIXVIDEOVOL   0x14
#define AC97MIXAUXINVOL   0x16
#define AC97MIXPCMOUTVOL  0x18
#define AC97MIXRECSEL     0x1a
#define AC97MIXRECGAIN    0x1c
#define AC97MINRECGAINMIC 0x1e
#define AC97MIXGENPURP    0x20
#define AC97MIX3DCTL      0x22

/* buffers */
#define AC97PCMINBUF      0x00
#define AC97PCMOUTBUF     0x10
#define AC97MICBUF        0x20
/* AC97 buffer descriptor */
#define AC97BUFPTR        0x00  // bit 0 is zero, 1..31 are buffer address
#define AC97BUFINFO       0x04  // see below
/* interrupt on completion */
#define AC97IOCBIT        0x80000000
/*
 * 0 -> if buffer is complete and next is not ready, transmit last valid sample
 * 1 -> if buffer complete and last valid, transmit zeros after processed
 *      - typically only set for the last buffer in current stream
 */
#define AC97BUPMASK       0x60000000
/* reserved bits */
#define AC97RESMASK       0x3fff0000
#define AC97BUFLENMASK    0x0000ffff    // max 65536 samples (128 KB)

/* commands */
#define AC97BUSMASTER     0x04
#define AC97IOSPACE       0x01
#define AC97INIT          (AC97BUSMASTER | AC97IOSPACE)

#define AC97NBUF          32
//#define AC97BUFSIZE       (65536 * sizeof(int16_t))
#define AC97BUFSIZE       4096

#define ac97setptr(bp, ptr)                                             \
    ((bp)->adr |= ((uint32_t)ptr & 0x7ffffffe))
#define ac97setioq(bp)                                                  \
    ((bp)->adr |= 0x80000000)
#define ac97clrioq(bp)                                                  \
    ((bp)->adr &= 0x7fffffff)
#define ac97setbup(bp, bup)                                             \
    ((bp)->info |= (bup) << 30))
#define ac97setlen(bp, len)                                             \
    ((bp)->info |= (len) & AC97BUFLENMASK)
struct ac97bufdesc {
    uint32_t adr;       // ptr, bit 0 is zero (16-bit aligned)
    uint32_t info;      // flags, length
} PACK();

#endif /* __KERN_IO_DRV_PC_AC97_H__ */

