#ifndef __KERN_MEM_SLAB64_H__
#define __KERN_MEM_SLAB64_H__

#define memslabgetprev(hp, slab)                                        \
    ((hp)->prev)
#define memslabgetnext(hp, slab)                                        \
    ((hp)->next)
#define memslabclrprev(hp)                                              \
    ((hp)->prev = NULL)
#define memslabclrnext(hp)                                              \
    ((hp)->next = NULL)
#define memslabclrlink(hp)                                              \
    ((hp)->prev = (hp)->next = NULL)
#define memslabsetprev(hp, hdr, slab)                                   \
    ((hp)->prev = (hdr))
#define memslabsetnext(hp, hdr, slab)                                   \
    ((hp)->next = (hdr))

#endif /* __KERN_MEM_SLAB64_H__ */

