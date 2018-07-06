#ifndef __KERN_MALLOC_H__
#define __KERN_MALLOC_H__

#include <kern/mem/mem.h>

/* TODO: linmalloc() which works like BSD contigmalloc() */

/* flg arguments for memalloc() */
#define MEM_ZERO        (1 << 0)        // initiliaze memory to all zero-bits
#define MEM_NODUMP      (1 << 1)        // >= PAGESIZE allocs left out of dumps
#define MEM_WAIT        (1 << 2)        // wait for allocation to succeed
#define MEM_USE_RESERVE (1 << 31)       // use reserved memory with MEM_NOWAIT

#include <mach/param.h>
#include <kern/mem/mem.h>
#define  kwalloc(nb)            memalloc(nb, MEMWIRE)
#define  kmalloc(nb)            memalloc(nb, 0)
#define  kcalloc(nb)            memalloc(nb, MEMZERO)
#define  kcwalloc(nb)           memalloc(nb, MEMZERO | MEMWIRE)
#define  kvalloc(nb)            memalloc(PAGESIZE, 0)
#define  kcvalloc(nb)           memalloc(PAGESIZE, MEMZERO)
#define  kcvwalloc(nb)          memalloc(PAGESIZE, MEMZERO | MEMWIRE)
#define  kwtmalloc(nb)          memwtalloc(nb, 0, 0)
#define  kwtwalloc(nb)          memwtalloc(nb, MEMWIRE, 0)
#define  kwtspinmalloc(nb, cnt) memwtalloc(nb, 0, cnt)
#define  kwtspinwalloc(nb, cnt) memwtalloc(nb, MEMWIRE, cnt)
void    *memalloc(size_t nb, long flg);
void    *memwtalloc(size_t nb, long flg, long spin);
void     kfree(void *ptr);

#endif /* __KERN_MALLOC_H__ */

