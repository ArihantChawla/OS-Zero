#ifndef __UNIT_IA32_VM_H__
#define __UNIT_IA32_VM_H__

#include <stdint.h>
#include <zero/param.h>
//#include <zero/mtx.h>
//#include <kern/buf.h>
#include <kern/types.h>
#include <kern/unit/x86/link.h>

extern long kernlongmode;

void vminitphys(uintptr_t base, unsigned long nbphys);
void vminitvirt(void *pagetab, void *virt, uint32_t size, uint32_t flg);
void vmfreephys(void *virt, uint32_t size);
void vmmapseg(void *pagetab, uint32_t virt, uint32_t phys, uint32_t lim,
              uint32_t flg);

#define KERNVIRTBASE      0xc0000000U
#define vmlinkadr(adr)    ((uint32_t)(adr) - KERNVIRTBASE)
#define vmphysadr(adr)    ((uintptr_t)(((uint32_t *)&_pagetab)[vmpagenum(adr)]) & VMPAGEMASK)
#define vmpagedirnum(adr) ((uint32_t)(adr) >> PDSHIFT)
#define vmpagenum(adr)    ((uint32_t)(adr) >> PTSHIFT)
#define vmpageofs(adr)    ((uint32_t)(adr) & (PAGESIZE - 1))

#define vmpageid(adr)     ((uint32_t)(adr) >> PAGESIZELOG2)
#define vmbufid(adr)      ((uint32_t)(adr) >> PAGESIZELOG2)
#define vmisbufadr(adr)   (!((uint32_t)(adr) & (BUFSIZE - 1)))

/* internal macros */

static __inline__ void
vmflushtlb(void *adr)
{
    __asm__ __volatile__ ("invlpg (%0)\n" : : "r" (adr) : "memory");
}

/* physical memory limit; leave high areas for devices */
//#define DEVMEMBASE      0xe0000000      // 3.5 G

/* virtual memory parameters */
//#define NPAGEMAX        (NPDE * NPTE)   // # of virtual pages
#define NPDE            1024            // per directory
#define NPTE            1024            // per table
#define PAGETABSIZE     (NPDE * NPTE * sizeof(uint32_t))
#define PDSHIFT         22
#define PTSHIFT         12
#define VMPDMASK        0xffc00000      // top 10 bits
#define VMPTMASK        0x003ff000      // bits 12..21
#define VMPAGEMASK      0xfffff000U     // page frame; 22 bits

/* page structure setup */

/*
 * page flags
 */
/* standard IA-32 flags */
#define PAGEPRES        0x00000001U	// present
#define PAGEWRITE       0x00000002U	// writeable
#define PAGEUSER        0x00000004U	// user-accessible
#define PAGEWRITETHRU   0x00000008U
#define PAGENOCACHE     0x00000010U     // disable caching
#define PAGEREF         0x00000020U	// has been referenced
#define PAGEDIRTY       0x00000040U	// has been written to
#define PAGESUPERPTE    0x00000080U	// 4M/2M page
#define PAGEGLOBAL      0x00000100U	// global
#define PAGESYS1        0x00000200U	// reserved for system
#define PAGESYS2        0x00000400U	// reserved for system
#define PAGESYS3        0x00000800U	// reserved for system
#define PAGESUPERPDE    0x00001000U
/* custom flags */
#define PAGESWAPPED     PAGESYS1        // swapped out
//#define PAGEBUF         PAGESYS2        // buffer cache
#define PAGEWIRED       PAGESYS3        // wired
#define PAGESYSFLAGS    (PAGESWAPPED | PAGEWIRED)

/* page fault management */

/* page fault exception */
#define NPAGEDEV     16
//#define pfltdev(adr)  (((adr) & PAGEFLTDEVMASK) >> 3)
#define pagefltadr(adr) ((adr) & PAGEFLTPAGEMASK)
#define PAGEFLTPRES     0x00000001U	// page is present
#define PAGEFLTWRITE    0x00000002U	// write fault
#define PAGEFLTUSER     0x00000004U	// user fault
#define PAGEFLTRESBIT   0x00000008U     // 1 in reserved bit
#define PAGEFLTINST     0x00000010U     // caused by instruction fetch
#define PAGEFLTFLGMASK  0x0000001fU
#define PAGEFLTADRMASK  0xffffffe0U
#define PAGEFLTPAGEMASK 0xfffff000U

struct vmpagemap {
    pde_t *dir; // page directory address
    pte_t *tab; // flat page-table of NPDE * NPTE entries
};

struct vmpagestat {
    unsigned long  nphys;
    unsigned long  nvirt;
    unsigned long  nmap;
    unsigned long  nbuf;
    unsigned long  nwire;
    void          *phys;
    void          *physend;
    void          *buf;
    void          *bufend;
};

#endif /* __UNIT_IA32_VM_H__ */

