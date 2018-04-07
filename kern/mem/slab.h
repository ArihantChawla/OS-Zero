#ifndef __KERN_MEM_SLAB_H__
#define __KERN_MEM_SLAB_H__

#include <stdint.h>
#include <limits.h>
#include <zero/param.h>
#include <kern/types.h>

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

#define memgethdrnum(hdr, zone)                                         \
    ((uintptr_t)(hdr) - (uintptr_t)(zone)->hdrtab)
#define memgetblknum(ptr, zone)                                         \
    (((uintptr_t)(ptr) - (zone)->base) >> MEMSLABSHIFT)
#define memgetadr(hdr, zone)                                            \
    ((void *)((uint8_t *)(zone)->base                                   \
              + (memgethdrnum(hdr, zone) << MEMMINSHIFT)))
#define memgetslab(ptr, zone)                                           \
    ((struct memslab *)(zone)->hdrtab + memgetblknum(ptr, zone))
#define memgetmag(ptr, zone)                                            \
    ((struct memmag *)(zone)->hdrtab + memgetblknum(ptr, zone))

#endif /* __KERN_MEM_SLAB_H__ */

