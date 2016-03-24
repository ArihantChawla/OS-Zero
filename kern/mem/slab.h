#ifndef __KERN_MEM_SLAB_H__
#define __KERN_MEM_SLAB_H__

#include <stdint.h>
#include <limits.h>
#include <zero/param.h>
#include <kern/types.h>
#include <kern/mem/mem.h>
#include <kern/mem/pool.h>

#define __STRUCT_MEMSLAB_PAD                                            \
    (CLSIZE - 3 * PTRSIZE - WORDSIZE)
struct memslab {
    void           *adr;
    m_ureg_t        info;
    struct memslab *prev;
    struct memslab *next;
#if (__STRUCT_MEMSLAB_PAD)
    uint8_t         _pad[__STRUCT_MEMSLAB_PAD];
#endif
};

#define memgethdrnum(hdr, pool)                                         \
    ((uintptr_t)(hdr) - (uintptr_t)(pool)->hdrtab)
#define memgetblknum(ptr, pool)                                         \
    (((uintptr_t)(ptr) - (pool)->base) >> MEMSLABSHIFT)
#define memgetadr(hdr, pool)                                            \
    ((void *)((uint8_t *)(pool)->base                                   \
              + (memgethdrnum(hdr, pool) << MEMMINSIZE)))
#define memgetslab(ptr, pool)                                           \
    ((struct memslab *)(pool)->hdrtab + memgetblknum(ptr, pool))
#define memgetmag(ptr, pool)                                            \
    ((struct memmag *)(pool)->hdrtab + memgetblknum(ptr, pool))

void * slaballoc(struct mempool *pool, unsigned long nb, unsigned long flg);
void   slabfree(struct mempool *pool, void *ptr);

#endif /* __KERN_MEM_SLAB_H__ */

