#ifndef __KERN_MEM_VM_H__
#define __KERN_MEM_VM_H__

#include <kern/conf.h>
#include <stdint.h>
#include <sys/param.h>
#include <mach/param.h>
#include <zero/mtx.h>
//#include <zero/spinrw.h>

#if (PAGESIZE / DEV_BSIZE <= 8)
#define VMBLKBITS 0xff
typedef uint8_t    vmblkbits;
#elif (PAGESIZE / DEV_BSIZE <= 16)
#define VMBLKBITS 0xffff
typedef uint16_t   vmblkbits;
#elif (PAGESIZE / DEV_BSIZE <= 32)
#define VMBLKBITS 0xffffffff
typedef uint32_t   vmblkbits;
#elif (PAGESIZE / DEV_BSIZE <= 64)
#define VMBLKBITS UINT64_C(0xffffffffffffffff)
typedef uint64_t   vmblkbits;
#else
#error define vmblkbits in <kern/mem/vm.h>
#endif

#if (defined(__i386__) || defined(__i486__)                             \
     || defined(__i586__) || defined(__i686__)                          \
     || defined(__x86_64__) || defined(__amd64__))
#include <kern/unit/x86/vm.h>
#endif

#define VM_LK_T         zerofmtx
#define vmlk(lp)        fmtxlk(lp)
#define vmunlk(lp)      fmtxunlk(lp)

#define VM_PROT_NONE    0
#define VM_PROT_EXECUTE (1 << 0)
#define VM_PROT_WRITE   (1 << 1)
#define VM_PROT_READ    (1 << 2)
#define VM_PROT_COPY    (1 << 3)

#define VM_PROT_ALL     (VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE)
#define VM_PROT_RW      (VM_PROT_READ | VM_PROT_WRITE)
#define VM_PROT_DEFAULT VM_PROT_ALL

#if (VMPAGETKTLK)
#include <zero/tktlk.h>
#define VMPAGE_LK_T     union zerotktlk
//#define vmspinlk(tp)    tktlkspin(tp)
#define vmlkpage(tp)    tktlk(tp)
#define vmunlkpage(tp)  tktunlk(tp)
#else
#define VMPAGE_LK_T     zerofmtx
#define vmlkpage(lp)    fmtxlk(lp)
#define vmunlkpage(lp)  fmtxunlk(lp)
#include <zero/mtx.h>
#endif

struct vmpage {
    VMPAGE_LK_T       lk;       // mutual exclusion lock
    m_ureg_t          adr;      // page [virtual] address
    m_atomic_t        nref;     // number of active references
    m_atomic_t        nmap;     // number of page-faults generated
    struct vmpage    *prev;     // previous on queue
    struct vmpage    *next;     // next on queue
    struct perm      *perm;     // permissions
    struct vmpageatr *vmatr;    // VM page-attributes
};

/* flg-bits - updated atomically */
#define VM_PAGE_EXECUTABLE (1 << 0)
#define VM_PAGE_WRITEABLE  (1 << 1)
#define VM_PAGE_READABLE   (1 << 2)
#define VM_PAGE_REFERENCED (1 << 3)
/* flg-bits */
#define VM_PAGE_UNUSED     (1 << 4)  // AVAILABLE
#define VM_PAGE_SWAPSLEEP  (1 << 5)  // waiting for swap to finish
#define VM_PAGE_UNMANAGED  (1 << 6)  // no´paging/management
#define VM_PAGE_SWAPINPROG (1 << 7)  // swap I/O in progress on page
#define VM_PAGE_NOSYNC     (1 << 8)  // don't collect for syncer
#define VM_PAGE_CACHED     (1 << 9)  // page is cached
#define VM_PAGE_FICTITIOUS (1 << 10) // physical page doesn't exist
#define VM_PAGE_ZERO       (1 << 11) // page is zeroed
#define VM_PAGE_MARKER     (1 << 12) // special queue marker page
#define VM_PAGE_WINATCFLS  (1 << 13) // flush dirty page on inactive queue
#define VM_PAGE_NODUMP     (1 << 14) // don't dump this page with core
#define VM_PAGE_UNHOLDFREE (1 << 15) // delayed free of a held page
/* qid-values */
#define VM_QUEUE_NONE       0xff
#define VM_QUEUE_INACTIVE   0x00
#define VM_QUEUE_ACTIVE     0x01
#define VM_QUEUE_COUNT      0x02
struct vmpageatr {
    uint8_t    *phys;   // page physical-address
    m_atomic_t  flg;    // page-flags
    m_atomic_t  nwire;  // # of wires
    m_atomic_t  nhold;  // # of holders
    m_ureg_t    pid;    // process ID
    vmblkbits   valid;  // map of valid device blocks
    vmblkbits   dirty;  // map of dirty device blocks
    uint8_t     qid;    // LRU-queue ID
    uint8_t     zone;   // memory zone ID
};

struct vmaffinity {
    uint8_t *base;
    uint8_t *end;
    long     domain;
};

#define VM_FREEZONE_DEFAULT 0   // default zone of physical pages
#define VM_FREEZONE_DIRECT  1   // zone for page tables and small allocations
#define VM_NFREEZONE        2

struct vmphysseg {
    uint8_t     *base;
    uint8_t     *end;
    struct vmpage *firstpage;
    long         domain;
};

struct vmdomain {
    struct vmpage queuetab[VM_QUEUE_COUNT]; // inactive and active queues
    long          npage;        // page count
    long          nfree;        // free count
    long          segbits;      // bitmasks of segments
    long          empty;        // boolean value
    long          pagedpass;    // local pagedaemon pass
    long          lastscan;
    struct vmpage marker;       // pagedaemon private use
};

#endif /* __KERN_MEM_VM_H__ */

