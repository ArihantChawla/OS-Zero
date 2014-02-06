#ifndef __UNIT_X86_APIC_H__
#define __UNIT_X86_APIC_H__

#include <kern/conf.h>

#if (SMP)

#include <stdint.h>
#include <kern/unit/ia32/link.h>

extern volatile uint32_t *mpapic;

void apicinit(long core);
void apicstart(uint8_t core, uint32_t adr);

/* initialisation */
#define RTCBASE        0x70
#define BIOSWRV        0x467            // warm reset vector

/* registers */
#define APICID         0x0020
#define APICVER        0x0030
#define APICTASKPRIO   0x0080
#define APICEOI        0x00b0
#define APICLDR        0x00d0
#define APICSPURIOUS   0x00f0
#define APICERRSTAT    0x0280
#define APICINTLO      0x0300
#define APICINTHI      0x0310
#define APICTIMER      0x0320           // timer
#define APICTHERMAL    0x0330           // thermal sensor interrupt
#define APICPERFINT    0x0340           // performance counter overflow
#define APICLINT0      0x0350
#define APICLINT1      0x0360           // NMI
#define APICERROR      0x0370
#define APICTMRINITCNT 0x0380
#define APICTMRCURCNT  0x0390
#define APICTMRDIVCONF 0x03e0
#define APICLAST       0x038f
//#define APICENABLE     0x0100
#define APICCPUFOCUS   0x0200
#define APICNMI        (4 << 8)
/* timer configuration */
#define APICDISABLE    0x00010000
//#define APICDIV1       0x00000008
#define APICBASEDIV    0x00100000
#define APICPERIODIC   0x00020000

/* flags */
#define APICENABLE     0x00000100
#define APICINIT       0x00000500
#define APICSTART      0x00000600
#define APICDELIVS     0x00001000
#define APICASSERT     0x00004000
#define APICDEASSERT   0x00000000
#define APICLEVEL      0x00008000
#define APICBCAST      0x00080000
#define APICPENDING    0x00001000
#define APICFIXED      0x00000000
#define APICSMI        0x00000200
#define APICEXTINT     0x00000700
#define APICMASKED     0x00010000
#define APICSELF       0x00040000
#define APICBUTSELF    0x00080000
#define APICALL        0x000c0000

static __inline__ uint32_t
apicread(uint32_t reg)
{
    uint32_t ret = mpapic[reg >> 2];

    return ret;
}

/* wait for write to finish by reading ID */
static __inline__ void
apicwrite(uint32_t val, uint32_t reg)
{
    mpapic[(reg) >> 2] = val;
    apicread(APICID);
}

static __inline__ void
apicsendirq(uint32_t hi, uint32_t lo, long nusec)
{
    long cnt = nusec << 4;
    __asm__ __volatile__ ("pushfl\n");
    __asm__ __volatile__ ("cli\n");
    apicwrite(hi, APICINTHI);
    while (cnt--) {
        ;
    }
    apicwrite(lo, APICINTLO);
    cnt = nusec << 4;
    while (cnt--) {
        ;
    }
    __asm__ __volatile__ ("sti\n");
    __asm__ __volatile__ ("popfl\n");
}

#if 0
static __inline__ void
apicsendirq(uint32_t hi, uint32_t lo, long nusec)
{
    __asm__ __volatile__ ("pushfl\n");
    __asm__ __volatile__ ("cli\n");
    apicwrite(hi, APICINTHI);
    usleep(nusec);
    apicwrite(lo, APICINTLO);
    __asm__ __volatile__ ("sti\n");
    __asm__ __volatile__ ("popfl\n");
}
#endif

#endif /* SMP */

#endif /* __UNIT_X86_APIC_H__ */

