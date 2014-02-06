#include <kern/conf.h>

#if (SMP)

#include <stddef.h>
#include <sys/io.h>
#include <zero/trix.h>
#include <zero/asm.h>
#include <zero/param.h>
#include <zero/types.h>
#include <kern/util.h>
#include <kern/mem.h>
#include <kern/unit/x86/asm.h>
#include <kern/unit/x86/cpu.h>
#include <kern/unit/x86/apic.h>
#include <kern/unit/ia32/link.h>
#include <kern/unit/ia32/boot.h>
#include <kern/unit/ia32/seg.h>
#include <kern/unit/ia32/mp.h>
#include <kern/unit/ia32/vm.h>

/* used to scan for MP table */
#define EBDAADR   0x040e
#define TOPMEMADR 0x0413
#define TOPMEMDEF 0xa0000
/* MP table signature */
#define MPSIG 0x5f504d5f        // "_MP_"

extern uint32_t *kernpagedir[NPDE];

extern void      gdtinit(void);
extern void      pginit(void);
extern void      idtinit(void);
extern void      ioapicinit(long id);
extern void      cpuinit(struct m_cpu *cpu);
extern void      seginit(long id);
extern void      idtset(void);

volatile struct m_cpu  mpcputab[NCPU] ALIGNED(PAGESIZE);
volatile struct m_cpu *mpbootcpu;
volatile long          mpmultiproc;
volatile long          mpncpu;
volatile long          mpioapicid;
volatile uint32_t     *mpapic;
volatile uint32_t     *mpioapic;

static long
mpchksum(uint8_t *ptr, unsigned long len)
{
    uint8_t sum = 0;

    while (len--) {
        sum += *ptr++;
    }

    return sum;
}

static struct mp *
mpfind(uintptr_t adr, unsigned long len)
{
    uint32_t  *lim = (uint32_t *)(adr + len);
    struct mp *mp = NULL;
    uint32_t  *ptr = (uint32_t *)adr;

    while (ptr < lim) {
        if (*ptr == MPSIG && !mpchksum((uint8_t *)ptr, sizeof(struct mp))) {
            mp = (struct mp *)ptr;

            return mp;
        }
        ptr++;
    }

    return mp;
}

/*
 * MP structure is in one of the following 3 locations
 * - the first KB of the EBDA
 * - the last KB of system base memory
 * - in the BIOS ROM between 0xf0000 and 0xfffff
 */
struct mp *
mpsearch(void)
{
    uint32_t       adr;
    struct mp     *mp;

    adr = (uint32_t)(((uint16_t *)EBDAADR)[0] << 4);
    mp = mpfind(adr, 1024);
    if (mp) {

        return mp;
    }
    adr = (uint32_t)((((uint16_t *)TOPMEMADR)[0] << 10) - 1024);
    mp = mpfind(adr - 1024, 1024);
    if (mp) {

        return mp;
    }
    if (adr != TOPMEMDEF - 1024) {
        adr = TOPMEMDEF - 1024;
        mp = mpfind(adr, 1024);
        if (mp) {
            
            return mp;
        }
    }
    adr = 0xf0000;
    mp = mpfind(adr, 0x10000);

    return mp;
}

static struct mpconf *
mpconf(struct mp **mptab)
{
    struct mp     *mp = mpsearch();
    struct mpconf *conf;

    if ((mp) && (mp->conftab)) {
        conf = mp->conftab;
        if (!kmemcmp(conf, "PCMP", 4)) {
            if ((conf->ver == 1 || conf->ver == 4)
                && !mpchksum((uint8_t *)conf, conf->len)) {
                *mptab = mp;

                return conf;
            } else {

                return NULL;
            }
        } else {

            return NULL;
        }
    }

    return NULL;
}

void
mpinit(void)
{
    struct mp       *mp;
    struct mpconf   *conf;
    struct mpcpu    *cpu;
    struct mpioapic *ioapic;
    uint8_t         *u8ptr;
    uint8_t         *lim;

    mpbootcpu = &mpcputab[0];
    conf = mpconf(&mp);
    if (!conf) {

        return;
    }
    cpuinit((struct m_cpu *)mpbootcpu);
    mpmultiproc = 1;
    mpapic = conf->apicadr;
    for (u8ptr = (uint8_t *)(conf + 1), lim = (uint8_t *)conf + conf->len ;
         u8ptr < lim ; ) {
        switch (*u8ptr) {
            case MPCPU:
                cpu = (struct mpcpu *)u8ptr;
                if (mpncpu != cpu->id) {
                    mpmultiproc = 0;
                }
                if (cpu->flags & MPCPUBOOT) {
                    mpbootcpu = &mpcputab[mpncpu];
                }
                mpcputab[mpncpu].id = mpncpu;
                mpncpu++;
                u8ptr += sizeof(struct mpcpu);

                continue;
            case MPIOAPIC:
                ioapic = (struct mpioapic *)u8ptr;
                mpioapicid = ioapic->apicnum;
                mpioapic = ioapic->adr;
                u8ptr += sizeof(struct mpioapic);

                continue;
            case MPBUS:
            case MPIOINTR:
            case MPLINTR:
                u8ptr += 8;

                continue;
            default:
                mpmultiproc = 0;

                break;
        }
    }
    if ((mpncpu == 1) || !mpmultiproc) {
        mpncpu = 1;
//        mptab = NULL;
//        mpioapicid = 0;

        return;
    } else {
        if (mp->intmode) {
            outb(0x70, 0x22);               // select IMCR
            outb(inb(0x23) | 0x01, 0x23);   // mask external timer interrupts
        }
    }
    /* local APIC initialisation where present */
    apicinit(0);
    /* I/O APIC initialisation */
    ioapicinit(0);

    return;
}

ASMLINK
void
mpmain(struct m_cpu *cpu)
{
//    gdtinit();
    seginit(cpu->id);
    idtset();
    m_xchgl(&cpu->started, 1L);
    while (1) {
        k_waitint();
    }
}

void
mpstart(void)
{
    volatile static int    first = 1;
    volatile struct m_cpu *cpu;
    volatile struct m_cpu *lim;
    uint32_t              *mpentrystk = (uint32_t *)MPENTRYSTK;

    lim = &mpcputab[0] + mpncpu;
    if (first) {
        kmemcpy((void *)MPENTRY,
                mpentry, (uint8_t *)&mpend - (uint8_t *)&mpentry);
        first = 0;
    }
    for (cpu = &mpcputab[0] ; cpu < lim ; cpu++) {
        if (cpu == mpbootcpu) {
            /* started already */
            
            continue;
        }
        kprintf("starting CPU %ld @ 0x%lx\n", cpu->id, MPENTRY);
        cpuinit((struct m_cpu *)cpu);
        apicinit(cpu->id);
        ioapicinit(cpu->id);
        *--mpentrystk = (uint32_t)cpu;
        *--mpentrystk = MPENTRYSTK - cpu->id * MPSTKSIZE;
        *--mpentrystk = (uint32_t)&kernpagedir;
        apicstart(cpu->id, MPENTRY);
        while (!cpu->started) {
            ;
        }
    }

    return;
}

/* spurious interrupt handler for SMP */
void
mpspurint(void)
{
    apicwrite(0, APICEOI);

    return;
}

#endif /* SMP */

