#ifndef __KERN_IO_DRV_PC_AC97_H__
#define __KERN_IO_DRV_PC_AC97_H__

/* interface for PCI device 0x31, function 5 (audio controller) */

#define AC97NBUF          32

#define AC97BASE          
#define ac97getirq()                                                    \
    inb(AC97INTLINE)
/* registers */
#define AC97GLOBALCS      0x60  // global control/status register
#define AC97CMD           0x04
#define AC97MIXERBASE     0x10
#define AC97BUSMASTERBASE 0x14
#define AC97INTLINE       0x3c

/* buffers */
#define AC97PCMINBUF      0x00
#define AC97PCMOUTBUF     0x10
#define AC97MICBUF        0x20
/* AC97 buffer descriptor */
#define AC97BUFPTR        0x00  // bit 0 is zero, 1..31 are buffer address
#define AC97BUFINFO       0x04  // see below
/* interrupt on completion */
/*
 * 0 -> if buffer is complete and next is not ready, transmit last valid sample
 * 1 -> if buffer complete and last valid, transmit zeros after processed
 *      - typically only set for the last buffer in current stream
 */
#define AC97IOCBIT        0x80000000
#define AC97BUPMASK       0x60000000
/* reserved bits */
#define AC97RESMASK       0x3fff0000
#define AC97BUFLENMASK    0x0000ffff    // max 65536 samples (128 KB)

/* commands */
#define AC97BUSMASTER     0x04
#define AC97IOSPACE       0x01
#define AC97INIT          (AC97BUSMASTER | AC97IOSPACE)

#define AC97NBUF          32
#define AC97BUFSIZE       (65536 * sizeof(int16_t))
struct ac97bufdesc {
    uint32_t adr;       // ptr, bit 0 is zero (16-bit aligned)
    uint32_t info;      // flags, length
} PACK();

#endif /* __KERN_IO_DRV_PC_AC97_H__ */

