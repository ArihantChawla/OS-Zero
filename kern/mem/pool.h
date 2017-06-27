#ifndef __KERN_MEM_POOL_H__
#define __KERN_MEM_POOL_H__

#include <stdint.h>
#include <zero/param.h>
#include <zero/asm.h>
#include <kern/mem/bkt.h>

struct kmempool {
    struct kmembkt  tab[PTRBITS];
    m_atomic_t      lk;
    uintptr_t       base;
    unsigned long   nblk;
    void           *hdrtab;
};

#endif /* __KERN_MEM_POOL_H__ */

