#ifndef __KERN_MALLOC_H__
#define __KERN_MALLOC_H__

#include <kern/mem/mem.h>

/* TODO: linmalloc() which works like BSD contigmalloc() */

/* flg arguments for memalloc() */
#define MEM_ZERO        (1 << 0)        // initiliaze memory to all zero-bits
#define MEM_NODUMP      (1 << 1)        // > PAGESIZE allocs left out of dumps
#define MEM_NOWAIT      (1 << 2)        // fail instead of waiting
#define MEM_WAITOK      (1 << 3)        // okay to wait for some allocations
#define MEM_USE_RESERVE (1 << 31)       // use reserved memory with MEM_NOWAIT

#include <zero/param.h>
#include <kern/mem/mem.h>
#define  kwalloc(nb)   memalloc(nb, MEMWIRE)
#define  kmalloc(nb)   memalloc(nb, 0)
#define  kcalloc(nb)   memalloc(nb, MEMZERO)
#define  kcwalloc(nb)  memalloc(nb, MEMZERO | MEMWIRE)
#define  kvalloc(nb)   memalloc(PAGESIZE, 0)
#define  kcvalloc(nb)  memalloc(PAGESIZE, MEMZERO)
#define  kcvwalloc(nb) memalloc(PAGESIZE, MEMZERO | MEMWIRE)
void    *memalloc(size_t nb, long flg);
void     kfree(void *ptr);

#endif /* __KERN_MALLOC_H__ */

