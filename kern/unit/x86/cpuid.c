/*
 * Thanks to
 * - Martin 'bluet' Stensgård
 * - Tim 'amon' Zebulla
 * for testing this code. :)
 * -vendu
 */

#if defined(__KERNEL__) && (__KERNEL__)
#define __printf kprintf
#define __strcmp kstrcmp
#else
#include <stdio.h>
#include <string.h>
#define __printf printf
#define __strcmp strcmp
#endif

/* TODO: fix this stuff to run on SMP (per-CPU m_cpuinfo structures) */

/*
 * README: http://softpixel.com/~cwright/programming/simd/cpuid.php
 */
#if !defined(TEST)
#define TEST 0
#endif

#if (TEST)
#include <stdio.h>
#include <stdlib.h>
#endif

#include <stdint.h>

#include <zero/param.h>
#include <zero/cdecl.h>
#include <zero/types.h>

#include <kern/util.h>
#include <kern/unit/x86/cpu.h>

#if defined(__ZEROKERNEL__) /* kernel-level stuff only */
//#include <mach/unit/ia32/cpu.h>
#endif

/* cpuid instruction. */

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

#define cpuid(op, ptr)                                                  \
    __asm__ __volatile__ ("movl %4, %%eax\n"                            \
                          "cpuid\n"                                     \
                          "movl %%eax, %0\n"                            \
                          "movl %%ebx, %1\n"                            \
                          "movl %%ecx, %2\n"                            \
                          "movl %%edx, %3\n"                            \
                          : "=m" ((ptr)->eax),                          \
                            "=m" ((ptr)->ebx),                          \
                            "=m" ((ptr)->ecx),                          \
                            "=m" ((ptr)->edx)                           \
                          : "i" (op)                                    \
                          : "eax", "ebx", "ecx", "edx")
        
struct m_cpuid {
    int32_t eax;
    int32_t ebx;
    int32_t ecx;
    int32_t edx;
};

union m_cpuidvendor {
    int32_t       wtab[4];
    unsigned char str[16];
};

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

/* vendor strings. */
/* vendor strings. */
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

static const char *_vendortab[]
= {
    "AuthenticAMD",
    "GenuineIntel",
    "CentaurHauls",
    "GenuineTMx86",
    "CyrixInstead",
    "CentaurHauls",
    "NexGenDriven",
    "UMC UMC UMC ", /* FIXME - Is the space right? */
    "SiS SiS SiS ", /* FIXME - Same. */
    "Geode by NSC",
    "RiseRiseRise"
};

static struct m_cacheinfo cpuidcacheinfo[256] ALIGNED(PAGESIZE);
struct m_cpuinfo          cpuinfo;

#define cpuidaddci(id, t, s, n, x)                                      \
    do {                                                                \
        cpuidcacheinfo[id].type = (t);                                  \
        cpuidcacheinfo[id].size = (s);                                  \
        cpuidcacheinfo[id].nway = (n);                                  \
        cpuidcacheinfo[id].xsize = (x);                                 \
    } while (0)

static void
cpuidinitci_intel(void)
{
    cpuidaddci(0x01, M_CPUIDINSTRTLB, 4096, 4, 32);
    cpuidaddci(0x02, M_CPUIDINSTRTLB, 4096 * 1024, 4, 4);
    cpuidaddci(0x03, M_CPUIDDATATLB, 4096, 4, 64);
    cpuidaddci(0x04, M_CPUIDDATATLB, 4096 * 1024, 4, 8);
    cpuidaddci(0x06, M_CPUIDINSTRCACHE, 8192, 4, 32);
    cpuidaddci(0x08, M_CPUIDINSTRCACHE, 16384, 4, 32);
    cpuidaddci(0x0a, M_CPUIDDATACACHE, 8192, 2, 32);
    cpuidaddci(0x0c, M_CPUIDDATACACHE, 16384, 2, 32);
    cpuidaddci(0x41, M_CPUIDUNICACHE, 131072, 4, 32);
    cpuidaddci(0x42, M_CPUIDUNICACHE, 262144, 4, 32);
    cpuidaddci(0x43, M_CPUIDUNICACHE, 524288, 4, 32);
    cpuidaddci(0x44, M_CPUIDUNICACHE, 1048576, 4, 32);
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

#define cpuidgetvendor(ptr)                                             \
    __asm__("movl %0, %%eax" : : "i" (M_CPUIDVENDOR));                  \
    __asm__("cpuid" : : : "eax", "ebx", "ecx", "edx");                  \
    __asm__("movl %%ebx, %0" : "=m" ((ptr)->wtab[0]));                  \
    __asm__("movl %%edx, %0" : "=m" ((ptr)->wtab[1]));                  \
    __asm__("movl %%ecx, %0" : "=m" ((ptr)->wtab[2]));                  \
    __asm__("xorl %eax, %eax");                                         \
    __asm__("movl %%eax, %0" : "=m" ((ptr)->wtab[3]));
#define cpuidgetinfo(ptr)                                               \
    cpuid(M_CPUIDINFO, ptr)
#define cpuidgetexti(ptr)                                               \
    cpuid(M_CPUIDEXTINFO, ptr)
#define cpuidgetci_intel(ptr)                                           \
    cpuid(M_CPUIDCACHE, ptr)
#define cpuidgetl1_amd(ptr)                                             \
    cpuid(M_CPUIDL1_AMD, ptr)
#define cpuidgetl2_amd(ptr)                                             \
    cpuid(M_CPUIDL2_AMD, ptr)

/* M_CPUIDINFO */
#define cpuidstepping(ptr)                                              \
    ((ptr)->eax & 0x0000000f)
#define cpuidmodel(ptr)                                                 \
    (((ptr)->eax & 0x000000f0) >> 4)
#define cpuidfamily(ptr)                                                \
    (((ptr)->eax & 0x00000f00) >> 8)
#define cpuidtype(ptr)                                                  \
    (((ptr)->eax & 0x00003000) >> 12)
#define cpuidextmodel(ptr)                                              \
    (((ptr)->eax & 0x000f0000) >> 16)
#define cpuidextfamily(ptr)                                             \
    (((ptr)->eax & 0x0ff00000) >> 20)
#define cpuidhaspse(ptr)                                                \
    ((ptr)->edx & CPUIDPSE)
#define cpuidhastsc(ptr)                                                \
    ((ptr)->edx & CPUIDTSC)
#define cpuidhassep(ptr)                                                \
    ((ptr)->edx & CPUIDSEP)
#define cpuidhaspge(ptr)                                                \
    ((ptr)->edx & CPUIDPGE)
#define cpuidhasmmx(ptr)                                                \
    ((ptr)->edx & CPUIDMMX)
#define cpuidhasclfl(ptr)                                               \
    ((ptr)->edx & CPUIDCLFL)
#define cpuidhasfxsr(ptr)                                               \
    ((ptr)->edx & CPUIDFXSR)
#define cpuidhassse(ptr)                                                \
    ((ptr)->edx & CPUIDSSE)
#define cpuidhassse2(ptr)                                               \
    ((ptr)->edx & CPUIDSSE2)
#define cpuidhassse3(ptr)                                               \
    ((ptr)->ecx & CPUIDSSE3)
#define cpuidhasapic(ptr)                                               \
    ((ptr)->edx & CPUIDAPIC)

/* CPUIDEXTINFO */
#define cpuidhasamd_mmx(ptr)                                            \
    ((ptr)->edx & CPUIDAMD_MMX)
#define cpuidhas3dnow(ptr)                                              \
    ((ptr)->edx & CPUID3DNOW)
#define cpuidhas3dnow2(ptr)                                             \
    ((ptr)->edx & CPUID3DNOW2)

/* %edx flags. */

/* CPUIDINFO */
#define CPUIDFPU       0x00000001 /* fpu present. */
#define CPUIDVME       0x00000002 /* virtual mode extensions. */
#define CPUIDDE        0x00000004 /* debugging extensions. */
#define CPUIDPSE       0x00000008 /* page size extensions. */
#define CPUIDTSC       0x00000010 /* time stamp counter. */
#define CPUIDMSR       0x00000020 /* model-specific registers. */
#define CPUIDPAE       0x00000040 /* physical address extensions. */
#define CPUIDMCE       0x00000080 /* machine check exceptions. */
#define CPUIDCXCHG8    0x00000100 /* compare and exchange 8-byte. */
#define CPUIDAPIC      0x00000200 /* on-chip apic. */
#define CPUIDSEP       0x00000800 /* sysexit and sysenter. */
#define CPUIDMTRR      0x00001000 /* memory type range registers. */
#define CPUIDPGE       0x00002000 /* page global enable. */
#define CPUIDMCA       0x00004000 /* machine check architecture. */
#define CPUIDCMOV      0x00008000 /* conditional move-instruction. */
#define CPUIDPAT       0x00010000 /* page attribute table. */
#define CPUIDPSE36     0x00020000 /* 36-bit page size extensions. */
#define CPUIDPSN       0x00040000 /* processor serial number. */
#define CPUIDCLFL      0x00080000 /* CLFLUSH - fixme? */
#define CPUIDDTES      0x00200000 /* debug trace and emon store msr's. */
#define CPUIDACPI      0x00400000 /* thermal control msr. */
#define CPUIDMMX       0x00800000 /* mmx instruction set. */
#define CPUIDFXSR      0x01000000 /* fast fpu save/restore. */
#define CPUIDSSE       0x02000000 /* sse. */
#define CPUIDSSE2      0x04000000 /* sse2. */
#define CPUIDSS        0x08000000 /* selfsnoop. */
#define CPUIDHTT       0x10000000 /* hyper-threading technology. */
#define CPUIDTM1       0x20000000 /* thermal interrupts, status msr's. */
#define CPUIDIA64      0x40000000 /* ia-64 (64-bit m_cpu). */
#define CPUIDPBE       0x80000000 /* pending break event. */

/* CPUIDEXTINFO */
#define CPUIDAMD_MMX   0x00400000
#define CPUID3DNOW2    0x40000000
#define CPUID3DNOW     0x80000000

/* %ecx flags. */
#define CPUIDSSE3      0x00000001 /* sse3. */

#if 0

/* control registers. need to run in system mode (ring 0). */

#define cpuidgetmodes(ptr)                                              \
    __asm__("movl %cr3, %eax");                                         \
    __asm__("movl %%eax, %0": "=m" ((ptr)->cr3));                       \
    __asm__("movl %cr4, %eax");                                         \
    __asm__("movl %%eax, %0": "=m" ((ptr)->cr4));

#define cpuidhaspwt(ptr)                                                \
    ((ptr)->cr3 & CR3_PWT)
#define cpuidhaspcd(ptr)                                                \
    ((ptr)->cr3 & CR3_PCD)
#define cpuidhasrdtsc(ptr)                                              \
    (!((ptr)->cr4 & CR4_TSD))
#define cpuidhaspse(ptr)                                                \
    ((ptr)->cr4 & CR4_PSE)
#define cpuidhaspge(ptr)                                                \
    ((ptr)->cr4 & CR4_PGE)
#define cpuidhasrdpmc(ptr)                                              \
    ((ptr)->cr4 & CR4_PCE)
struct m_cpuidcregs {
    int32_t cr3;
    int32_t cr4;
};

#endif /* 0 */

void
cpuprobe(struct m_cpuinfo *info)
{
    struct m_cpuid       buf;
    union  m_cpuidvendor vbuf;
#if defined(__ZEROKERNEL__)
    struct m_cpuidcregs  rbuf;
#endif
    struct m_cacheinfo  *cbuf;

    cpuidgetvendor(&vbuf);
    if (!__strcmp((const char *)vbuf.str, _vendortab[CPUIDINTEL])) {
        cpuidinitci_intel();
        cpuidgetci_intel(&buf);
        cbuf = &cpuidcacheinfo[M_CPUIDINSTRCACHE];
        info->l1i.size = cbuf->size;
        info->l1i.clsz = cbuf->xsize;
        info->l1i.nway = cbuf->nway;
        cbuf = &cpuidcacheinfo[M_CPUIDINSTRTLB];
        info->l1i.ntlb = cbuf->xsize;
        cbuf = &cpuidcacheinfo[M_CPUIDDATACACHE];
        info->l1d.size = cbuf->size;
        info->l1d.clsz = cbuf->xsize;
        info->l1d.nway = cbuf->nway;
        cbuf = &cpuidcacheinfo[M_CPUIDDATATLB];
        info->l1i.ntlb = cbuf->xsize;
        cbuf = &cpuidcacheinfo[M_CPUIDUNICACHE];
        info->l2.size = cbuf->size;
        info->l2.clsz = cbuf->xsize;
        info->l2.nway = cbuf->nway;
    } else if (!__strcmp((const char *)vbuf.str, _vendortab[CPUIDAMD])) {
        cpuidgetl1_amd(&buf);
        info->l1i.size = buf.edx >> 14;
        info->l1i.clsz = buf.edx & 0xff;
        info->l1i.nway = (buf.edx >> 16) & 0xff ;
        info->l1i.ntlb = buf.ebx & 0xff;
        info->l1d.size = buf.ecx >> 14;
        info->l1d.clsz = buf.ecx & 0xff;
        info->l1d.nway = (buf.ecx >> 16) & 0xff ;
        info->l1d.ntlb = (buf.ebx >> 16) & 0xff;
        cpuidgetl2_amd(&buf);
        info->l2.size = buf.ecx >> 6 & 0xffc00;
        info->l2.clsz = buf.ecx & 0xff;
        info->l2.nway = (buf.ebx >> 12) & 0x0f;
        info->l2.ntlb = buf.ebx & 0x0fff;
    }
    cpuidgetinfo(&buf);
    if (cpuidhasfxsr(&buf)) {
        info->flags |= CPUHASFXSR;
    }
    if (cpuidhasapic(&buf)) {
        info->flags |= CPUHASAPIC;
    }

    return;
}

void
cpuprintinfo(void)
{
    struct m_cpuid       buf;
    union  m_cpuidvendor vbuf;
#if defined(__ZEROKERNEL__)
    struct m_cpuidcregs  mbuf;
#endif
    
    cpuidgetvendor(&vbuf);
    __printf("CPU: vendor: %s\n", vbuf.str);
    if (!__strcmp((const char *)vbuf.str, _vendortab[CPUIDINTEL])) {
        cpuidinitci_intel();
        cpuidgetci_intel(&buf);
        cpuid_print_cache_info_intel(buf.eax);
        cpuid_print_cache_info_intel(buf.ebx);
        cpuid_print_cache_info_intel(buf.ecx);
        cpuid_print_cache_info_intel(buf.edx);
    } else if (!__strcmp((const char *)vbuf.str, _vendortab[CPUIDAMD])) {
        cpuidgetl1_amd(&buf);
        cpuid_print_l1_info_amd(&buf);
        cpuidgetl2_amd(&buf);
        cpuid_print_l2_info_amd(&buf);
    }
    /* stepping, model, family, type, ext_model, ext_family */
    cpuidgetinfo(&buf);
#if 0
    __printf("cpu info:\n");
    __printf("\tstepping: %u\n", cpuidstepping(&buf));
    __printf("\tmodel: %u\n", cpuidmodel(&buf));
    __printf("\tfamily: %u\n", cpuidfamily(&buf));
    __printf("\ttype: %u\n", cpuidtype(&buf));
    __printf("\text_model: %u\n", cpuidextmodel(&buf));
    __printf("\text_family: %u\n", cpuidextfamily(&buf));
#endif
    
    __printf("CPU: features:");
    if (cpuidhaspse(&buf)) {
        __printf(" pse");
    }
    if (cpuidhastsc(&buf)) {
        __printf(" tsc");
    }
    if (cpuidhassep(&buf)) {
        __printf(" sep");
    }
    if (cpuidhaspge(&buf)) {
        __printf(" pge");
    }
    if (cpuidhasmmx(&buf)) {
        __printf(" mmx");
    }
    if (cpuidhasclfl(&buf)) {
        __printf(" clfl");
    }
    if (cpuidhasfxsr(&buf)) {
        __printf(" fxsr");
    }
    if (cpuidhassse(&buf)) {
        __printf(" sse");
    }
    if (cpuidhassse2(&buf)) {
        __printf(" sse2");
    }
    if (cpuidhassse3(&buf)) {
        __printf(" sse3");
    }
//    __printf("\n");
    
    cpuidgetexti(&buf);
//    __printf("amd features: ");
    if (cpuidhasamd_mmx(&buf)) {
        __printf(" mmx");
    }
    if (cpuidhas3dnow(&buf)) {
        __printf(" 3dnow");
    }
    if (cpuidhas3dnow2(&buf)) {
        __printf(" 3dnow2");
    }
    __printf("\n");
    
#if defined(__ZEROKERNEL__)
    cpuidgetmodes(&mbuf);
    __printf("cpu modes:");
    if (cpuidhaspwt(&mbuf)) {
        __printf(" pwt");
    }
    if (cpuidhaspcd(&mbuf)) {
        __printf(" pcd");
    }
    if (cpuidhasrdtsc(&mbuf)) {
        __printf(" rdtsc");
    }
    if (cpuidhaspse(&mbuf)) {
        __printf(" pse");
    }
    if (cpuidhaspge(&mbuf)) {
        __printf(" pge");
    }
    if (cpuidhasrdpmc(&mbuf)) {
        __printf(" rdpmc");
    }
    __printf("\n");
#endif /* 0 */
 
    return;
}

#if (TEST) && 0
int
main(int argc,
     char *argv[])
{
    cpuprobe(&cpuinfo);
    cpuprintinfo();
    
    exit(0);
}
#endif /* TEST */

