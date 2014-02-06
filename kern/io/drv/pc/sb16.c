#include <stddef.h>
#include <stdint.h>
#include <sys/io.h>

#include <zero/param.h>
#include <zero/cdecl.h>
#include <zero/mtx.h>
#include <zero/trix.h>

#include <kern/util.h>
#include <kern/mem.h>
//#include <kern/prio.h>
//#include <kern/thr.h>
#include <kern/io/drv/pc/dma.h>
#include <kern/io/drv/pc/sb16.h>

void        sb16intr(void);
void        sbsetrate(long cmd, uint16_t hz);
//extern void pitsleep(long msec, void (*func)(void));
extern void usleep(unsigned long nusec);

extern void *irqvec[];

#define SB16DATA8BUFSIZE  (4 * SB16BUFSIZE)
#define SB16DATA16BUFSIZE (8 * SB16BUFSIZE)

static long      _sb16init = 0;
static uint8_t  *_sb16dmabuf8;
static uint16_t *_sb16dmabuf16;
static uint8_t   _sb16irq;
static uint8_t   _sb16dma8;
static uint8_t   _sb16dma16;
static long      _sb16buflock = MTXINITVAL;
static long      _sb16dmainofs8 = 0;
static long      _sb16dmainofs16 = 0;
static long      _sb16dmaoutofs8 = 0;
static long      _sb16dmaoutofs16 = 0;
static long      _sb16dmain8full = 0;
static long      _sb16dmain16full = 0;
static long      _sb16dmaout8empty = 1;
static long      _sb16dmaout16empty = 1;

static uint8_t  *_sb16inbuf8;
static uint8_t  *_sb16outbuf8;
static uint16_t *_sb16inbuf16;
static uint16_t *_sb16outbuf16;
static void     *_sb16inptr8;
static void     *_sb16inlim8;
static void     *_sb16outptr8;
static void     *_sb16outlim8;
static void     *_sb16inptr16;
static void     *_sb16inlim16;
static void     *_sb16outptr16;
static void     *_sb16outlim16;

#if 0
uint16_t
sb16ackint(void)
{
    uint16_t ret = inb(SB16RDBUFSTAT);

    return ret;
}
#endif

void
sb16setup(void)
{
    uint8_t u8val;

    /* probe IRQ */
    outb(SB16CONFIRQ, SB16MIXERADR);
    u8val = inb(SB16MIXERDATA);
    switch (u8val) {
        case SB16IRQ5BIT:
            _sb16irq = 5;

            break;
        case SB16IRQ7BIT:
            _sb16irq = 7;

            break;
        case SB16IRQ10BIT:
            _sb16irq = 10;

            break;
    }
    /* probe DMA */
    outb(SB16CONFDMA, SB16MIXERADR);
    u8val = inb(SB16MIXERDATA);
    switch (u8val & 0x0f) {
        case SB16DMA0BIT:
            _sb16dma8 = 0;

            break;
        case SB16DMA1BIT:
            _sb16dma8 = 1;

            break;
        case SB16DMA3BIT:
            _sb16dma8 = 3;

            break;
    }
    switch (u8val & 0xf0) {
        case SB16DMA5BIT:
            _sb16dma16 = 5;

            break;
        case SB16DMA6BIT:
            _sb16dma16 = 6;

            break;
        case SB16DMA7BIT:
            _sb16dma16 = 7;

            break;
    }
    kprintf("SB16 @ 0x%x, IRQ %d, DMA %d (8-bit), DMA %d (16-bit)\n",
            SB16BASE, _sb16irq, _sb16dma8, _sb16dma16);
    /* initialize interrupt management */
    irqvec[_sb16irq] = sb16intr;
    /* initialize DMA interface */
    _sb16dmabuf8 = dmabufadr(_sb16dma8);
    _sb16dmabuf16 = dmabufadr(_sb16dma16);
    dmasetmode(_sb16dma8, DMAAUTOINIT | DMAADRINCR | DMABLOCK);
    dmasetmode(_sb16dma16, DMAAUTOINIT | DMAADRINCR | DMABLOCK);
    dmasetadr(_sb16dma8, _sb16dmabuf8);
    dmasetadr(_sb16dma16, _sb16dmabuf16);
    dmasetcnt(_sb16dma8, SB16BUFSIZE >> 1);
    dmasetcnt(_sb16dma16, SB16BUFSIZE >> 1);
    /* set input and output rates */
    sbsetrate(SB16INPUTRATE, 44100);
    sbsetrate(SB16OUTPUTRATE, 44100);
    /* set block transfer size (16-bit words) */
    outw(SB16BUFSIZE >> 2, SB16SETBLKSIZE);
    _sb16init = 1;

    return;
}

void
sb16init(void)
{
    /* initialise and zero wired buffers */
    _sb16inbuf8 = kwalloc(SB16DATA8BUFSIZE);
    kbzero(_sb16inbuf8, SB16DATA8BUFSIZE);
    _sb16outbuf8 = kwalloc(SB16DATA8BUFSIZE);
    kbzero(_sb16outbuf8, SB16DATA8BUFSIZE);
    _sb16inbuf16 = kwalloc(SB16DATA16BUFSIZE);
    kbzero(_sb16inbuf16, SB16DATA16BUFSIZE);
    _sb16outbuf16 = kwalloc(SB16DATA16BUFSIZE);
    kbzero(_sb16outbuf16, SB16DATA16BUFSIZE);
    _sb16inptr8 = _sb16inbuf8;
    _sb16inlim8 = _sb16inbuf8 + SB16DATA8BUFSIZE - (SB16BUFSIZE >> 1);
    _sb16outptr8 = _sb16outbuf8;
    _sb16outlim8 = _sb16outbuf8 + SB16DATA8BUFSIZE - (SB16BUFSIZE >> 1);
    _sb16inptr16 = _sb16inbuf16;
    _sb16inlim16 = _sb16inbuf16 + SB16DATA16BUFSIZE - (SB16BUFSIZE >> 2);
    _sb16outptr16 = _sb16outbuf16;
    _sb16outlim16 = _sb16outbuf16 + SB16DATA16BUFSIZE - (SB16BUFSIZE >> 2);
    /* reset sound card */
    sb16reset();
    /* sleep for SB16RESETMS milliseconds, then trigger sb16setup() */
//    pitsleep(SB16RESETMS, sb16setup);
    usleep(SB16RESETMS * 1000);
    sb16setup();

    return;
}

void
sb16unload(void)
{
    kfree(_sb16inbuf8);
    kfree(_sb16outbuf8);
    kfree(_sb16inbuf16);
    kfree(_sb16outbuf16);

    return;
}

void
sb16filloutbuf8(void)
{
    ;
}

void
sb16filloutbuf16(void)
{
    ;
}

void
sb16flushinbuf8(void)
{
    ;
}

void
sb16flushinbuf16(void)
{
    ;
}

void
sb16intr(void)
{
    uint16_t reg;
    uint16_t stat;
    long     val;
    long     op = DMAWRITEOP;

    stat = inb(SB16RDBUFSTAT);
    if (stat & SB16BUFSTATBIT) {
        op = DMAREADOP;
    }
    outb(SB16INTRSTAT, SB16MIXERADR);
    stat = inb(SB16MIXERDATA);
    switch (stat) {
        case SB16DMA8MIDIBIT:
            reg = SB16DMA8MIDISTAT;
            val = DMAIOBUFSIZE >> 1;
            mtxlk(&_sb16buflock);
            if (op == DMAREADOP) {
                if (_sb16dmain8full) {
                    sb16flushinbuf8();
                }
                kmemcpy(_sb16inptr8, _sb16dmabuf8 + _sb16dmainofs8, val);
                _sb16dmainofs8 = (_sb16dmainofs8 + val) & (DMAIOBUFSIZE - 1);
                if (_sb16inptr8 < _sb16inlim8) {
                    _sb16inptr8 += val;
                    _sb16dmain8full ^= _sb16dmain8full;
                } else {
                    _sb16inptr8 = _sb16inbuf8;
                    _sb16dmain8full = 1;
                }
            } else {
                if (_sb16dmaout8empty) {
                    sb16filloutbuf8();
                }
                kmemcpy(_sb16dmabuf8 + _sb16dmaoutofs8, _sb16outptr8, val);
                _sb16dmaoutofs8 = (_sb16dmaoutofs8 + val) & (DMAIOBUFSIZE - 1);
                if (_sb16outptr8 < _sb16outlim8) {
                    _sb16outptr8 += val;
                    _sb16dmaout8empty ^= _sb16dmaout8empty;
                } else {
                    _sb16outptr8 = _sb16outbuf8;
                    _sb16dmaout8empty = 1;
                }
            }
            mtxunlk(&_sb16buflock);

            break;
        case SB16DMA16BIT:
            reg = SB16DMA16STAT;
            val = DMAIOBUFSIZE >> 2;
            mtxlk(&_sb16buflock);
            if (op == DMAREADOP) {
                if (_sb16dmain16full) {
                    sb16flushinbuf16();
                }
                kmemcpy(_sb16inptr16, _sb16dmabuf16 + _sb16dmainofs16, val);
                _sb16dmainofs16 = (_sb16dmainofs16 + val) & (DMAIOBUFSIZE - 1);
                if (_sb16inptr16 < _sb16inlim16) {
                    _sb16inptr16 += val;
                    _sb16dmain16full ^= _sb16dmain16full;
                } else {
                    _sb16inptr16 = _sb16inbuf16;
                    _sb16dmain16full = 1;
                }
            } else {
                if (_sb16dmaout16empty) {
                    sb16filloutbuf16();
                }
                kmemcpy(_sb16dmabuf16 + _sb16dmaoutofs16, _sb16outptr16, val);
                _sb16dmaoutofs16 = (_sb16dmaoutofs16 + val) & (DMAIOBUFSIZE - 1);
                if (_sb16outptr16 < _sb16outlim16) {
                    _sb16outptr16 += val;
                    _sb16dmaout16empty ^= _sb16dmaout16empty;
                } else {
                    _sb16outptr16 = _sb16outbuf16;
                    _sb16dmaout16empty = 1;
                }
            }
            mtxunlk(&_sb16buflock);

            break;
        case SB16MPU401BIT:
            reg = SB16MPU401STAT;

            break;
    }
    /* acknowledge IRQ */
    val = inb(reg);
}

/* see sb16.h for the reg parameter (0x30..0x3b) */
void
sb16setvol(uint16_t reg, uint8_t val)
{
    val = max(val, SB16MAXVOL);
    outb(reg, SB16MIXERADR);
    outb(val, SB16MIXERDATA);

    return;
}

/* cmd is SB16INPUTRATE or SB16OUTPUTRATE */
void
sbsetrate(long cmd, uint16_t hz)
{
    outb(cmd, SB16WRITE);
    outb(hz & 0xff, SB16WRITE);
    outb((hz >> 8) & 0xff, SB16WRITE);

    return;
}

