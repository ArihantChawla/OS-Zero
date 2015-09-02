#ifndef __KERN_IO_DRV_PC_AC97_H__
#define __KERN_IO_DRV_PC_AC97_H__

/* interface for PCI device 0x31, function 5 (audio controller) */

//#define ac97reset(ap)  outb(AC97INIT, ap->iobase + AC97CMD)
//#define ac97getirq(ap) inb(ap->iobase + AC97INTLINE)

#define AC97AUDFUNC        0x05U

/* 32 * 32K == 1024K buffer space */
#define AC97BUFSIZE        32768
#define AC97NBUF           32

/* configuration registers */
#define AC97VENDORID       0x00         // 16-bit
#define AC97DEVID          0x02         // 16-bit
#define AC97CMD            0x04         // 16-bit
#define AC97STAT           0x06         // 16-bit
#define AC97REVISIONID     0x08         // 8-bit
#define AC97PROGINTERFACE  0x09         // 8-bit
#define AC97SUBCLASS       0x0a         // 8-bit
#define AC97BASECLASS      0x0b         // 8-bit
#define AC97HDRTYPE        0x0e         // 8-bit
#define AC97MIXERBASE      0x10         // 32-bit
#define AC97AUDIOBASEADR   0x14         // 32-bit
#define AC97MIXERBASEADR   0x18         // 32-bit
#define AC97BUSMASTERADR   0x1c         // 32-bit
#define AC97INTRLINE       0x3c         // 8-bit

/* initialisation */
#define AC97PIC1BASE       0x20
#define AC97PIC1CMD        0x21
#define AC97PIC2BASE       0xa0
#define AC97PIC2CMD        0xa1
#define AC97ICW1           0x10         // write to controller base address
#define AC97ICW2           0x05         // IRQs 7 and 14
#define AC97ICW3           0x04         // IRQ 2 used for cascading the slave
#define AC97ICW4           0x01         // Intel-architecture

/* mixer registers */
#define AC97MIXRESET       0x00
#define AC97MIXMASTVOL     0x02
#define AC97MIXAOUTVOL     0x04
#define AC97MIXMONOVOL     0x06
#define AC97MIXMASTTONE    0x08
#define AC97MIXBEEPVOL     0x0a
/* phone volume is 0x0c; audio only */
#define AC97MIXMICVOL      0x0e
#define AC97MIXLINEINVOL   0x10
#define AC97MIXCDVOL       0x12
#define AC97MIXVIDEOVOL    0x14
#define AC97MIXAUXINVOL    0x16
#define AC97MIXPCMOUTVOL   0x18
#define AC97MIXRECSEL      0x1a
#define AC97MIXRECGAIN     0x1c
#define AC97MINRECGAINMIC  0x1e
#define AC97MIXGENPURP     0x20
#define AC97MIX3DCTL       0x22
#define AC97GLOBALCTL      0x2c // global control/status register
#define AC97GLOBALSTAT     0x30 // global control/status register
/* status bits */
#define AC97CODEC1READY    0x100

/* buffers */
#define AC97PCMINBUF       0x00
#define AC97PCMOUTBUF      0x10
#define AC97MICBUF         0x20
/* AC97 buffer descriptor */
#define AC97BUFPTR         0x00  // bit 0 is zero, 1..31 are buffer address
#define AC97BUFINFO        0x04  // see below
/* interrupt on completion */
#define AC97IOCBIT         0x80000000
/*
 * 0 -> if buffer is complete and next is not ready, transmit last valid sample
 * 1 -> if buffer complete and last valid, transmit zeros after processed
 *      - typically only set for the last buffer in current stream
 */
#define AC97BUPMASK        0x60000000
/* reserved bits */
#define AC97RESMASK        0x3fff0000
#define AC97BUFLENMASK     0x0000ffff    // max 65536 samples (128 KB)

/* commands */
#define AC97INIT          (AC97BUSMASTER | AC97IOSPACE)
#define AC97BUSMASTER      0x04U
#define AC97IOSPACE        0x01U

#define ac97setptr(bp, ptr)                                             \
    ((bp)->adr |= ((uint32_t)ptr & 0x7ffffffe))
#define ac97setioc(bp)                                                  \
    ((bp)->adr |= AC97IOCBIT)
#define ac97clrioc(bp)                                                  \
    ((bp)->adr &= ~AC97IOCBIT)
#define ac97setbup(bp, bup)                                             \
    ((bp)->info |= (bup) << 30))
#define ac97setlen(bp, len)                                             \
    ((bp)->info |= (len) & AC97BUFLENMASK)
struct ac97bufdesc {
    uint32_t adr;       // ptr, bit 0 is zero (16-bit aligned)
    uint32_t info;      // flags, length
};

struct ac97drv {
    struct ac97bufdesc inbuftab[AC97NBUF];
    struct ac97bufdesc outbuftab[AC97NBUF];
    long               irq;
    long               dma;
    long               inbufhalf;
    long               outbufhalf;
    uint16_t           iobase;
};

#endif /* __KERN_IO_DRV_PC_AC97_H__ */

