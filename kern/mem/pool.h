#ifndef __KERN_MEM_POOL_H__
#define __KERN_MEM_POOL_H__

#include <stdint.h>
#include <zero/param.h>
#include <kern/mem/bkt.h>

struct mempool {
    struct membkt  tab[PTRBITS];
    volatile long  lk;
    uintptr_t      base;
    unsigned long  nblk;
    void          *hdrtab;
};

#endif /* __KERN_MEM_POOL_H__ */

