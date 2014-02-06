#ifndef __KERN_DRV_SND_SB16_SB16_H__
#define __KERN_DRV_SND_SB16_SB16_H__

#include <stdint.h>
#include <sys/io.h>

typedef uint8_t sb16pcm8_t;
typedef int16_t sb16pcm16_t; /* little endian */

#define SB16BUFSIZE      16384U
/* # of milliseconds slept after reset */
#define SB16RESETMS      8

/* basic I/O */
#define SB16BASE         0x220
#define SB16RESET        (0x206 | (SB16BASE & 0x0f0))
#define SB16READ         (0x20a | (SB16BASE & 0x0f0))
#define SB16WRITE        (0x20c | (SB16BASE & 0x0f0))
#define SB16WRBUFSTAT    (0x20c | (SB16BASE & 0x0f0))
#define SB16RDBUFSTAT    (0x20e | (SB16BASE & 0x0f0))
#define SB16BUFSTATBIT   0x80
#define SB16READY        0xaa
/* configuration */
#define SB16MIXERADR     (0x204 | (SB16BASE & 0x0f0))
#define SB16MIXERDATA    (0x205 | (SB16BASE & 0x0f0))
#define SB16CONFIRQ      0x80
#define SB16CONFDMA      0x81
#define SB16IRQ5BIT      0x02
#define SB16IRQ7BIT      0x04
#define SB16IRQ10BIT     0x08
#define SB16DMA0BIT      0x01
#define SB16DMA1BIT      0x02
#define SB16DMA3BIT      0x08
#define SB16DMA5BIT      0x20
#define SB16DMA6BIT      0x40
#define SB16DMA7BIT      0x80
/* interrupt handling */
#define SB16INTRSTAT     0x82
#define SB16DMA8MIDIBIT  0x01
#define SB16DMA16BIT     0x02
#define SB16MPU401BIT    0x04
#define SB16DMA8MIDISTAT (0x20e | (SB16BASE & 0x0f0))
#define SB16DMA16STAT    (0x20f | (SB16BASE & 0x0f0))
#define SB16MPU401STAT   (0x300 | (SB16BASE & 0x0f0))
/* DMA interface */
#define SB16SETBLKSIZE   0x48
/* CT1745 mixer */
#define SB16RESETMIXER   0x00
/* volume */
#if 0
#define SB16VOICEVOL     0x04 // LLLLRRRR
#define SB16MICVOL       0x0a // 00000MMM
#define SB16MASTERVOL    0x22 // LLLLRRRR
#define SB16MIDIVOL      0x26 // LLLLRRRR
#define SB16CDVOL        0x28 // LLLLRRRR
#define SB16LINEVOL      0x2e // LLLLRRRR
#endif
#define SB16MAXVOL       32
#define SB16MASTERLVOL   0x30 // 000VVVVV
#define SB16MASTERRVOL   0x31 // 000VVVVV
#define SB16VOICELVOL    0x32 // 000VVVVV
#define SB16VOICERVOL    0x33 // 000VVVVV
#define SB16MIDILVOL     0x34 // 000VVVVV
#define SB16MIDIRVOL     0x35 // 000VVVVV
#define SB16CDLVOL       0x36 // 000VVVVV
#define SB16CDRVOL       0x37 // 000VVVVV
#define SB16LINELVOL     0x38 // 000VVVVV
#define SB16LINERVOL     0x39 // 000VVVVV
#define SB16MICMVOL      0x3a // 000VVVVV
#define SB16PCSPKRVOL    0x3b // 000000VV
/* switch bits, listed from #0 up */
#define SB16OUTPUTBITS   0x3c // mic, CDR, CDL, lineR, lineL
#define SB16INPUTLBITS   0x3d // mic, CDR, CDL, lineR, lineL, MIDIR, MIDIL
#define SB16INPUTRBITS   0x3e // mic, CDR, CDL, lineR, lineL, MIDIR, MIDIL
#define SB16INPUTLGAIN   0x3f // 000000GG
#define SB16INPUTRGAIN   0x40 // 000000GG
#define SB16OUTPUTLGAIN  0x41 // 000000GG
#define SB16OUTPUTRGAIN  0x42 // 000000GG
#define SB16AGC          0x43 // 000000AA
#define SB16TREBLEL      0x44 // TTTT0000
#define SB16TREBLER      0x45 // TTTT0000
#define SB16BASSL        0x44 // BBBB0000
#define SB16BASSR        0x45 // BBBB0000
/* input and output rate commands */
#define SB16INPUTRATE    0x41
#define SB16OUTPUTRATE   0x42

/*
 * - write 1 to reset port
 * - wait 3 microseconds
 * - write 0 to reset port
 * - poll for SB16READY on SB16RDBUFSTAT
 */
#define sb16reset()                                                     \
    do {                                                                \
        unsigned long stat = 0;                                         \
                                                                        \
        outw(0x01, SB16RESET);                                          \
        /* usleep(3); */                                                \
        outw(0x00, SB16RESET);                                          \
        do {                                                            \
            stat = inb(SB16RDBUFSTAT) & SB16BUFSTATBIT;                 \
        } while (!stat && inb(SB16READ) != SB16READY);                  \
    } while (0)

/*
 * - read maximum of len bytes into buf
 * - read only when SB16BUFSTATBIT is returned from SBRDBUFSTAT
 */
static __inline__
size_t sb16read(void *buf, size_t len)
{
    size_t   nread = 0;
    uint8_t *u8ptr = buf;
    
    len >>= 1;
    while ((len) && inb(SB16RDBUFSTAT & SB16BUFSTATBIT)) {
        *u8ptr++ = inb(SB16READ);
        nread++;
        len--;
    }

    return nread;
}

static __inline__
size_t sb16write(void *buf, size_t len)
{
    size_t   nwritten = 0;
    uint8_t *u8ptr = buf;

    len >>= 1;
    do {
        while (inb(SB16WRBUFSTAT) & SB16BUFSTATBIT) {
            ;
        }
        outw(*u8ptr, SB16WRITE);
        u8ptr++;
        nwritten++;
        len--;
    } while (len);

    return nwritten;
}

#endif /* __KERN_DRV_SND_SB16_SB16_H__ */

