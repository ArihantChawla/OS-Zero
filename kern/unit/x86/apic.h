#ifndef __KERN_UNIT_X86_APIC_H__
#define __KERN_UNIT_X86_APIC_H__

#include <kern/conf.h>

#if (SMP) || (APIC)

#if !defined(__ASSEMBLER__)

#include <stdint.h>
#include <zero/param.h>
#include <zero/cdecl.h>
#include <kern/unit/x86/pit.h>
#include <kern/unit/x86/link.h>

extern volatile uint32_t *mpapic;

uint32_t apicinit(long cpuid);
void     ioapicinit(long id);
void     apicstart(uint8_t id, uint32_t adr);
void     kusleep(unsigned long nusec);

#define apiceoi()        apicwrite(0, APICEOI)

#endif /* !defined(__ASSEMBLER__) */

#define APICBASE         0xfee00000
#define APICMSR          0x1b
#define APICGLOBENABLE   0x00000800

/* registers */
#define APICID           0x0020
#define APICVER          0x0030
#define APICTASKPRIO     0x0080
#define APICEOI          0x00b0
#define APICLDR          0x00d0
#define APICDFR          0x00e0
#define APICSPURIOUS     0x00f0
#define APICERRSTAT      0x0280
#define APICINTRLO       0x0300
#define APICINTRHI       0x0310
#define APICTMR          0x0320         // timer
#define APICTHERMAL      0x0330         // thermal sensor interrupt
#define APICPERFINTR     0x0340         // performance counter overflow
#define APICLINTR0       0x0350
#define APICLINTR1       0x0360         // NMI
#define APICERROR        0x0370
#define APICTMRINITCNT   0x0380
#define APICTMRCURCNT    0x0390
#define APICTMRDIVCONF   0x03e0
#define APICLAST         0x038f
//#define APICENABLE     0x0100
#define APICCPUFOCUS     0x0200
#define APICNMI        (4 << 8)
/* timer configuration */
//#define APICDISABLE      0x00010000
//#define APICDIV1       0x00000008
#define APICBASEDIV      0x0000000b
#define APICPERIODIC     0x00020000
//#define APICMODEPERIODIC 0x02

/* flags */
#define APICSWENABLE     0x00000100
#define APICINIT         0x00000500
#define APICSTART        0x00000600
#define APICDELIVS       0x00001000
#define APICASSERT       0x00004000
#define APICDEASSERT     0x00000000
#define APICLEVEL        0x00008000
#define APICBCAST        0x00080000
#define APICPENDING      0x00001000
#define APICFIXED        0x00000000
#define APICSMI          0x00000200
#define APICEXTINTR      0x00000700
#define APICMASKED       0x00010000
#define APICSELF         0x00040000
#define APICBUTSELF      0x00080000
#define APICALL          0x000c0000

#if !defined(__ASSEMBLER__)

struct lvtreg {
    unsigned vec : 8;
    unsigned delivmode : 3;
    unsigned res1      : 1;
    unsigned delivstat : 1;
    unsigned polarity  : 1;
    unsigned rmtintr   : 1;
    unsigned trig      : 1;
    unsigned mask      : 1;
    unsigned res2      : 15;
    uint32_t res3[3];
};

/* TODO: pad this to fill a page */
struct apic {
    uint32_t       res1;
    uint32_t       res2;
    unsigned       res3   : 24;
    unsigned       physid : 4;
    unsigned       res4   : 4;
    unsigned       res5[3];
    const unsigned version : 8;
    const unsigned res6    : 8;
    const unsigned maxlvt  : 8;
    uint32_t       res7[3];
    uint32_t       res8[4];
    uint32_t       res9[4];
    uint32_t       res10[4];
    uint32_t       res11[4];
    /* task priority register */
    struct {
        unsigned   lvl  : 8;
        unsigned   res12 : 24;
        uint32_t   res13[3];
    } taskprio;
    /* arbitration priority register */
    const struct {
        unsigned   lvl  : 8;
        unsigned   res14 : 24;
        uint32_t   res15[3];
    } arbprio;
    uint32_t       eoi;
    uint32_t       res16[3];
    /* remote register; not used in P4 or Xeon */
    const uint32_t res17[4];
    struct {
        unsigned   res18 : 24;
        unsigned   val   : 8;
        uint32_t   res19[3];
    } logdest;
    /* destination format register */
    struct {
        unsigned   res20 : 28;
        unsigned   model : 4;
        uint32_t   res21[3];
    } destfmt;
    struct {
        unsigned   vec    : 8;
        unsigned   on     : 1;
        unsigned   res22  : 1; /* focus CPU, not supported on Xeon */
        unsigned   res23  : 2;
        unsigned   eoibcs : 1; /* EOI broadcast suppression */
        unsigned   res24  : 19;
        uint32_t   res25[3];
    } spurint;
    /* interrupt status register */
    const struct {
        uint32_t   bitmap;
        uint32_t   res26[3];
    } intrstat;
    /* trigger mode register */
    const struct {
        uint32_t   bitmap;
        uint32_t   res27[3];
    } trigmode;
    /* interrupt request register */
    const struct {
        uint32_t   bitmap;
        uint32_t   res28[3];
    } intrreq;
    /* error status register */
    const struct {
        uint32_t   code;
        uint32_t   res29[3];
    } error;
    uint32_t       res30[24];
    /* LVT CMCI */
    struct {
        uint32_t   val;
        uint32_t   res31[3];
    } lvtcmci;
    struct {
        unsigned   vec : 8;
        unsigned   delivmode : 3;
        unsigned   destmode  : 1;
        unsigned   delivstat : 1;
        unsigned   res32     : 1;
        unsigned   lvl       : 1;
        unsigned   trig      : 1;
        unsigned   res33     : 2;
        unsigned   shorthand : 1;
        unsigned   res34     : 12;
        uint32_t   res35[3];
        unsigned   res36     : 24;
        unsigned   dest      : 8;
        uint32_t   res37[3];
    } intrcmd;
    struct {
        unsigned   vec       : 8;
        unsigned   res38     : 4;
        unsigned   delivstat : 1;
        unsigned   res39     : 3;
        unsigned   mask      : 1;
        unsigned   mode      : 2;
        unsigned   res40     : 13;
        uint32_t   res41[3];
    } lvttmr;
    struct lvtreg  lvttherm;
    struct lvtreg  lvtperf;
    struct lvtreg  lvtlint0;
    struct lvtreg  lvtlint1;
    struct lvtreg  lvterror;
    /* initial count register for timer */
    struct {
        uint32_t   cnt;
        uint32_t   res42[3];
    } tmrinit;
    /* current count register for timer */
    struct {
        uint32_t   cur;
        uint32_t   res42[3];
    } tmrcnt;
    uint32_t       res43[16];
    /* divide configuration register for timer */
    struct {
        unsigned   div   : 4;
        unsigned   res44 : 28;
        uint32_t   res45[3];
    } divconf;
    uint32_t       res46[4];
};

static __inline__ uint32_t
apicread(uint32_t reg)
{
    volatile uint32_t ret = mpapic[reg >> 2];

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
    __asm__ __volatile__ ("pushfl\n");
    __asm__ __volatile__ ("cli\n");
    apicwrite(hi, APICINTRHI);
    kusleep(nusec);
    apicwrite(lo, APICINTRLO);
    kusleep(nusec);
    __asm__ __volatile__ ("sti\n");
    __asm__ __volatile__ ("popfl\n");
}

#endif /* !defined(__ASSEMBLER__) */

#endif /* SMP || APIC */

#endif /* __KERN_UNIT_X86_APIC_H__ */

