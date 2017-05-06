#ifndef __KERN_IO_DRV_PC_PCI_H__
#define __KERN_IO_DRV_PC_PCI_H__

#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>

long     pciprobe(void);
uint16_t pcireadconfw(uint8_t busid, uint8_t slotid,
                      uint8_t funcid, uint8_t ofs);
int      pcireadconf1(uint8_t busid, uint8_t slotid, uint8_t funcid,
                      uint16_t regid, uint8_t len);
void     pciwriteconf1(uint8_t busid, uint8_t slotid, uint8_t funcid,
                       uint16_t regid, uint8_t len, uint32_t val);
int      pcireadconf2(uint8_t busid, uint8_t slotid, uint8_t funcid,
                      uint16_t regid, uint8_t len);
void     pciwriteconf2(uint8_t busid, uint8_t slotid, uint8_t funcid,
                       uint16_t regid, uint8_t len, uint8_t val);

#define pciconfadr1(bus, slot, func, reg)                               \
    (PCICONFBIT                                                         \
     | (((reg) & 0xf00) << 16)                                          \
     | ((bus) << 16)                                                    \
     | ((slot) << 11)                                                   \
     | ((func) << 8)                                                    \
     | ((reg) & 0xfc))
#define pciconfadr2(slot, reg)                                          \
    ((uint16_t)(0xc000 | ((slot) << 8) | (reg)))

#define PCINBUS       256       // maximum number of buses
#define PCINSLOT      32        // maximum number of slots on a bus
#define PCINDEV       64
#define PCICONFADR1   0x0cf8
#define PCICONFADR2   0x0cfa
#define PCICONFDATA   0x0cfc

#define PCIRES1MASK   0x00000003
#define PCIREGMASK    0x000000fc
#define PCIFUNCMASK   0x00000700
#define PCIDEVMASK    0x0000f800
#define PCIBUSMASK    0x00ff0000
#define PCIRES2MASK   0x7f000000
#define PCICONFBIT    0x80000000

#define PCIREADCONFB  0xb108
#define PCIREADCONFW  0xb109
#define PCIREADCONFL  0xb10a
#define PCIWRITECONFB 0xb10b
#define PCIWRITECONFW 0xb10c
#define PCIWRITECONFL 0xb10d

/* class codes */
#define PCINOCLASS    0x00
#define PCISTORAGE    0x01
#define PCINETWORK    0x02
#define PCIDISPLAY    0x03
#define PCIMULTIMEDIA 0x04
#define PCIMEMORY     0x05
#define PCIBRIDGE     0x06
#define PCICOMM       0x07
#define PCIPERIPHERAL 0x08
#define PCIINPUT      0x09
#define PCIDOCK       0x0a
#define PCIPROCESSOR  0x0b
#define PCISERIALBUS  0x0c
#define PCIWIRELESS   0x0d
#define PCIIOCTRL     0x0e
#define PCISATELLITE  0x0f
#define PCICRYPT      0x10
#define PCIDATASIGNAL 0x11

struct pcidev {
//    struct pcidev *prev;
//    struct pcidev *next;
    char          *str;
    long           ndx;
    uint16_t       vendor;
    uint16_t       id;
    uint8_t        bus;
    uint8_t        slot;
    uint8_t        irq;
//    uint8_t        dma;
    /* DMA buffers */
    long           inbufsz;
    void          *inbuf;
    long           outbufsz;
    void          *outbuf;
};

struct pcidevlist {
    volatile long  lk;
    struct pcidev *head;
    struct pcidev *tail;
};

struct pcihdr00 {
    uint16_t vendor;
    uint16_t dev;
    uint16_t cmd;
    uint16_t status;
    uint8_t  rev;
    uint8_t  progif;
    uint8_t  subclass;
    uint8_t  class;
    uint8_t  clsize;
    uint8_t  lattmr;
    uint8_t  hdrtype;
    uint8_t  bist;
    uint32_t bar0;
    uint32_t bar1;
    uint32_t bar2;
    uint32_t bar3;
    uint32_t bar4;
    uint32_t bar5;
    uint32_t cbcis;
    uint16_t subvendor;
    uint16_t subsys;
    uint32_t romadr;
    uint8_t  cap;
    uint8_t  res1;
    uint16_t res2;
    uint32_t res3;
    uint8_t  intrline;
    uint8_t  intrpin;
    uint8_t  mingrant;
    uint8_t  maxlat;
};

struct pcihdr01 {
    uint16_t vendor;
    uint16_t dev;
    uint16_t cmd;
    uint16_t status;
    uint8_t  rev;
    uint8_t  progif;
    uint8_t  subclass;
    uint8_t  class;
    uint32_t bar0;
    uint32_t bar1;
    uint8_t  primbus;
    uint8_t  secbus;
    uint8_t  subbus;
    uint8_t  seclattmr;
    uint8_t  iobase;
    uint8_t  iolim;
    uint16_t secstatus;
    uint16_t membase;
    uint16_t memlim;
    uint16_t prefmemlo;
    uint16_t preflimlo;
    uint32_t prefmemhi;
    uint32_t preflimhi;
    uint16_t iobasehi;
    uint16_t iolimhi;
    uint8_t  cap;
    uint8_t  res1;
    uint16_t res2;
    uint32_t romadr;
    uint8_t  intrline;
    uint8_t  intrpin;
    uint16_t bridgectl;
};

/* TODO: implement header 02 (pci-to-cardbus bridge) */

struct pcihdrtype {
    unsigned type : 7;
    unsigned mf   : 1;
};

struct pcibist {
    unsigned cmplcode  : 4;
    unsigned res       : 2;
    unsigned startbist : 1;
    unsigned bistcap   : 1;
};

struct pcicmd {
    unsigned iospace      : 1;
    unsigned memspace     : 1;
    unsigned busmaster    : 1;
    unsigned speccycles   : 1;
    unsigned wrinvldenab  : 1;
    unsigned vgapalsnoop  : 1;
    unsigned parity       : 1;
    unsigned res1         : 1;
    unsigned serrenab     : 1;
    unsigned fastbtobenab : 1;
    unsigned intrdisab    : 1;
    unsigned res2         : 1;
};

struct pcistatus {
    unsigned res1      : 3;
    unsigned intrstat  : 1;
    unsigned cap       : 1;
    unsigned mhz66     : 1;
    unsigned res2      : 1;
    unsigned fastbtob  : 1;
    unsigned masterpar : 1;
    unsigned devsel    : 2;
    unsigned sigabort  : 1;
    unsigned targabort : 1;
    unsigned mastabort : 1;
    unsigned syserr    : 1;
    unsigned parityerr : 1;
};

struct pcibaseadr {
    unsigned zero     : 1;
    unsigned type     : 2;
    unsigned prefetch : 1;
    unsigned adr      :28;
};

struct pciiobaseadr {
    unsigned one : 1;
    unsigned res : 1;
    unsigned adr : 30;
};

struct pcidrv {
    long type;
    long ndev;
};

typedef void pciinitfunc_t(struct pcidev *dev);

struct pcidrvent {
    pciinitfunc_t *init;
    const char    *str;
    long           devid;
};

extern struct pcidev pcidevtab[PCINDEV];
extern long          pcindev;

#endif /* __KERN_IO_DRV_PC_PCI_H__ */

