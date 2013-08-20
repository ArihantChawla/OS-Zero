#ifndef __KERN_UNIT_X86_HPET_H__
#define __KERN_UNIT_X86_HPET_H__

#define HPETREGSIZE   1024
#define HPETREGSIZE64 65536
#define HPETSIG       0x54455048U       // "HPET"

/* configuration space register offsets */
#define hpettmrconfofs(id)                                              \
    (0x0100 + (id) * 0x20)
#define hpettmrcmpofs(id)                                               \
    (0x0108 + (id) * 0x20)
#define hpettmrofs(id)                                                  \
    (0x0110 + (id) * 0x20)
/* ID register contents */
/* revision */
#define hpetidrev(val)                                                  \
    ((val) & 0xff)
/* number of timers in block */
#define hpetidnumtmr(val)                                               \
    ((((val) >> 8) & 0x1f) + 1)
/* counter size; 0 -> 32-bit, 1 -> 64-bit */
#define hpetidcntsize(val)                                              \
    (((val) >> 13) & 0x01)
/* reserved bit (should be zero) */
#define hpetidres(val)                                                  \
    0x00004000
/* support for legacy interrupt route */
#define hpetidlegrt(val)                                                \
    (((val) >> 15) & 0x01)
/* vendor ID; must not be 0 */
#define hpetidvendor(val)                                               \
    (((val) >> 16) & 0xffff)
/*
 * counter increment period in femtoseconds (10^-15); must be <= 0x5f5e100
 * (== 100 nanoseconds)
 */
#define hpetidclkperiod(val)                                            \
    ((val) >> 32)
/* CONF register contents */
/*
 * route timer 0 to IRQ0 on non-APIC, IRQ2 in the I/O APIC if set
 * route timer 1 to IRQ8 on non-APIC; IRQ8 in the I/O APIC if set
 * route timers 2-n according to routing bits in configuration registers
 * if not set, honor routing bits for all timers
 */
/*
 * allow main counter to run and allow timer interrupts if enabled
 */
#define HPETCONFENABLE  0x0001
#define HPETCONFLEGRT   0x0002
/* all except the ID register are read-write */
#define HPETIDREG       0x0000
#define HPETCONFREG     0x0010
#define HPETINTSTATREG  0x0020
#define HPETMAINCNTREG  0x00f0
#define HPETTMR0CONFREG 0x0100
#define HPETTMR0CMPREG  0x0108
#define HPETTMR0FSBREG  0x0110
#define HPETTMR1CONFREG 0x0120
#define HPETTMR1CMPREG  0x0128
#define HPETTMR1FSBREG  0x0130
#define HPETTMR2CONFREG 0x0140
#define HPETTMR2CMPREG  0x0148
#define HPETTMR2FSBREG  0x0150

/* tmrblkid */
#define hpettmrid0(hdp)                                                 \
    ((hdp->tmrblkid) >> 16)
#define hpettmrlegrt(hdp)                                               \
    ((hdp)->tmrblkid & 0x8000)
#define hpettmrcntsz(hdp)                                               \
    ((hdp)->tmrblkid & 0x2000)
#define hpettmrncmp(hdp)                                                \
    (((hdp)->tmrblkid >> 8) & 0x1f)
#define hpethwrev(hdp)                                                  \
    ((hdp)->tmrblkid & 0xff)
#define hpetseqnum(hdp)                                                 \
    ((hdp)->hpetnum)
/* protoematr */
#define hpetpgprot(hdp)                                                 \
    ((hdp)->protoematr & 0x0f)
#define HPETNOPROT  0x00
#define HPET4KPROT  0x01
#define HPET64KPROT 0x02
/* acpiadr */
#define hpetadrmem(hdp)                                                 \
    (!(hpet->adrtype & 0xff))
#define hpetregsz(hdp)                                                  \
    ((hdp)->regsz)
#define hpetbifofs(hdp)                                                 \
    ((hdp)->bitofs)
#define hpetadr(hdp)                                                    \
    ((hdp)->baseadr)
struct hpet {
    uint32_t sig;               // HPETSIG ("HPET")
    uint32_t len;               // length in bytes
    uint8_t  rev;               // revision
    uint8_t  cksum;             // table must sum to zero
    uint8_t  oemid[6];          // OEM id
    uint64_t oemtabid;          // manufacturer model ID
    uint32_t oemrev;            // OEM revision
    uint32_t creatid;           // creator ID
    uint32_t creatrev;          // creator revision
    uint32_t tmrblkid;          // event timer block ID
    uint8_t  adrtype;           // ACPI address type -> 0 = memory, 1 = I/O
    uint8_t  regsz;             // register width in bits
    uint8_t  bitofs;            // register bit offset
    uint8_t  res;               // reserved
    uint64_t baseadr;           // 64-bit address
    uint8_t  hpetnum;           // sequence number; 0 = 1st table, ...
    uint16_t mintick;           // main counter minimum tick in periodic mode
    uint8_t  protoematr;
} PACK();

#endif /* __KERN_UNIT_X86_HPET_H__ */

