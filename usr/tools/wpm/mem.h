#ifndef __WPM_MEM_H__
#define __WPM_MEM_H__

#include <stddef.h>
#include <string.h>
#include <stdint.h>

#if defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__)
typedef uint32_t wpmpage_t;
typedef uint32_t wpmsize_t;
typedef uint32_t wpmmemadr_t;
#endif

void        wpminitmem(wpmsize_t nbphys);
wpmmemadr_t mempalloc(wpmsize_t size);
void        mempfree(wpmmemadr_t adr);

extern wpmpage_t *mempagetab;
extern uint8_t   *physmem;

#define MINBKT      12
#define MEMSIZE     (128 * 1024 * 1024)
#define MEMHWBASE   0xc0000000

#define PAGEPRES    0x00000001
#define PAGEDIRTY   0x00000002
#define PAGEADRMASK 0xfffff000

#define pagenum(adr)                                                    \
    ((adr) >> 12)
#define pageofs(adr) ((adr) & ((1U << 12) - 1))
#define slabadr(slab) ((slab - memslabtab) << MINBKT)

struct slab {
    uint32_t     bkt;
    uint32_t     free;
    struct slab *prev;
    struct slab *next;
};

static __inline__ long
ceil2(size_t size)
{
    size--;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
#if (LONGSIZE == 8)
    size |= size >> 32;
#endif
    size++;

    return size;
}

static __inline__ void
memstoreq(int64_t src, uint32_t virt)
{
    int64_t *ptr = NULL;

    if (virt < MEMSIZE) {
        ptr = (int64_t *)(&physmem[virt]);
    } else if (virt < MEMHWBASE) {
        ptr = (int64_t *)((int8_t *)mempagetab[pagenum(virt)] + pageofs(virt));
    }
    if (!ptr) {
        fprintf(stderr, "illegal write at address 0x%x\n", virt);

        exit(1);
    }
    *ptr = src;

    return;
}

static __inline__ void
memstorel(int32_t src, uint32_t virt)
{
    int32_t *ptr = NULL;

    if (virt < MEMSIZE) {
        ptr = (int32_t *)(&physmem[virt]);
    } else if (virt < MEMHWBASE) {
        ptr = (int32_t *)((int8_t *)mempagetab[pagenum(virt)] + pageofs(virt));
    }
    if (!ptr) {
        fprintf(stderr, "illegal write at address 0x%x\n", virt);

        exit(1);
    }
    *ptr = src;

    return;
}

static __inline__ void
memstoreb(int8_t src, uint32_t virt)
{
    int8_t *ptr = NULL;

    if (virt < MEMSIZE) {
        ptr = (int8_t *)(&physmem[virt]);
    } else if (virt < MEMHWBASE) {
        ptr = (int8_t *)mempagetab[pagenum(virt)] + pageofs(virt);
    }
    if (!ptr) {
        fprintf(stderr, "illegal write at address 0x%x\n", virt);

        exit(1);
    }
    *ptr = src;

    return;
}

static __inline__ void
memstorew(int16_t src, uint32_t virt)
{
    int16_t *ptr = NULL;

    if (virt < MEMSIZE) {
        ptr = (int16_t *)(&physmem[virt]);
    } else if (virt < MEMHWBASE) {
        ptr = (int16_t *)((int8_t *)mempagetab[pagenum(virt)] + pageofs(virt));
    }
    if (!ptr) {
        fprintf(stderr, "illegal write at address 0x%x\n", virt);

        exit(1);
    }
    *ptr = src;

    return;
}

static __inline__ int64_t
memfetchq(uint32_t virt)
{
    int64_t *ptr = NULL;
    int64_t  retval;

    if (virt < MEMSIZE) {
        ptr = (int64_t *)(&physmem[virt]);
    } else if (virt < MEMHWBASE) {
        ptr = (int64_t *)((int8_t *)mempagetab[pagenum(virt)] + pageofs(virt));
    }
    if (!ptr) {
        fprintf(stderr, "illegal read at address %x (%x)\n", virt, MEMSIZE);

        exit(1);
    }
    retval = *ptr;

    return retval;
}

static __inline__ int32_t
memfetchl(uint32_t virt)
{
    int32_t *ptr = NULL;
    int32_t  retval;

    if (virt < MEMSIZE) {
        ptr = (int32_t *)(&physmem[virt]);
    } else if (virt < MEMHWBASE) {
        ptr = (int32_t *)((int8_t *)mempagetab[pagenum(virt)] + pageofs(virt));
    }
    if (!ptr) {
        fprintf(stderr, "illegal read at address %x (%x)\n", virt, MEMSIZE);

        exit(1);
    }
    retval = *ptr;

    return retval;
}

static __inline__ int8_t
memfetchb(uint32_t virt)
{
    int8_t *ptr = NULL;
    int8_t  retval;

    if (virt < MEMSIZE) {
        ptr = (int8_t *)(&physmem[virt]);
    } else if (virt < MEMHWBASE) {
        ptr = (int8_t *)((int8_t *)mempagetab[pagenum(virt)] + pageofs(virt));
    }
    if (!ptr) {
        fprintf(stderr, "illegal read at address %x\n", virt);

        exit(1);
    }
    retval = *ptr;

    return retval;
}

static __inline__ int16_t
memfetchw(uint32_t virt)
{
    int16_t *ptr = NULL;
    int16_t  retval;

    if (virt < MEMSIZE) {
        ptr = (int16_t *)(&physmem[virt]);
    } else if (virt < MEMHWBASE) {
        ptr = (int16_t *)((int8_t *)mempagetab[pagenum(virt)] + pageofs(virt));
    }
    if (!ptr) {
        fprintf(stderr, "illegal read at address %x\n", virt);

        exit(1);
    }
    retval = *ptr;

    return retval;
}

static __inline__ void
memcopy(uint32_t src, uint32_t dest, uint32_t len)
{
    void *srcp = NULL;
    void *destp = NULL;

    if (src < MEMSIZE) {
        srcp = &physmem[src];
    } else if (src < MEMHWBASE) {
        srcp = (int8_t *)mempagetab[pagenum(src)] + pageofs(src);
    }
    if (dest < MEMSIZE) {
        destp = &physmem[dest];
    } else if (dest < MEMHWBASE) {
        destp = (int8_t *)mempagetab[pagenum(dest)] + pageofs(dest);
    }
    memcpy(destp, srcp, len);

    return;
}

#endif /* __WPM_MEM_H__ */

