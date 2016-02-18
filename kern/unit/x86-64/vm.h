#ifndef __UNIT_X86_64_VM_H__
#define __UNIT_X86_64_VM_H__

#include <kern/types.h>

/*
 * Virtual addresses of things.  Derived from the page directory and
 * page table indexes from pmap.h for precision.
 *
 * 0x0000000000000000 - 0x00007fffffffffff   user map
 * 0x0000800000000000 - 0xffff7fffffffffff   does not exist (hole)
 * 0xffff800000000000 - 0xffff804020100fff   recursive page table (512GB slot)
 * 0xffff804020101000 - 0xfffff7ffffffffff   unused
 * 0xfffff80000000000 - 0xfffffbffffffffff   4TB direct map
 * 0xfffffc0000000000 - 0xfffffdffffffffff   unused
 * 0xfffffe0000000000 - 0xffffffffffffffff   2TB kernel map
 *
 * Within the kernel map:
 *
 * 0xffffffff80000000                        KERNBASE
 */

#if defined(__x86_64__) || defined(__amd64__)

#include <stdint.h>
#include <zero/param.h>
#include <zero/mtx.h>
#include <kern/unit/ia32/vm.h>

extern uint64_t kernpagemapl4tab[512];

#define NADRBIT ADRBITS

static INLINE uint64_t
vmsignadr(void *adr)
{
    int64_t  val = (int64_t)adr;
    int64_t  sign = INT64_C(1) << (NADRBIT - 1);
    int64_t  lobits = (INT64_C(1) << NADRBIT) - 1;
    int64_t  res;

    val &= lobits;
    res = (val ^ sign) - sign;

    return res;
}

#define vmpt4num(adr)  (((uint64_t)(adr) >> 39) & 0x1ff)
#define vmpt3num(adr)  (((uint64_t)(adr) >> 30) & 0x1ff)
#define vmpt2num(adr)  (((uint64_t)(adr) >> 21) & 0x1ff)
#define vmpt1num(adr)  (((uint64_t)(adr) >> 12) & 0x1ff) 
#define vmpagenum(adr) (((uint64_t)(adr) >> PTSHIFT) & UINT64_C(0xfffffffff))
#define vmpageofs(adr) ((uint64_t)(adr) & (PAGESIZE - 1))

#if !defined(__ASSEMBLER__)
#define PAGENOEXEC  UINT64_C(0x8000000000000000U)
#endif

#define EXTPAGEREAD       0x00000001
#define EXTPAGEWRITE      0x00000002
#define EXTPAGEEXEC       0x00000004
#define EXTPAGENOPAT      0x00000040
#define EXTPAGESUPER      0x00000080
#define EXTPAGEREF        0x00000100
#define EXTPAGEDIRTY      0x00000200
#define EXTPAGEMEMTYPE(x) ((x) << 3)

#define NPAGETAB          512 // # of page-structure table entries
#define PTSHIFT           12
#if !defined(__ASSEMBLER__)
#define VMPGMASK          UINT64_C(0xfffffffffffff000)
#if (ADRBITS == 48)
#define VMPHYSMASK        UINT64_C(0x0000ffffffffffff)
#elif (ADRBITS == 56)
#define VMPHYSMASK        UINT64_C(0x00ffffffffffffff)
#elif (ADRBITS == 64)
#define VMPHYSMASK        UINT64_C(0xffffffffffffffff)
#endif
#endif

#define PAGEMANAGED       PAGESYS2
#undef VMPAGEMASK
#define VMPAGEMASK        UINT64_C(0x000ffffffffff000)
#define VMSUPERPAGEMASK   UINT64_C(0x000ffffffff00000)

#define PAGEPDECACHE      (PAGESUPERPDE | PAGEWRITETHRU | PAGENOCACHE)
#define PAGEPTECACHE      (PAGESUPERPTE | PAGEWRITETHRU | PAGENOCACHE)

/* identical settings for 2MB (PDE) and 4KB (PTE) page mappings */
#define PAGEPTEPROMOTE                                                  \
    (PAGENOEXEC | PAGEMANAGED | PAGEWIRED | PAGEGLOBAL | PAGEPTECACHE   \
     | PAGEDIRTY | PAGEREF | PAGEUSER | PAGEWRITE | PAGEPRES)

#define PAGEFLTPROTBAD    0x01  // violation vs. not present
#define PAGEFLTWRITE      0x02  // write fault
#define PAGEFLTUSER       0x04  // access from user-mode
#define PAGEFLTPTEBAD     0x08  // reserved PTE-field non-zero
#define PAGEFALTINST      0x10  // instruction fetch fault

#if defined(AMD64_NPT_AWARE)
//#undef PAGESYS1
#undef PAGEGLOBAL
#undef PAGEREF
#undef PAGEDIRTY
#undef PAGESUPERPTE
#undef PAGEPTECACHE
#undef PAGESUPERPDE
#undef PAGESPDECACHE
#undef PAGEWRITE
#undef PAGEPRES
#endif /* defined(AMD64_NPT_AWARE) */

#define vmpageid(adr) ((uintptr_t)(adr) >> PAGESIZELOG2)

/* level #1 */
#define NPTEPG        (PAGESIZE / sizeof(pte_t))
#define NPTEPGSHIFT   9
#define PAGESHIFT     12
#define PAGEMASK      (PAGESIZE - 1)
/* levvel #2 */
#define NPDEPG        (PAGESIZE / sizeof(pde_t))
#define NPDEPGSHIF    9
#define PDRSHIFT      21
#define NBPDR         (1 << PDRSHIFT)
#define PDRMASK       (NBPDR - 1)
/* level #3 */
#define NPDPEPG       (PAGESIZE / sizeof(pdpe_t))
#define NPDPEPGSHIFT  9
#define PDPSHIFT      30
#define NBPDP         (1 << PDPSHIFT)
#define PDPMASK       (NBPDP - 1)
/* level #4 */
#define NPML4EPG      (PAGESIZE / sizeof(pml4e_t))
#define NPML4EPGSHIFT 9
#define PML4SHIFT     39
#define NBPML4        (1UL << PML4SHIFT)
#define PML4MASK      (NBPML4 - 1)

#define kernvirtadr(l4, l3, l2, l1)                                     \
    (((uint64_t)-1 << 47)                                               \
     | ((uint64_t)(l4) << PML4SHIFT)                                    \
     | ((uint64_t)(l3) << PDPSHIFT)                                     \
     | ((uint64_t)(l2) << PDRSHIFT)                                     \
     | ((uint64_t)(l1) << PAGESHIFT))
    
#define usrvirtadr(l4, l3, l2, l1)                                      \
    (((uint64_t)(l4) << PML4SHIFT)                                      \
     | ((uint64_t)(l3) << PDPSHIFT)                                     \
     | ((uint64_t)(l2) << PDRSHIFT)                                     \
     | ((uint64_t)(l1) << PAGESHIFT))

#define NKERNPML4E   4                    // # of kernel PML4 slots
#define NUMPLM4E     (NPML4EPG / 2)       // # of userland PML4 pages
#define NUPDPE       (NUPML4E * NPDPEPG)  // # of userland PDP pages
#define NUPDE        (NUPDPE * NPDEPG)    // # of userlaND PD entries

#define NDPML4E      8                    // # of PML4 entries for direct map

#define PML4PML4I    (NPML4EPG / 2)       // index of recursive PML4 mapping

#define KPML4BASE    (NPML4EPG - NKPML4E) // KVM at highest addresses
#define DMPML4I      rounddown(KPML4BASE - NDMPML4E, NDMPML4E) // below KVM

#define KPML4I       (NPML4EPG - 1)
#define KPDPI        (NPDPEPG - 2)        // kernel base at -2GB

#if defined(__KERNEL__)

#define PTMAPADR     kernvirtadr(PML4PML4I, 0, 0, 0)
#define PDMAPADR     kernvirtadr(PML4PML4I, PML4PML4I, 0, 0)
#define PDPMAPADR    kernvirtadr(PML4PML4I, PML4PML4I, PML4PML4I, 0)
#define PML4MAPADR   kernvirtadr(PML4PML4I, PML4PML4I, PML4PML4I, PML4PML4I)
#define PML4PML4EADR (PML4MAPADR + (PML4PML4I * sizeof(pml4e_t)))

#define PTMAP        ((pte_t *)PTMAPADR)
#define PDMAP        ((pde_t *)PDMAPADR)
#define PDPMAP       ((pde_t *)PDPMAPADR)
#define PML4MAP      ((pde_t *)PML4MAPADR)
#define PML4PML4E    ((pde_t *)PML4PML4EADR)

#define vmadrtophys(virt) pmap_kextract(((vm_offset_t)(virt)))

static __inline__
pte_t *
vmadrtopte(uintptr_t adr)
{
    uintptr_t mask = ((UINT64_C(1) << (NPTEPGSHIFT + NPDEPGSHIFT
                                       + NPDPEPGSHIFT + NPML4EPGSHIFT - 1)));

    return (PTMAP + ((adr >> PAGESHIFT) & mask));
}

static __inline__
pde_t *
vmadrtopde(uintptr_t adr)
{
    uintptr_t mask = ((UINT64_C(1) << (NPDEPGSHIFT
                                       + NPDPEPGSHIFT + NPML4EPGSHIFT - 1)));

    return (PDMAP + ((adr >> PDRSHIFT) & mask));
}

#define pte_load_store(ptep, pte) m_swap(ptep, pte)
#define pte_load_clear(ptep)      m_swap(ptep, 0)
#define pte_store(ptep, pte)      (*(uintptr_t *)(ptep) = (uintptr_t)(pte))
#define pte_clear(ptep)           pte_store(ptep, 0)
#define pde_store(pdep, pde)      pte_store(pdep, pde)

#endif /* defined(__KERNEL__) */

#define PMAP_PT_X86 0   // regular x86 page tables
#define PMAP_EPT    1   // Intel's nested page tables
#define PMAP_PT_RVI 2   // AMD's nested page tables

struct vmmachpage {
    struct vmmachpage *list;
    int                gen;
    int                pat;
};

#define PMAP_PCID_NONE    0xffffffff
#define PMAP_PCID_KERN    0
#define PMAP_PCID_OVERMAX 0x1000

struct vmpmappcids {
    uint32_t pcid;
    uint32_t gen;
};

#if 0
struct vmpagemap {
    pde_t *dir; // page directory address
};
#endif

struct vmmapentry {
    void              *adr;
    struct vmmapentry *next;
};

struct vmpagemap {
    volatile long  lk;
    pml4e_t       *pml4tab;
    uint64_t       cr3;
};

#endif /* __x86_64__ || __amd64__ */

#define _NPCM   3
#define _NPCPV  168
struct memchunk {
    struct vmpagemap  *pagemap;
    struct memchunk   *list;
    struct memchunk   *lruqueue;
    uint64_t           bitmap[_NPCM];  /* bitmap; 1 = free */
    struct vmmapentry  mapqueue[_NPCPV];
};

#endif /* __UNIT_X86_64_VM_H__ */

