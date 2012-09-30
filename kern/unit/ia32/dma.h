#ifndef __IA32_DMA_H__
#define __IA32_DMA_H__

#include <stdint.h>
#include <kern/ia32/io.h>

#define dmagetbuf(chan) ((void *)(DMABUFBASE + (chan) * DMACHANBUFSIZE))

extern const uint8_t dmapageports[];

#define DMAIDLE         0
#define DMAREADOP       1
#define DMAWRITEOP      2

#define DMANCHAN        8
#define DMACHANBUFSIZE  (512 * 1024)
#define DMAIOBUFSIZE    65536

#define DMABUFBASE      0x00400000U
#define DMABUFSIZE      0x00400000U
//#define DMACBUFSIZE     (1U << DMACBUFSIZELOG2)
#define DMABUFNPAGE     (DMACHANBUFSIZE >> PAGESIZELOG2)
//#define DMACBUFSIZELOG2 17

/* 8237 DMA controllers */
#define DMA1BASE        0x00U
#define DMA2BASE        0xc0U

/* controller 1 registers */
#define DMA1CMD         0x08U   // command register
#define DMA1STAT        0x08U   // status register
#define DMA1REQ         0x09U   // request register
#define DMA1MASK        0x0aU   // single mask register bit
#define DMA1MODE        0x0bU   // mode register
#define DMA1CLRPTR      0x0cU   // clear LSB/MSB flip-flop
#define DMA1TEMP        0x0dU   // temporary register (not present in 82374)
#define DMA1RESET       0x0dU   // master clear/reset
#define DMA1CLRMASK     0x0eU   // clear mask register
#define DMA1MASKALL     0x0fU   // read/write all mask register bits (82374)

/* controller 2 registers */
#define DMA2CMD         0xd0U
#define DMA2STAT        0xd0U
#define DMA2REQ         0xd2U
#define DMA2MASK        0xd4U
#define DMA2MODE        0xd6U
#define DMA2CLRPTR      0xd8U
#define DMA2TEMP        0xdaU
#define DMA2RESET       0xdaU
#define DMA2CLRMASK     0xdcU
#define DMA2MASKALL     0xdeU

/* address registers */
#define DMAADR0         0x00U
#define DMAADR1         0x02U
#define DMAADR2         0x04U
#define DMAADR3         0x06U
#define DMAADR4         0xc0U
#define DMAADR5         0xc4U
#define DMAADR6         0xc8U
#define DMAADR7         0xccU

/* count registers */
#define DMACNT0         0x01U
#define DMACNT1         0x03U
#define DMACNT2         0x05U
#define DMACNT3         0x07U
#define DMACNT4         0xc2U
#define DMACNT5         0xc6U
#define DMACNT6         0xcaU
#define DMACNT7         0xceU

/* page registers for low byte (23-16) */
#define DMAPAGE0        0x87U
#define DMAPAGE1        0x83U
#define DMAPAGE2        0x81U
#define DMAPAGE3        0x82U
#define DMAPAGE5        0x8bU
#define DMAPAGE6        0x89U
#define DMAPAGE7        0x8aU
/* low byte page refresh */
#define DMAPAGEREFR     0x8fU

/* operation modes */
#define DMAREAD         0x44U
#define DMAWRITE        0x48U
#define DMAAUTOINIT     0x10U
#define DMAADRINCR      0x20U
#define DMASINGLE       0x40U
#define DMABLOCK        0x80U
#define DMACASCADE      0xc0U

#if 0
/*
 * channel structure
 * num    - channel number
 * buf    - I/O buffer
 * bufsz  - buffer size in bytes
  */
struct m_dmachan {
    long      num;
    void     *buf;
    size_t    bufsz;
};
#endif

#if 0
struct m_iodev {
    /* DMA channel number, -1 means PIO */
    long           dma;
    /*
     * iochans - device I/O channel table
     * niochan - number of channels in iochans
     */
    uint16_t      *iochans;
    long           niochan;
    /* system call interface */
    long          (*open)(char *, long, long);
    long long     (*seek)(long, long long, long);
    unsigned long (*read)(long, void *, unsigned long);
    unsigned long (*write)(long, void *, unsigned long);
}
#endif

#define DMACHANMASK    0x03
#define DMA2BIT        0x04
#define DMAMASKBIT     0x04

#define _isdma1(c)     (!(~((c) & DMACHANMASK)))

static __inline__ void
dmaunmask(uint8_t chan)
{
    if (_isdma1(chan)) {
	outb(DMACHANMASK, chan);
    } else {
	outb(DMA2MASK, chan & DMACHANMASK);
    }
}

static __inline__ void
dmamask(uint8_t chan)
{
    if (_isdma1(chan)) {
	outb(DMACHANMASK, chan | DMAMASKBIT);
    } else {
	outb(DMA2MASK, (chan & DMACHANMASK) | DMAMASKBIT);
    }
}

static __inline__ void
dmaclrptr(uint8_t chan)
{
    if (_isdma1(chan)) {
	outb(DMA1CLRPTR, 0);
    } else {
	outb(DMA2CLRPTR, 0);
    }
}

static __inline__ void
dmasetmode(uint8_t chan, uint8_t mode)
{
    if (_isdma1(chan)) {
	outb(DMA1MODE, mode | chan);
    } else {
	outb(DMA1MODE, mode | (chan & DMACHANMASK));
    }
}

static __inline__ void
dmasetpage(uint8_t chan, uint8_t page)
{
    if (_isdma1(chan)) {
	outb(dmapageports[page], chan);
    } else {
	outb(dmapageports[page], chan & 0xfe);
    }
}

static __inline__ void
dmasetadr(uint8_t chan, void *ptr)
{
    uint16_t port;
    uint32_t adr = (uint32_t)ptr;

    dmasetpage(chan, adr >> 16);
    if (_isdma1(chan)) {
	port = DMA1BASE + ((chan & DMACHANMASK) << 1);
	outb(port, adr & 0xff);
	outb(port, (adr >> 8) & 0xff);
    } else {
	port = DMA1BASE + ((chan & DMACHANMASK) << 2);
	outb(port, (adr >> 1) & 0xff);
	outb(port, (adr >> 9) & 0xff);
    }
}

/*
 * NOTE: the number of transfers is one higher than the initial count.
 */
static __inline__ void
dmasetcnt(uint8_t chan, uint32_t cnt)
{
    uint16_t port;

    cnt--;
    if (_isdma1(chan)) {
	port = DMA1BASE + ((chan & DMACHANMASK) << 1) + 1;
	outb(port, cnt & 0xff);
	outb(port, (cnt >> 8) & 0xff);
    } else {
	port = DMA1BASE + ((chan & DMACHANMASK) << 1) + 2;
	outb(port, (cnt >> 1) & 0xff);
	outb(port, (cnt >> 9) & 0xff);
    }
}

static __inline__ uint32_t
dmagetcnt(uint8_t chan)
{
    uint16_t port;
    uint32_t cnt;
    uint32_t tmp;

    if (_isdma1(chan)) {
	port = DMA1BASE + ((chan & DMACHANMASK) << 1) + 1;
    } else {
	port = DMA1BASE + ((chan & DMACHANMASK) << 1) + 2;
    }
    cnt = inb(port);
    tmp = inb(port);
    cnt++;
    cnt += tmp << 8;
    if (_isdma1(chan)) {

	return cnt;
    } else {

	return (cnt << 1);
    }
}

/* invalidate cache for buffers of chan */
static __inline__ void
dmainvlbuf(uint8_t chan)
{
    uint8_t *pg;
    long     n;

    pg = (uint8_t *)(DMABUFBASE + chan * DMACHANBUFSIZE);
    n = DMABUFNPAGE;
    while (n--) {
        vmflushtlb(pg);
        pg += PAGESIZE;
    }

    return;
}

#endif /* __IA32_DMA_H__ */

