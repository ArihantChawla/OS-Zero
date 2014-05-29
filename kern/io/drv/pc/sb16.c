#include <stddef.h>
#include <stdint.h>
#include <sys/io.h>

#include <zero/param.h>
#include <zero/cdecl.h>
#include <zero/mtx.h>
#include <zero/trix.h>

#include <kern/util.h>
#include <kern/tmr.h>
#include <kern/unit/ia32/vm.h>
#include <kern/mem.h>
//#include <kern/prio.h>
//#include <kern/thr.h>
#include <kern/io/drv/pc/dma.h>
#include <kern/io/drv/pc/sb16.h>

void sb16intr(void);
void sb16setrate(long cmd, uint16_t hz);
void sb16setvol(uint16_t reg, uint8_t val);
//extern void pitsleep(long msec, void (*func)(void));

extern void *irqvec[];

/* intpu and output buffers are allocated from DMA zone according to channels */
#define SB16DATA8BUFSIZE  (8 * SB16BUFSIZE)
#define SB16DATA16BUFSIZE (16 * SB16BUFSIZE)

static struct sb16drv sb16drv ALIGNED(PAGESIZE);

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
            sb16drv.irq = 5;

            break;
        case SB16IRQ7BIT:
            sb16drv.irq = 7;

            break;
        case SB16IRQ10BIT:
            sb16drv.irq = 10;

            break;
    }
    /* probe DMA */
    outb(SB16CONFDMA, SB16MIXERADR);
    u8val = inb(SB16MIXERDATA);
    switch (u8val & 0x0f) {
        case SB16DMA0BIT:
            sb16drv.dma8 = 0;

            break;
        case SB16DMA1BIT:
            sb16drv.dma8 = 1;

            break;
        case SB16DMA3BIT:
            sb16drv.dma8 = 3;

            break;
    }
    dmatakechan(sb16drv.dma8);
    switch (u8val & 0xf0) {
        case SB16DMA5BIT:
            sb16drv.dma16 = 5;

            break;
        case SB16DMA6BIT:
            sb16drv.dma16 = 6;

            break;
        case SB16DMA7BIT:
            sb16drv.dma16 = 7;

            break;
    }
    dmatakechan(sb16drv.dma16);
    kprintf("SB16 @ 0x%x, IRQ %d, DMA %d (8-bit), DMA %d (16-bit)\n",
            SB16BASE, sb16drv.irq, sb16drv.dma8, sb16drv.dma16);
    /* initialize DMA interface */
    /* reserve buffers from DMA zone */
    sb16drv.dmabuf8 = dmabufadr(sb16drv.dma8);
    sb16drv.dmabuf16 = (uint16_t *)dmabufadr(sb16drv.dma16);
    /* initialise DMA transfer environment */
    dmasetmode(sb16drv.dma8, DMAAUTOINIT | DMAADRINCR | DMABLOCK);
    dmasetmode(sb16drv.dma16, DMAAUTOINIT | DMAADRINCR | DMABLOCK);
    dmasetadr(sb16drv.dma8, sb16drv.dmabuf8);
    dmasetadr(sb16drv.dma16, sb16drv.dmabuf16);
    dmasetcnt(sb16drv.dma8, SB16BUFSIZE >> 1);
    dmasetcnt(sb16drv.dma16, SB16BUFSIZE >> 1);
    /* set input and output rates */
    sb16setrate(SB16INPUTRATE, 44100);
    sb16setrate(SB16OUTPUTRATE, 44100);
    /* TODO: set mixer volumes */
    sb16setvol(SB16MASTERLVOL, SB16MAXVOL >> 1);
    sb16setvol(SB16MASTERRVOL, SB16MAXVOL >> 1);
    /* set block transfer size (16-bit words) */
    outw(SB16BUFSIZE >> 2, SB16SETBLKSIZE);
    sb16drv.init = 1;
    /* initialize interrupt management */
    irqvec[sb16drv.irq] = sb16intr;

    return;
}

void
sb16init(void)
{
    sb16drv.dmaout8empty = 1;
    sb16drv.dmaout16empty = 1;
    /* reset sound card */
    sb16reset();
    /* sleep for SB16RESETMS milliseconds, then trigger sb16setup() */
    usleep(SB16RESETMS * 1000);
    sb16setup();
    /* initialise and zero wired buffers */
#if 0
    sb16drv.inbuf8 = kwalloc(SB16DATA8BUFSIZE);
    kbzero(sb16drv.inbuf8, SB16DATA8BUFSIZE);
    sb16drv.outbuf8 = kwalloc(SB16DATA8BUFSIZE);
    kbzero(sb16drv.outbuf8, SB16DATA8BUFSIZE);
    sb16drv.inbuf16 = kwalloc(SB16DATA16BUFSIZE);
    kbzero(sb16drv.inbuf16, SB16DATA16BUFSIZE);
    sb16drv.outbuf16 = kwalloc(SB16DATA16BUFSIZE);
    kbzero(sb16drv.outbuf16, SB16DATA16BUFSIZE);
#endif
    vmmapseg((uint32_t *)&_pagetab,
             (uint32_t)dmabufadr(sb16drv.dma8),
             (uint32_t)dmabufadr(sb16drv.dma8),
             (uint32_t)dmabufadr(sb16drv.dma8) + SB16DATA8BUFSIZE,
             PAGEPRES | PAGEWRITE | PAGENOCACHE);
    vmmapseg((uint32_t *)&_pagetab,
             (uint32_t)dmabufadr(sb16drv.dma16),
             (uint32_t)dmabufadr(sb16drv.dma16),
             (uint32_t)dmabufadr(sb16drv.dma16) + SB16DATA16BUFSIZE,
             PAGEPRES | PAGEWRITE | PAGENOCACHE);
    sb16drv.inbuf8 = dmabufadr(sb16drv.dma8);
    kbzero(sb16drv.inbuf8, SB16DATA8BUFSIZE);
    sb16drv.outbuf8 = dmabufadr(sb16drv.dma8) + (DMACHANBUFSIZE >> 1);
    kbzero(sb16drv.outbuf8, SB16DATA8BUFSIZE);
    sb16drv.inbuf16 = (uint16_t *)dmabufadr(sb16drv.dma16);
    kbzero(sb16drv.inbuf16, SB16DATA16BUFSIZE);
    sb16drv.outbuf16 = (uint16_t *)dmabufadr(sb16drv.dma16)
        + (DMACHANBUFSIZE >> 2);
    kbzero(sb16drv.outbuf16, SB16DATA16BUFSIZE);
    sb16drv.inptr8 = sb16drv.inbuf8;
    sb16drv.inlim8 = sb16drv.inbuf8 + SB16DATA8BUFSIZE - (SB16BUFSIZE >> 1);
    sb16drv.outptr8 = sb16drv.outbuf8;
    sb16drv.outlim8 = sb16drv.outbuf8 + SB16DATA8BUFSIZE - (SB16BUFSIZE >> 1);
    sb16drv.inptr16 = sb16drv.inbuf16;
    sb16drv.inlim16 = sb16drv.inbuf16 + SB16DATA16BUFSIZE - (SB16BUFSIZE >> 2);
    sb16drv.outptr16 = sb16drv.outbuf16;
    sb16drv.outlim16 = sb16drv.outbuf16
        + SB16DATA16BUFSIZE - (SB16BUFSIZE >> 2);

    return;
}

void
sb16unload(void)
{
#if 0
    kfree(sb16drv.inbuf8);
    kfree(sb16drv.outbuf8);
    kfree(sb16drv.inbuf16);
    kfree(sb16drv.outbuf16);
#endif

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
    uint16_t reg = 0;
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
            mtxlk(&sb16drv.buflock);
            if (op == DMAREADOP) {
                if (sb16drv.dmain8full) {
                    sb16flushinbuf8();
                }
                kmemcpy(sb16drv.inptr8,
                        sb16drv.dmabuf8 + sb16drv.dmainofs8,
                        val);
                sb16drv.dmainofs8 = (sb16drv.dmainofs8 + val)
                    & (DMAIOBUFSIZE - 1);
                if (sb16drv.inptr8 < sb16drv.inlim8) {
                    sb16drv.inptr8 += val;
                    sb16drv.dmain8full = 0;
                } else {
                    sb16drv.inptr8 = sb16drv.inbuf8;
                    sb16drv.dmain8full = 1;
                }
            } else {
                if (sb16drv.dmaout8empty) {
                    sb16filloutbuf8();
                }
                kmemcpy(sb16drv.dmabuf8 + sb16drv.dmaoutofs8,
                        sb16drv.outptr8,
                        val);
                sb16drv.dmaoutofs8 = (sb16drv.dmaoutofs8 + val)
                    & (DMAIOBUFSIZE - 1);
                if (sb16drv.outptr8 < sb16drv.outlim8) {
                    sb16drv.outptr8 += val;
                    sb16drv.dmaout8empty = 0;
                } else {
                    sb16drv.outptr8 = sb16drv.outbuf8;
                    sb16drv.dmaout8empty = 1;
                }
            }
            mtxunlk(&sb16drv.buflock);

            break;
        case SB16DMA16BIT:
            reg = SB16DMA16STAT;
            val = DMAIOBUFSIZE >> 2;
            mtxlk(&sb16drv.buflock);
            if (op == DMAREADOP) {
                if (sb16drv.dmain16full) {
                    sb16flushinbuf16();
                }
                kmemcpy(sb16drv.inptr16,
                        sb16drv.dmabuf16 + sb16drv.dmainofs16,
                        val);
                sb16drv.dmainofs16 = (sb16drv.dmainofs16 + val)
                    & (DMAIOBUFSIZE - 1);
                if (sb16drv.inptr16 < sb16drv.inlim16) {
                    sb16drv.inptr16 += val;
                    sb16drv.dmain16full = 0;
                } else {
                    sb16drv.inptr16 = sb16drv.inbuf16;
                    sb16drv.dmain16full = 1;
                }
            } else {
                if (sb16drv.dmaout16empty) {
                    sb16filloutbuf16();
                }
                kmemcpy(sb16drv.dmabuf16 + sb16drv.dmaoutofs16,
                        sb16drv.outptr16,
                        val);
                sb16drv.dmaoutofs16 = (sb16drv.dmaoutofs16 + val)
                    & (DMAIOBUFSIZE - 1);
                if (sb16drv.outptr16 < sb16drv.outlim16) {
                    sb16drv.outptr16 += val;
                    sb16drv.dmaout16empty = 0;
                } else {
                    sb16drv.outptr16 = sb16drv.outbuf16;
                    sb16drv.dmaout16empty = 1;
                }
            }
            mtxunlk(&sb16drv.buflock);

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
    val = min(val, SB16MAXVOL);
    outb(reg, SB16MIXERADR);
    outb(val, SB16MIXERDATA);

    return;
}

/* cmd is SB16INPUTRATE or SB16OUTPUTRATE */
void
sb16setrate(long cmd, uint16_t hz)
{
    outb(cmd, SB16WRITE);
    outb(hz & 0xff, SB16WRITE);
    outb((hz >> 8) & 0xff, SB16WRITE);

    return;
}

