#ifndef __KERN_UNIT_X86_CPU_H__
#define __KERN_UNIT_X86_CPU_H__

#include <kern/conf.h>
#include <stdint.h>
//#include <zero/cdecl.h>
//#include <sys/types.h>
#if defined(__KERNEL__)
#include <zero/param.h>
#include <zero/mtx.h>
#include <kern/cpu.h>
#include <kern/types.h>
#include <kern/proc/proc.h>
#include <kern/proc/task.h>
#include <kern/unit/x86/bits/cpu.h>
#endif
//#include <zero/types.h>

#if defined(__KERNEL__) && (__KERNEL__)
#define __printf kprintf
#define __strcmp kstrcmp
#include <kern/util.h>
#else
#include <stdio.h>
#include <string.h>
#define __printf printf
#define __strcmp strcmp
#endif

#define M_CPUIDINSTRTLB   0x00
#define M_CPUIDDATATLB    0x01
#define M_CPUIDINSTRCACHE 0x02
#define M_CPUIDDATACACHE  0x03
#define M_CPUIDUNICACHE   0x04
struct m_cacheinfo {
    int32_t type;
    int32_t size;
    int32_t nway;
    int32_t xsize;
};

extern struct m_cacheinfo cpuidcacheinfo[16];

/* cpuid instruction. */

#define cpuid(op, buf)                                                  \
    do {                                                                \
        __asm__ __volatile__ ("movl %4, %%eax\n"                        \
                              "cpuid\n"                                 \
                              "movl %%eax, %0\n"                        \
                              "movl %%ebx, %1\n"                        \
                              "movl %%ecx, %2\n"                        \
                              "movl %%edx, %3\n"                        \
                              : "=m" ((buf)->eax),                      \
                                "=m" ((buf)->ebx),                      \
                                "=m" ((buf)->ecx),                      \
                                "=m" ((buf)->edx)                       \
                              : "i" (op)                                \
                              : "eax", "ebx", "ecx", "edx");            \
    } while (0)
        
struct m_cpuid {
    int32_t eax;
    int32_t ebx;
    int32_t ecx;
    int32_t edx;
};

#define cpugetclsize(buf)  ((buf)->l1i.clsz)
#define cpugetntlb(buf)    ((buf)->l1i.ntlb)
#define cpugetl1isize(buf) ((buf)->l1i.size)
#define cpugetl1inway(buf) ((buf)->l1i.nway)
#define cpugetl1dsize(buf) ((buf)->l1d.size)
#define cpugetl1dnway(buf) ((buf)->l1d.nway)
#define cpugetl2size(buf)  ((buf)->l2.size)
#define cpugetl2nway(buf)  ((buf)->l2.nway)

struct m_cpucache {
    uint32_t size;
    uint32_t clsz;
    uint32_t nway;
    uint32_t ntlb;
};

#define CPUIDAMD       0
#define CPUIDINTEL     1
#define CPUIDVIA       2
#define CPUIDTRANSMETA 3
#define CPUIDCYRIX     4
#define CPUIDCENTAUR   5
#define CPUIDNEXGEN    6
#define CPUIDUMC       7
#define CPUIDSIS       8
#define CPUIDNSC       9
#define CPUIDRISE      10

#define cpuidaddci(buf, id, t, s, n, x)                                 \
    do {                                                                \
        (buf)->type = (t);                                              \
        (buf)->size = (s);                                              \
        (buf)->nway = (n);                                              \
        (buf)->xsize = (x);                                             \
    } while (0)

static void
cpuidinitci_intel(struct m_cacheinfo *buf)
{
    if (!buf) {
        buf = cpuidcacheinfo;
    }
    cpuidaddci(buf, 0x01, M_CPUIDINSTRTLB, 4096, 4, 32);
    cpuidaddci(buf, 0x02, M_CPUIDINSTRTLB, 4096 * 1024, 4, 4);
    cpuidaddci(buf, 0x03, M_CPUIDDATATLB, 4096, 4, 64);
    cpuidaddci(buf, 0x04, M_CPUIDDATATLB, 4096 * 1024, 4, 8);
    cpuidaddci(buf, 0x06, M_CPUIDINSTRCACHE, 8192, 4, 32);
    cpuidaddci(buf, 0x08, M_CPUIDINSTRCACHE, 16384, 4, 32);
    cpuidaddci(buf, 0x0a, M_CPUIDDATACACHE, 8192, 2, 32);
    cpuidaddci(buf, 0x0c, M_CPUIDDATACACHE, 16384, 2, 32);
    cpuidaddci(buf, 0x41, M_CPUIDUNICACHE, 131072, 4, 32);
    cpuidaddci(buf, 0x42, M_CPUIDUNICACHE, 262144, 4, 32);
    cpuidaddci(buf, 0x43, M_CPUIDUNICACHE, 524288, 4, 32);
    cpuidaddci(buf, 0x44, M_CPUIDUNICACHE, 1048576, 4, 32);
}

static void
cpuid_print_cache_info_intel(uint8_t id)
{
    struct m_cacheinfo *info;

    info = &cpuidcacheinfo[id];
    if (info->size) {
        switch (info->type) {
            case M_CPUIDINSTRTLB:
                __printf("itlb: %ldK pages, %ld-way, %ld entries\n",
                       (long)info->size / 1024, (long)info->nway, (long)info->xsize);
                
                break;
            case M_CPUIDDATATLB:
                __printf("dtlb: %ldK pages, %ld-way, %ld entries\n",
                       (long)info->size / 1024, (long)info->nway, (long)info->xsize);
                
                break;
            case M_CPUIDINSTRCACHE:
                __printf("icache: %ldK, %ld-way, %ld-byte line\n",
                       (long)info->size / 1024, (long)info->nway, (long)info->xsize);
                
                break;
            case M_CPUIDDATACACHE:
                __printf("dcache: %ldK, %ld-way, %ld-byte line\n",
                       (long)info->size / 1024, (long)info->nway, (long)info->xsize);
                
                break;
            case M_CPUIDUNICACHE:
                __printf("ucache: %ldK, %ld-way, %ld-byte line\n",
                       (long)info->size / 1024, (long)info->nway, (long)info->xsize);
                
                break;
            default:
                __printf("failed to probe cache info\n");

                break;
        }
    }

    return;
}

static void
cpuid_print_l1_info_amd(struct m_cpuid *cpuid)
{
    __printf("dtlb: 4K pages, %ld-way, %ld entries\n",
           (long)cpuid->ebx >> 24, ((long)cpuid->ecx >> 16) & 0xff);
    __printf("dtlb: 2M pages, %ld-way, %ld entries\n",
           (long)cpuid->eax >> 24, (long)cpuid->eax & 0xff);
    __printf("dtlb: 4M pages, %ld-way, %ld entries\n",
           (long)cpuid->eax >> 24, ((long)cpuid->eax & 0xff) >> 1);

    __printf("itlb: 4K pages, %ld-way, %ld entries\n",
           ((long)cpuid->ebx >> 8) & 0xff, (long)cpuid->edx & 0xff);
    __printf("itlb: 2M pages, %ld-way, %ld entries\n",
           ((long)cpuid->eax >> 8) & 0xff, (long)cpuid->eax & 0xff);
    __printf("itlb: 4M pages, %ld-way, %ld entries\n",
           ((long)cpuid->eax >> 8) & 0xff, ((long)cpuid->eax & 0xff) >> 1);

    __printf("dcache: %ldK, %ld-way, %ld-byte line\n",
           (long)cpuid->ecx >> 24, ((long)cpuid->ecx >> 16) & 0xff, (long)cpuid->ecx & 0xff);
    __printf("icache: %ldK, %ld-way, %ld-byte line\n",
           (long)cpuid->edx >> 24, ((long)cpuid->edx >> 16) & 0xff, (long)cpuid->edx & 0xff);

    return;
}

static void
cpuid_print_l2_info_amd(struct m_cpuid *cpuid)
{
    __printf("l2: %ldK, %ld-way, %ld-byte line\n",
           (long)cpuid->ecx >> 16, ((long)cpuid->ecx >> 12) & 0x0f, (long)cpuid->ecx & 0xff);

    __printf("l2dtlb: 4K, %ld-way,  %ld entries\n",
           (long)cpuid->ebx >> 28, ((long)cpuid->ebx >> 16) & 0x0fff);
    __printf("l2dtlb: 2M, %ld-way,  %ld entries\n",
           (long)cpuid->eax >> 28, ((long)cpuid->eax >> 16) & 0x0fff);
    __printf("l2dtlb: 4M, %ld-way,  %ld entries\n",
           (long)cpuid->eax >> 28, (((long)cpuid->eax >> 16) & 0x0fff) >> 1);

    __printf("l2itlb: 4K, %ld-way,  %ld entries\n",
           ((long)cpuid->ebx >> 12) & 0x0f, (long)cpuid->ebx & 0x0fff);
    __printf("l2itlb: 2M, %ld-way,  %ld entries\n",
           ((long)cpuid->eax >> 12) & 0x0f, (long)cpuid->eax & 0x0fff);
    __printf("l2itlb: 4M, %ld-way,  %ld entries\n",
           ((long)cpuid->eax >> 12) & 0x0f, ((long)cpuid->eax & 0x0fff) >> 1);

    return;
}

#define M_CPUIDVENDOR  0x00000000
#define M_CPUIDINFO    0x00000001
#define M_CPUIDCACHE   0x00000002
#define M_CPUIDSERIAL  0x00000003
#define M_CPUIDCORE    0x00000004
#define M_CPUIDSSE3    0x00000005
#define M_CPUIDPM      0x00000006
#define M_CPUIDHI      0x80000000
#define M_CPUIDEXTINFO 0x80000001
#define M_CPUIDL1_AMD  0x80000005
#define M_CPUIDL2_AMD  0x80000006

union m_cpuidvendor {
    int32_t       wtab[4];
    unsigned char str[16];
};

#define cpuidgetvendor(ptr)                                             \
    do {                                                                \
        __asm__("movl %0, %%eax" : : "i" (M_CPUIDVENDOR));              \
        __asm__("cpuid" : : : "eax", "ebx", "ecx", "edx");              \
        __asm__("movl %%ebx, %0" : "=m" ((ptr)->wtab[0]));              \
        __asm__("movl %%edx, %0" : "=m" ((ptr)->wtab[1]));              \
        __asm__("movl %%ecx, %0" : "=m" ((ptr)->wtab[2]));              \
        __asm__("xorl %eax, %eax");                                     \
        __asm__("movl %%eax, %0" : "=m" ((ptr)->wtab[3]));              \
    } while (0)

/* control registers. need to run in system mode (ring 0). */

#define cpugetmodes(ptr)                                                \
    __asm__("movl %cr4, %eax");                                         \
    __asm__("movl %%eax, %0": "=m" ((ptr)->cr4));

#define cpuhastsc(ptr)                                                  \
    (!((ptr)->cr4 & CPU_CR4_TSD))
#define cpuhasde(ptr)                                                   \
    ((ptr)->cr4 & CPU_CR4_DE)
#define cpuhaspse(ptr)                                                  \
    ((ptr)->cr4 & CPU_CR4_PSE)
#define cpuhaspge(ptr)                                                  \
    ((ptr)->cr4 & CPU_CR4_PGE)
#define cpuhaspce(ptr)                                                  \
    ((ptr)->cr4 & CPU_CR4_PCE)
struct m_cpuidcregs {
    int32_t cr4;
};

struct m_cpucacheinfo {
    struct m_cpucache l1i;
    struct m_cpucache l1d;
    struct m_cpucache l2;
};

struct m_cpuinfo {
    long                  flg;
    struct m_cpuid        id;
    struct m_cpuidcregs   cregs;
    struct m_cpucacheinfo cache;
};

void cpuprobe(volatile struct m_cpuinfo *cpuinfo,
              volatile struct m_cpucacheinfo *cache);

#if defined(__KERNEL__)

#if (PTRBITS == 32)
extern volatile struct cpu  * k_curcpu  __asm__ ("%gs:0");
extern volatile long          k_curunit __asm__ ("%gs:4");
extern volatile struct task * k_curtask __asm__ ("%gs:8");
extern volatile long          k_curpid  __asm__ ("%gs:12");
#elif (PTRBITS == 64)
extern volatile struct cpu  * k_curcpu  __asm__ ("%gs:0");
extern volatile long          k_curunit __asm__ ("%gs:8");
extern volatile struct task * k_curtask __asm__ ("%gs:16");
extern volatile long          k_curpid  __asm__ ("%gs:24");
#endif

#endif /* defined(__KERNEL__) */

//#define NCPUWORD     6 /* cpu, proc, task, pdir, pid, info */
/* CPU flg-values */
#define CPUSTARTED (1L << 0)
#define CPURESCHED (1L << 1)
#define CPUINITBIT (1L << 2)
#define CPUHASINFO (1L << 3)
struct cpu {
    /* cpu-local variables */
    struct cpu       *cpu;
    long              unit;
    struct task      *task;
    long              pid;
    struct proc      *proc;
    pde_t             pdir;
    long              flg;
    /* scheduler parameters */
    unsigned long     ntick;    // tick count
    long              nicemin;  // minimum nice in effect
    long              loaduser; // user/timeshare load
    long              load;     // load
    /* info about cpu cache and features */
    struct m_cpuinfo  info;
};

#endif /* __KERN_UNIT_X86_CPU_H__ */

