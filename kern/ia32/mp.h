#ifndef __KERN_IA32_MP_H__
#define __KERN_IA32_MP_H__

#include <kern/conf.h>

#if (SMP)

#define MPENTRY     0x9f000
#define MPENTRYSTK  0x9e000
#define MPGDT       0x9e000
#define MPSTKSIZE   16384
#define NMPGDT      3

#if !defined(__ASSEMBLY__)

#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <kern/ia32/apic.h>
#include <kern/ia32/cpu.h>
#include <kern/ia32/mp.h>

void mpinit(void);

extern volatile long mpmultiproc;

struct mp {
    unsigned char  signature[4];        // "_MP_"
    void          *conftab;             // physical address
    unsigned char  len;                 // 1
    unsigned char  rev;                 // 14
    unsigned char  chksum;              // header bytes add up to 0
    unsigned char  type;                // MP system config type
    unsigned char  intmode;
    unsigned char  res[3];
} PACK();

/* table entry types */
#define MPCPU    0x00                   // processor
#define MPBUS    0x01                   // bus
#define MPIOAPIC 0x02                   // I/O APIC
#define MPIOINTR 0x03                   // bus interrupt source
#define MPLINTR  0x04                   // system interrupt source

struct mpconf {
    unsigned char  signature[4];        // "PCMP"
    uint16_t       len;                 // table length
    unsigned char  ver;                 // 14
    unsigned char  chksum;              // header bytes add up to 0
    unsigned char  product[20];         // product ID
    uint32_t      *oemtab;              // OEM table pointer
    uint16_t       oemlen;              // OEM table length
    uint16_t       entry;               // entry count
    uint32_t      *apicadr;            // local APIC address
    uint16_t       xlen;                // extended table length
    unsigned char  xchksum;             // extended table checksum
    unsigned char  res;
} PACK();

/* flags */
#define MPCPUBOOT 0x02
struct mpcpu {
    unsigned char type;                 // entry type (0)
    unsigned char id;                   // local APIC ID
    unsigned char apicver;              // local APIC version
    unsigned char flags;                // CPU flags
    unsigned char signature[4];         // CPU signature
    uint32_t      featflg;              // feature flags from CPUID
    unsigned char res[8];
} PACK();

struct mpioapic {
    unsigned char  type;                // entry type (2)
    unsigned char  apicnum;             // I/O APIC ID
    unsigned char  apicver;             // I/O APIC version
    unsigned char  flags;               // I/O APIC flags
    uint32_t      *adr;                 // I/O APIC address
} PACK();

static __inline__ long
mpcpunum(void)
{
    long ret = 0;

    __asm__ __volatile__ ("pushfl\n");
    __asm__ __volatile__ ("cli\n");
    if (mpapic) {
        ret = apicread(APICID);
    }
    __asm__ __volatile__ ("sti\n");
    __asm__ __volatile__ ("popfl\n");

    return ret;
}

#endif /* !defined(__ASSEMBLY__) */

#endif /* SMP */

#endif /* __KERN_IA32_MP_H__ */

