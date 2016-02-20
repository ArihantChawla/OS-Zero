#ifndef __KERN_MEM_VM_H__
#define __KERN_MEM_VM_H__

#include <stdint.h>
#include <zero/param.h>
//#include <zero/spinrw.h>

#if (PAGESIZE == 4096)
#define VMPAGEBITS 0xff
typedef uint8_t    vmpagebits;
#elif (PAGESIZE == 8192)
#define VMPAGEBITS 0xffff
typedef uint16_t   vmpagebits;
#elif (PAGESIZE == 16384)
#define VMPAGEBITS 0xffffffff
typedef uint32_t   vmpagebits;
#elif (PAGESIZE == 8192)
#define VMPAGEBITS UINT64_C(0xffffffffffffffff)
typedef uint64_t   vmpagebits;
#endif

#if (defined(__i386__) || defined(__i486__)                             \
     || defined(__i586__) || defined(__i686__))                         \
    && !defined(__x86_64__) && !defined(__amd64__)
#include <kern/unit/ia32/vm.h>
#elif defined(__x86_64__) || defined(__amd64__)
#include <kern/unit/x86-64/vm.h>
#endif

#define VM_PROT_NONE       0x00
#define VM_PROT_READ       0x01
#define VM_PROT_WRITE      0x02
#define VM_PROT_EXECUTE    0x04
#define VM_PROT_COPY       0x08

#define VM_PROT_ALL        (VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE)
#define VM_PROT_RW         (VM_PROT_READ | VM_PROT_WRITE)
#define VM_PROT_DEFAULT    VM_PROT_ALL

/* flg-bits */
#define VM_PAGE_CACHED     0x0001       // page is cached
#define VM_PAGE_FICTITIOUS 0x0004       // physical page doesn't exist
#define VM_PAGE_ZERO       0x0008       // page is zeroed
#define VM_PAGE_MARKER     0x0010       // special queue marker page
#define VM_PAGE_WINATCFLS  0x0020       // flush dirty page on inactive queue
#define VM_PAGE_NODUMP     0x0040       // don't dump this page with core
#define VM_PAGE_UNHOLDFREE 0x0080       // delayed free of a held page
/* aflg-bits - updated atomically */
#define VM_PAGE_WRITEABLE  0x01
#define VM_PAGE_REFERENCED 0x02
#define VM_PAGE_EXECUTABLE 0x04
/* objflg-bits */
#define VM_OBJ_UNUSED      0x01 // AVAILABLE
#define VM_OBJ_SWAPSLEEP   0x02 // waiting for swap to finish
#define VM_OBJ_UNMANAGED   0x04 // no´paging/management
#define VM_OBJ_SWAPINPROG  0x08 // swap I/O in progress on page
#define VM_OBJ_NOSYNC      0x10 // don't collect for syncer
/* qid-values */
#define VM_PQ_NONE         0xff
#define VM_PQ_INACTIVE     0x00
#define VM_PQ_ACTIVE       0x01
#define VM_PQ_COUNT        0x02
struct vmpage {
    volatile long  lk;
    uint8_t       *physadr;
    volatile long  nwire;
    volatile long  nhold;
    uint16_t       usecnt;
    uint16_t       flg;
    uint8_t        aflg;        // FIXME: use atomic operations for this :)
    uint8_t        objflg;
    uint8_t        qid;
    uint8_t        pool;
    vmpagebits     valid;       // map of valid DEV_BSIZE chunks
    vmpagebits     dirty;       // map of dirty DEV_BSIZE chunks
};

struct vmpagequeue {
    volatile long  lk;
    struct vmpage *list;
    long           npage;
};

struct vmaffinity {
    uint8_t *base;
    uint8_t *end;
    long     domain;
};

#define VM_FREEPOOL_DEFAULT 0   // default pool of physical pages
#define VM_FREEPOOL_DIRECT  1   // pool for page tables and small allocations
#define VM_NFREEPOOL        2

struct vmphysseg {
    uint8_t       *base;
    uint8_t       *end;
    struct vmpage *firstpage;
    long           domain;
};

struct vmdomain {
    struct vmpagequeue queuetab[VM_PQ_COUNT];   // inactive and active queues
    long               npage;                   // page count
    long               nfree;                   // free count
    long               segbits;                 // bitmasks of segments
    long               empty;                   // boolean value
    long               pagedpass;               // local pagedaemon pass
    long               lastscan;
    struct vmpage      marker;                  // pagedaemon private use
};

#endif /* __KERN_MEM_VM_H__ */

