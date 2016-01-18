#if !defined(__x86_64__) && !defined(__amd64__)

#include <kern/conf.h>

#if (SMP) || (APIC)

#include <stddef.h>
#include <sys/io.h>
#include <zero/trix.h>
#include <zero/asm.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/types.h>
#include <kern/util.h>
#include <kern/malloc.h>
#include <kern/cpu.h>
#include <kern/mem/vm.h>
#include <kern/unit/x86/boot.h>
#include <kern/unit/x86/asm.h>
#include <kern/unit/x86/apic.h>
#include <kern/unit/x86/link.h>
#include <kern/unit/ia32/seg.h>
#include <kern/unit/ia32/mp.h>

#if (HPET)
extern void hpetinit(void);
#endif
extern void tssinit(long id);

/* used to scan for MP table */
#define EBDAADR   0x040e
#define TOPMEMADR 0x0413
#define TOPMEMDEF 0xa0000
/* MP table signature */
#define MPSIG 0x5f504d5f        // "_MP_"

extern pde_t    *kernpagedir[NPDE];

extern void      gdtinit(void);
extern void      trapinitidt(void);
#if (IOAPIC)
extern void      ioapicinit(long id);
#endif
extern void      cpuinit(volatile struct cpu *cpu);
extern void      seginit(long id);
extern void      idtset(void);

extern volatile struct cpu  cputab[NCPU];
volatile struct cpu        *mpbootcpu;
volatile long               mpmultiproc;
volatile long               mpncpu;
volatile long               mpioapicid;
volatile uint32_t          *mpapic;
volatile uint32_t          *mpioapic;

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
mpprobe(uintptr_t adr, unsigned long len)
{
    struct mp *mp = NULL;
    uint32_t  *ptr = (uint32_t *)adr;
    uint32_t  *lim = (uint32_t *)(adr + len);

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
    uint32_t       adr = 0;
    struct mp     *mp = NULL;

    adr = (uint32_t)(((uint16_t *)EBDAADR)[0] << 4);
    if (adr) {
        mp = mpprobe(adr, 1024);
    }
    if (mp) {

        return mp;
    }
    adr = (uint32_t)((((uint16_t *)TOPMEMADR)[0] << 10) - 1024);
    mp = mpprobe(adr - 1024, 1024);
    if (mp) {

        return mp;
    }
    if (adr != TOPMEMDEF - 1024) {
        adr = TOPMEMDEF - 1024;
        mp = mpprobe(adr, 1024);
        if (mp) {
            
            return mp;
        }
    }
    adr = 0xf0000;
    mp = mpprobe(adr, 0x10000);

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
    long             core;
    uint8_t         *u8ptr;
    uint8_t         *lim;

    conf = mpconf(&mp);
    if (!conf) {

        return;
    }
    mpapic = conf->apicadr;
    for (u8ptr = (uint8_t *)(conf + 1), lim = (uint8_t *)conf + conf->len ;
         u8ptr < lim ; ) {
        switch (*u8ptr) {
            case MPCPU:
                cpu = (struct mpcpu *)u8ptr;
                core = cpu->id;
#if 0
                if (mpncpu != core) {
                    mpmultiproc = 0;
                }
#endif
                if (cpu->flags & MPCPUBOOT) {
                    mpbootcpu = &cputab[core];
#if 0
                    cpuinit((struct cpu *)mpbootcpu);
#endif
                }
                cputab[core].id = core;
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
    if (mpncpu > 1) {
        mpmultiproc = 1;
    } else {

        return;
    }
    if (mp->intmode) {
        outb(0x70, 0x22);               // select IMCR
        outb(inb(0x23) | 0x01, 0x23);   // mask external timer interrupts
    }
#if 0
    /* Boot CPU */
    /* local APIC initialisation where present */
//    apicinit(0);
    /* I/O APIC initialisation */
//    ioapicinit(0);
#endif

    return;
}

ASMLINK NORETURN
void
mpmain(struct cpu *cpu)
{
    seginit(cpu->id);
    idtset();
    m_xchg(&cpu->statflg, CPUSTARTED);
    /* TODO: initialise HPET; enable [rerouted] interrupts */
#if (HPET)
    hpetinit();
#endif
    apicinit(cpu->id);
#if (IOAPIC)
    ioapicinit(cpu->id);
#endif
    tssinit(cpu->id);
#if 0
    while (1) {
        k_waitint();
    }
#endif
    schedloop();

    /* NOTREACHED */
    for ( ; ; ) { ; }
}

void
mpstart(void)
{
    volatile struct cpu *cpu;
    volatile struct cpu *lim;
    uint32_t            *mpentrystk = (uint32_t *)MPENTRYSTK;

    lim = &cputab[0] + mpncpu;
#if 0
    if (first) {
        kmemcpy((void *)MPENTRY,
                mpentry, (uint8_t *)&mpend - (uint8_t *)&mpentry);
        first = 0;
    }
#endif
    for (cpu = &cputab[0] ; cpu < lim ; cpu++) {
        if (cpu == mpbootcpu) {
            /* started already */
            
            continue;
        }
        kprintf("starting CPU %ld @ 0x%lx\n", cpu->id, MPENTRY);
        cpuinit(cpu);
        apicinit(cpu->id);
        ioapicinit(cpu->id);
        *--mpentrystk = (uint32_t)cpu;
        *--mpentrystk = MPENTRYSTK - cpu->id * MPSTKSIZE;
        *--mpentrystk = (uint32_t)&kernpagedir;
        apicstart(cpu->id, MPENTRY);
        while (!cpu->statflg) {
            ;
        }
    }

    return;
}

/* spurious interrupt handler for SMP */
void
mpspurint(void)
{
    apiceoi();

    return;
}

#endif /* SMP || APIC */

#endif /* !__x86_64__ && !__amd64__ */
