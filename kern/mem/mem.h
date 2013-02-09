#ifndef __MEM_MEM_H__
#define __MEM_MEM_H__

#include <zero/trix.h>

#define MEM_CONST_SIZE_TRICK 1
#if (MEM_CONST_SIZE_TRICK)
#define membkt(sz) memfastbkt(sz)
#else
#define membkt(sz) memcalcbkt(sz)
#endif

/* calculate bucket for allocation of size. */
static __inline__ unsigned long
memcalcbkt(unsigned long size)
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

/* use compiler optimizations to evaluate bucket for constant allocation size */
#if (MEM_CONST_SIZE_TRICK)
#if (PTRBITS == 32)
#define memfastbkt(sz)                                                  \
    ((!__builtin_constant_p(sz)                                         \
      ? memcalcbkt(sz)                                                  \
      : (((sz) <= MAGMIN)                                               \
         ? MAGMINLOG2                                                   \
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
#define memfastbkt(sz)                                                  \
    ((!__builtin_constant_p(sz)                                         \
      ? memcalcbkt(sz)                                                  \
      : (((sz) <= MAGMIN)                                               \
         ? MAGMINLOG2                                                   \
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

#endif /* __MEM_MEM_H__ */

