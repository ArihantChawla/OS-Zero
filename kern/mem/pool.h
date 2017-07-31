#ifndef __KERN_MEM_POOL_H__
#define __KERN_MEM_POOL_H__

#include <stddef.h>
#include <stdint.h>
#include <zero/param.h>
#include <zero/asm.h>
#include <zero/trix.h>
#include <kern/mem/bkt.h>

struct kmempool {
    struct kmembkt  tab[PTRBITS];
    uintptr_t       base;
    unsigned long   nblk;
    void           *hdrtab;
    m_atomic_t      lk;
    uint8_t         _pad[CLSIZE
                         - sizeof(uintptr_t)
                         - sizeof(unsigned long)
                         - sizeof(void *)
                         - sizeof(m_atomic_t)];
};

#endif /* __KERN_MEM_POOL_H__ */

