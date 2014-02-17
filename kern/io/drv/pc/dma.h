#ifndef __UNIT_X86_DMA_H__
#define __UNIT_X86_DMA_H__

#include <stdint.h>
#include <sys/io.h>
#include <kern/unit/ia32/vm.h>

#define dmabufadr(chan) ((uint8_t *)(DMABUFBASE + (chan) * DMACHANBUFSIZE))

extern const uint8_t dmapageports[];

#define DMADRAMCHAN     0       // reserved (DRAM refresh)
#define DMAFDCHAN       2       // hardwired channel for floppy disk controllers
#define DMACASCADECHAN  4       // reserved (cascade)

#define DMAIDLE         0
#define DMAREADOP       1
#define DMAWRITEOP      2

#define DMANCHAN        8
#define DMACHANBUFSIZE  (1024 * 1024)
#define DMAIOBUFSIZE    65536

#define DMABUFBASE      0x00400000U
#define DMABUFSIZE      (DMANCHAN * DMACHANBUFSIZE)
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

/* page registers for high byte (23-16) */
#define DMAPAGE0        0x87U
#define DMAPAGE1        0x83U
#define DMAPAGE2        0x81U
#define DMAPAGE3        0x82U
#define DMAPAGE4        0x8fU
#define DMAPAGE5        0x8bU
#define DMAPAGE6        0x89U
#define DMAPAGE7        0x8aU
/* low byte page refresh */
#define DMAPAGEREFRESH  0x8fU

/* operation modes */
#define DMAREAD         0x44U
#define DMAWRITE        0x48U
#define DMAAUTOINIT     0x10U
#define DMAADRINCR      0x20U
#define DMASINGLE       0x40U
#define DMABLOCK        0x80U
#define DMACASCADE      0xc0U

#define DMACHANMASK     0x03
#define DMA2BIT         0x04
#define DMAMASKBIT      0x04

//#define _isdma1(c)     (!(~((c) & DMACHANMASK)))
#define _isdma1(c)      (!((c) & (~DMACHANMASK)))

static __inline__ void
dmaunmask(uint8_t chan)
{
    if (_isdma1(chan)) {
	outb(chan, DMA1MASK);
    } else {
	outb(chan & DMACHANMASK, DMA2MASK);
    }
}

static __inline__ void
dmamask(uint8_t chan)
{
    if (_isdma1(chan)) {
	outb(chan | DMAMASKBIT, DMA1MASK);
    } else {
	outb((chan & DMACHANMASK) | DMAMASKBIT, DMA2MASK);
    }
}

static __inline__ void
dmaclrptr(uint8_t chan)
{
    if (_isdma1(chan)) {
	outb(0x00, DMA1CLRPTR);
    } else {
	outb(0x00, DMA2CLRPTR);
    }
}

static __inline__ void
dmasetmode(uint8_t chan, uint8_t mode)
{
    if (_isdma1(chan)) {
	outb(mode | chan, DMA1MODE);
    } else {
	outb(mode | (chan & DMACHANMASK), DMA2MODE);
    }
}

static __inline__ void
dmasetpage(uint8_t chan, uint8_t page)
{
    if (_isdma1(chan)) {
	outb(chan, dmapageports[page]);
    } else {
	outb(chan & 0xfe, dmapageports[page]);
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
	outb(adr & 0xff, port);
	outb((adr >> 8) & 0xff, port);
    } else {
	port = DMA1BASE + ((chan & DMACHANMASK) << 2);
	outb((adr >> 1) & 0xff, port);
	outb((adr >> 9) & 0xff, port);
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
	outb(cnt & 0xff, port);
	outb((cnt >> 8) & 0xff, port);
    } else {
	port = DMA1BASE + ((chan & DMACHANMASK) << 1) + 2;
	outb((cnt >> 1) & 0xff, port);
	outb((cnt >> 9) & 0xff, port);
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

#endif /* __UNIT_X86_DMA_H__ */

