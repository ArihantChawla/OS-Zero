#ifndef __UNIT_IA32_VM_H__
#define __UNIT_IA32_VM_H__

#include <stdint.h>
//#include <kern/buf.h>

#include <kern/unit/ia32/link.h>

#if !defined(__KERNEL__)
#define __KERNEL__ 1
#endif
#include <zero/mtx.h>

void  vminitphys(uintptr_t base, unsigned long nbphys);
void *vmmapvirt(uint32_t *pagetab, void *virt, uint32_t size, uint32_t flags);
void  vmfreephys(void *virt, uint32_t size);
void  vmmapseg(uint32_t *pagetab, uint32_t virt, uint32_t phys, uint32_t lim,
               uint32_t flg);

#define KERNVIRTBASE      0xc0000000U
#define vmlinkadr(adr)    ((uint32_t)(adr) - KERNVIRTBASE)
#define vmphysadr(adr)    ((uintptr_t)(((uint32_t *)&_pagetab)[vmpagenum(adr)]) & VMPGMASK)
#define vmpagedirnum(adr) ((uint32_t)(adr) >> PDSHIFT)
#define vmpagenum(adr)    ((uint32_t)(adr) >> PTSHIFT)
#define vmpageofs(adr)    ((uint32_t)(adr) & (PAGESIZE - 1))

#define vmpageid(adr)     ((uint32_t)(adr) >> PAGESIZELOG2)
#define vmbufid(adr)      ((uint32_t)(adr) >> PAGESIZELOG2)
#define vmisbufadr(adr)   (!((uint32_t)(adr) & (BUFSIZE - 1)))

//#define vmpageadr(pg, pt) (((pg) - (pt)) << PAGESIZELOG2)

/* internal macros */

static __inline__ void
vmflushtlb(void *adr)
{
    __asm__ __volatile__ ("invlpg (%0)\n" : : "r" (adr) : "memory");
}

/* physical memory limit; leave high areas for devices */
#define DEVMEMBASE      0xe0000000                      // 3.5 G

/* virtual memory parameters */
#define NPAGEMAX        (NPDE * NPTE)                   // # of virtual pages
#define NPDE            1024                            // per directory
#define NPTE            1024                            // per table
//#define PAGETAB         0x00700000U                     // physical address
#define PAGETABSIZE     (NPDE * NPTE * sizeof(uint32_t))
#define PDSHIFT         22
#define PTSHIFT         12
#define VMPDMASK        0xffc00000                      // top 10 bits
#define VMPTMASK        0x003ff000                      // bits 12..21
#define VMPGMASK        0xfffff000U                     // page frame; 22 bits

/* page structure setup */

/*
 * page flags
 */
/* standard IA-32 flags */
#define PAGEPRES     0x00000001U	// present
#define PAGEWRITE    0x00000002U	// writeable
#define PAGEUSER     0x00000004U	// user-accessible
#define PAGENOCACHE  0x00000010U        // disable caching
#define PAGEREF      0x00000020U	// has been referenced
#define PAGEDIRTY    0x00000040U	// has been written to
#define PAGESUPER    0x00000080U	// 4M page
#define PAGEGLOBAL   0x00000100U	// global
#define PAGESYS1     0x00000200U	// reserved for system
#define PAGESYS2     0x00000400U	// reserved for system
#define PAGESYS3     0x00000800U	// reserved for system
/* custom flags */
#define PAGESWAPPED  PAGESYS1           // swapped out
#define PAGEBUF      PAGESYS2           // buffer cache
#define PAGEWIRED    PAGESYS3           // wired
#define PAGESYSFLAGS (PAGESWAPPED | PAGEBUF | PAGEWIRED)

/* page fault management */

/* page fault exception */
#define NPAGEDEV     16
//#define pfdev(adr)  (((adr) & PFDEVMASK) >> 3)
#define pfadr(adr)   ((adr) & PFPAGEMASK)
#define PFPRES       0x00000001U	// page is present
#define PFWRITE      0x00000002U	// write fault
#define PFUSER       0x00000004U	// user fault
#define PFFLGMASK    0x00000007U
#define PFADRMASK    0xfffffff8U
#define PFPAGEMASK   0xfffff000U

struct vmpagestat {
    unsigned long nphys;
    unsigned long nmapped;
    unsigned long nbuf;
    unsigned long nwired;
};

#define VMBUFNREFMASK 0x07
#define vmsetbufnref(bp, npg)                                           \
    ((bp)->prev = (void *)((uint32_t)((bp)->prev)                       \
                           | (((npg) & VMBUFNREFMASK) << 4)),           \
     (bp)->next = (void *)((uint32_t)((bp)->next)                       \
                           | (((npg) >> 4) & VMBUFNREFMASK)))
#define vmgetbufnref(bp)                                                \
    (((uint32_t)((bp)->prev) & VMBUFNREFMASK)                           \
     | (((uint32_t)((bp)->next) & VMBUFNREFMASK) << 4))
#define vmgetprevbuf(bp)                                                \
    ((void *)((uint32_t)((bp)->prev) & ~VMBUFNREFMASK))
#define vmgetnextbuf(bp)                                                \
    ((void *)((uint32_t)((bp)->next) & ~VMBUFNREFMASK))
struct vmbuf {
    struct vmbuf *prev;
    struct vmbuf *next;
};

struct vmbufq {
    long           lk;
    struct vmbuf *head;
    struct vmbuf *tail;
};

#define vmaddbuf(adr)                                                   \
    do {                                                                \
        struct vmbufq *_bufq = &vmbufq;                                 \
        struct vmbuf  *_hdrtab = vmbuftab;                              \
        struct vmbuf  *_buf = &_hdrtab[vmbufid(adr)];                   \
        struct vmbuf  *_head;                                           \
                                                                        \
        _buf->prev = NULL;                                              \
        mtxlk(&vmbufq.lk);                                              \
        _head = _bufq->head;                                            \
        _buf->next = _head;                                             \
        if (_head) {                                                    \
            _head->prev = _buf;                                         \
        } else {                                                        \
            _bufq->tail = _buf;                                         \
        }                                                               \
        _bufq->head = _buf;                                             \
        mtxunlk(&vmbufq.lk);                                            \
    } while (0)

#define vmrmbuf(adr)                                                    \
    do {                                                                \
        struct vmbufq *_bufq = &vmbufq;                                 \
        struct vmbuf  *_hdrtab = vmbuftab;                              \
        struct vmbuf  *_buf = &_hdrtab[vmbufid(adr)];                   \
        struct vmbuf  *_tmp;                                            \
                                                                        \
        mtxlk(&vmbufq.lk);                                              \
        _tmp = _buf->prev;                                              \
        if (_tmp) {                                                     \
            _tmp->next = _buf->next;                                    \
        } else {                                                        \
            _tmp = _buf->next;                                          \
            _bufq->head = _tmp;                                         \
            if (_tmp) {                                                 \
                _tmp->prev = _buf->prev;                                \
            } else {                                                    \
                _bufq->tail = _tmp;                                     \
            }                                                           \
            _bufq->head = _tmp;                                         \
        }                                                               \
        _tmp = _buf->next;                                              \
        if (_tmp) {                                                     \
            _tmp->prev = _buf->prev;                                    \
        } else {                                                        \
            _tmp = _buf->prev;                                          \
            _bufq->tail = _tmp;                                         \
            if (_tmp) {                                                 \
                _tmp->next = NULL;                                      \
            } else {                                                    \
                _bufq->head = _bufq->tail = _tmp;                       \
            }                                                           \
        }                                                               \
        mtxunlk(&vmbufq.lk);                                            \
    } while (0)

#define vmdeqpage(rpp)                                                  \
    do {                                                                \
        struct vmpageq *_pageq = &curproc->pagelruq;                    \
        struct vmpage  *_tail;                                          \
                                                                        \
        vmlklruq(_pageq);                                               \
        _tail = _pageq->tail;                                           \
        if (_tail) {                                                    \
            if (_tail->prev) {                                          \
                _tail->prev->next = NULL;                               \
            } else {                                                    \
                _pageq->head = NULL;                                    \
            }                                                           \
            _pageq->tail = _tail->prev;                                 \
        }                                                               \
        *(rpp) = _tail;                                                 \
        vmunlklruq(_pageq);                                             \
    } while (0)

#endif /* __UNIT_IA32_VM_H__ */

