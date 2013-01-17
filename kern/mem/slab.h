#ifndef __MEM_SLAB_H__
#define __MEM_SLAB_H__

#if defined(__x86_64__) || defined(__amd64__)
#include <kern/mem/slab64.h>
#elif defined(__i386__) || defined(__arm__)
#include <kern/mem/slab32.h>
#endif

#define slablk(bkt)   mtxlk(&_physlktab[bkt], MEMPID);
#define slabunlk(bkt) mtxunlk(&_physlktab[bkt], MEMPID);

#define SLAB_CONST_SIZE_TRICK 1
#if (SLAB_CONST_SIZE_TRICK)
#define slabbkt(sz) slabfastbkt(sz)
#else
#define slabbkt(sz) slabcalcbkt(sz)
#endif

/* slab header and macros for manipulation */
#define SLABFREE    0x01L
#define SLABWIRE    0x02L
#define SLABZERO    0x04L
#define SLABFLGBITS 0x07L
#define NSLABFLGBIT 3

#define slabclrbkt(hp)                                                  \
    ((hp)->nfo &= ~SLABFLGBITS)
#define slabsetbkt(hp, bkt)                                             \
    (slabclrbkt(hp), (hp)->nfo |= ((bkt) << NSLABFLGBIT))
#define slabgetbkt(hp)                                                  \
    ((hp)->nfo >> NSLABFLGBIT)
#define slabisfree(hp)                                                  \
    (((hp)->nfo) & SLABFREE)
#define slabsetfree(hp)                                                 \
    ((hp)->nfo |= SLABFREE)
#define slabclrfree(hp)                                                 \
    ((hp)->nfo &= ~SLABFREE)
#define slabsetflg(hp, flg)                                             \
    ((hp)->nfo |= (flg))
#define slabclrflg(hp)                                                  \
    ((hp)->nfo &= ~SLABFLGBITS)
#define slabsetprev(hp, hdr, tab)                                       \
    (slabclrprev(hp), (hp)->link |= slabhdrnum(hdr, tab))
#define slabsetnext(hp, hdr, tab)                                       \
    (slabclrnext(hp), (hp)->link |= slabhdrnum(hdr, tab) << 16)

extern struct slabhdr *virtslabtab[];
extern struct slabhdr  virthdrtab[];

void  slabinit(struct slabhdr **zone, struct slabhdr *hdrtab,
               unsigned long base, unsigned long size);
void *slaballoc(struct slabhdr **zone, struct slabhdr *hdrtab,
                unsigned long nb, unsigned long flg);
void   slabfree(struct slabhdr **zone, struct slabhdr *hdrtab, void *ptr);

#define SLABMIN      (1UL << SLABMINLOG2)
#define SLABMINLOG2  16 // don't make this less than 16

#if (PTRBITS == 32)
#define SLABNHDR     (1UL << (PTRBITS - SLABMINLOG2))
#define SLABHDRTABSZ (SLABNHDR * sizeof(struct slabhdr))
#define SLABHDRBASE  (VIRTBASE - SLABHDRTABSZ)
#endif

#define slabnum(ptr)                                                    \
    ((uintptr_t)(ptr) >> SLABMINLOG2)
#define slabhdrnum(hdr, tab)                                            \
    ((uintptr_t)((hdr) - (tab)))
#if (PTRBITS == 32)
#define slabadr(hdr, tab)                                               \
    ((void *)(slabhdrnum(hdr, tab) << SLABMINLOG2))
#define slabhdr(ptr, tab)                                               \
    ((tab) + slabnum(ptr))
#else
#define slabhdr(ptr, tab)                                               \
    slabgethdr(ptr, tab)
#endif

/* use compiler optimizations to evaluate bucket for constant allocation size */
#if (SLAB_CONST_SIZE_TRICK)
#if (PTRBITS == 32)
#define slabfastbkt(sz)                                                 \
    ((!__builtin_constant_p(sz)                                         \
      ? slabcalcbkt(sz)                                                 \
      : (((sz) <= SLABMIN)                                              \
         ? SLABMINLOG2                                                  \
         : (((sz) <= (1UL << 4))                                        \
            ? 4                                                         \
            : (((sz) <= (1UL << 5))                                     \
               ? 5                                                      \
               : (((sz) <= (1UL << 6))                                  \
                  ? 6                                                   \
                  : (((sz) <= (1UL << 7))                               \
                     ? 7                                                \
                     : (((sz) <= (1UL << 8))                            \
                        ? 8                                             \
                        : (((sz) <= (1UL << 9))                         \
                           ? 9                                          \
                           : (((sz) <= (1UL << 10))                     \
                              ? 10                                      \
                              : (((sz) <= (1UL << 11))                  \
                                 ? 11                                   \
                                 : (((sz) <= (1UL << 12))               \
                                    ? 12                                \
                                    : (((sz) <= (1UL << 13))            \
                                       ? 13                             \
                                       : (((sz) <= (1UL << 14))         \
                                          ? 14                          \
                                          : (((sz) <= (1UL << 15))      \
                                             ? 15                       \
                                             : (((sz) <= (1UL << 16))   \
                                                ? 16                    \
                                                : (((sz) <= (1UL << 17)) \
                                                   ? 17                 \
                                                   : (((sz) <= (1UL << 18)) \
                                                      ? 18              \
                                                      : (((sz) <= (1UL << 19) \
                                                          ? 19          \
                                                          : (((sz) <= (1UL << 20)) \
                                                             ? 20       \
                                                             : (((sz) <= (1UL << 21)) \
                                                                ? 21    \
                                                                : (((sz) <= (1UL << 22)) \
                                                                   ? 22 \
                                                                   : (((sz) <= (1UL << 23)) \
                                                                      ? 23 \
                                                                      : (((sz) <= (1UL << 24)) \
                                                                         ? 24 \
                                                                         : (((sz) <= (1UL << 25)) \
                                                                            ? 25 \
                                                                            : (((sz) <= (1UL << 26)) \
                                                                               ? 26 \
                                                                               : (((sz) <= (1UL << 27)) \
                                                                                  ? 27 \
                                                                                  : (((sz) <= (1UL << 28)) \
                                                                                     ? 28 \
                                                                                     : (((sz) <= (1UL << 29)) \
                                                                                        ? 29 \
                                                                                        : (((sz) <= (1UL << 30)) \
                                                                                           ? 30 \
                                                                                           : 31)))))))))))))))))))))))))))))))
#elif (PTRBITS == 64)
#define slabfastbkt(sz)                                                 \
    ((!__builtin_constant_p(sz)                                         \
      ? slabcalcbkt(sz)                                                 \
      : (((sz) <= SLABMIN)                                              \
         ? SLABMINLOG2                                                  \
         : (((sz) <= (1UL << 4))                                        \
            ? 4                                                         \
            : (((sz) <= (1UL << 5))                                     \
               ? 5                                                      \
               : (((sz) <= (1UL << 6))                                  \
                  ? 6                                                   \
                  : (((sz) <= (1UL << 7))                               \
                     ? 7                                                \
                     : (((sz) <= (1UL << 8))                            \
                        ? 8                                             \
                        : (((sz) <= (1UL << 9))                         \
                           ? 9                                          \
                           : (((sz) <= (1UL << 10))                     \
                              ? 10                                      \
                              : (((sz) <= (1UL << 11))                  \
                                 ? 11                                   \
                                 : (((sz) <= (1UL << 12))               \
                                    ? 12                                \
                                    : (((sz) <= (1UL << 13))            \
                                       ? 13                             \
                                       : (((sz) <= (1UL << 14))         \
                                          ? 14                          \
                                          : (((sz) <= (1UL << 15))      \
                                             ? 15                       \
                                             : (((sz) <= (1UL << 16))   \
                                                ? 16                    \
                                                : (((sz) <= (1UL << 17)) \
                                                   ? 17                 \
                                                   : (((sz) <= (1UL << 18)) \
                                                      ? 18              \
                                                      : (((sz) <= (1UL << 19) \
                                                          ? 19          \
                                                          : (((sz) <= (1UL << 20)) \
                                                             ? 20       \
                                                             : (((sz) <= (1UL << 21)) \
                                                                ? 21    \
                                                                : (((sz) <= (1UL << 22)) \
                                                                   ? 22 \
                                                                   : (((sz) <= (1UL << 23)) \
                                                                      ? 23 \
                                                                      : (((sz) <= (1UL << 24)) \
                                                                         ? 24 \
                                                                         : (((sz) <= (1UL << 25)) \
                                                                            ? 25 \
                                                                            : (((sz) <= (1UL << 26)) \
                                                                               ? 26 \
                                                                               : (((sz) <= (1UL << 27)) \
                                                                                  ? 27 \
                                                                                  : (((sz) <= (1UL << 28)) \
                                                                                     ? 28 \
                                                                                     : (((sz) <= (1UL << 29)) \
                                                                                        ? 29 \
                                                                                        : (((sz) <= (1UL << 30)) \
                                                                                           ? 30 \
                                                                                           : (((sz) <= (1UL << 31)) \
                                                                                              ? 31 \
                                                                                              : (((sz) <= (1UL << 32)) \
                                                                                                 ? 32 \
                                                                                                 : (((sz) <= (1UL << 33)) \
                                                                                                    ? 33 \
                                                                                                    : (((sz) <= (1UL << 34)) \
                                                                                                       ? 34 \
                                                                                                       : (((sz) <= (1UL << 35)) \
                                                                                                          ? 35 \
                                                                                                          :(((sz) <= (1UL << 36)) \
                                                                                                            ? 36 \
                                                                                                            : (((sz) <= (1UL << 37)) \
                                                                                                               ? 37 \
                                                                                                               : (((sz) <= (1UL << 38)) \
                                                                                                                  ? 38 \
                                                                                                                  : (((sz) <= (1UL << 39)) \
                                                                                                                     ? 39 \
                                                                                                                     : (((sz) <= (1UL << 40)) \
                                                                                                                        ? 40 \
                                                                                                                        : (((sz) <= (1UL << 41)) \
                                                                                                                           ? 41 \
                                                                                                                           : (((sz) <= (1UL << 42)) \
                                                                                                                              ? 42 \
                                                                                                                              : (((sz) <= (1UL << 7)) \
                                                                                                                                 ? 43 \
                                                                                                                                 : (((sz) <= (1UL << 7)) \
                                                                                                                                    ? 44 \
                                                                                                                                    : (((sz) <= (1UL << 7)) \
                                                                                                                                       ? 45 \
                                                                                                                                       : (((sz) <= (1UL << 7)) \
                                                                                                                                          ? 46 \
                                                                                                                                          : (((sz) <= (1UL << 7)) \
                                                                                                                                             ? 47 \
                                                                                                                                             : (((sz) <= (1UL << 7)) \
                                                                                                                                                ? 48 \
                                                                                                                                                : (((sz) <= (1UL << 7)) \
                                                                                                                                                   ? 49 \
                                                                                                                                                   : (((sz) <= (1UL << 7)) \
                                                                                                                                                      ? 50 \
                                                                                                                                                      : (((sz) <= (1UL << 7)) \
                                                                                                                                                         ? 51))))))))))))))))))))))))))))))))))))))))))))))))))))
#endif
#endif

/* calculate bucket for allocation of size. */
static __inline__ unsigned long
slabcalcbkt(unsigned long size)
{
    unsigned long tmp = size;
    unsigned long bkt = 0;

    if (!powerof2(tmp)) {
        tmp--;
        tmp |= tmp >> 1;
        tmp |= tmp >> 2;
        tmp |= tmp >> 4;
        tmp |= tmp >> 8;
        tmp |= tmp >> 16;
#if (LONGSIZE == 8)
        tmp |= tmp >> 32;
#endif
        tmp++;
    }
#if (LONGSIZE == 4)
    tzero32(tmp, bkt);
#elif (LONGSIZE == 8)
    tzero64(tmp, bkt);
#endif

    return bkt;
}

#endif /* __MEM_SLAB_H__ */

