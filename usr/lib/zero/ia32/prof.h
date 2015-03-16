#ifndef __ZERO_IA32_PROF_H__
#define __ZERO_IA32_PROF_H__

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * THANKS
 * ------
 * Thomas 'tommycannady' Cannady for help with testing the MSVC versions of
 * the macros... :)
 */ 

#include <stdint.h>

union _tickval {
    uint64_t u64;
    uint32_t u32v[2];
};

#if defined(_MSC_VER)
#define _rdtsc(tp)                                                      \
    do {                                                                \
        unsigned __int64 _cnt;                                          \
                                                                        \
        _cnt = __rdtsc();                                               \
        tp->u64 = _cnt;                                                 \
    } while (0)
#define _rdpmc(tp)                                                      \
    do {                                                                \
        unsigned __int64 _cnt;                                          \
                                                                        \
        _cnt = __rdpmc();                                               \
        tp->u64 = _cnt;                                                 \
    } while (0)
#else /* !defined(_MSC_VER) */
/* read TSC (time stamp counter) */
#define _rdtsc(tp)                                                      \
    __asm__("rdtsc\n"                                                   \
            "movl %%eax, %0\n"                                          \
            "movl %%edx, %1\n"                                          \
            : "=m" ((tp)->u32v[0]), "=m" ((tp)->u32v[1])                \
            :                                                           \
            : "eax", "edx");

/* read performance monitor counter */
static __inline__ uint64_t
_rdpmc(union _tickval *tp, int id)
{
    __asm__("movl %0, %%ecx\n"
            "rdpmc\n"
            "mov %%eax, %1\n"
            "mov %%edx, %2"
            : "=rm" (tp->u32v[0]), "=rm" (tp->u32v[1])
            : "rm" (id)
            : "eax", "edx");

    return (tp->u64);
}
#endif

#define PROFDECLTICK(id)                                                \
    union _tickval __tv##id[2]
#define profinittick(id)                                                \
    (__tv##id[0].u64 = __tv##id[1].u64 = UINT64_C(0))
//    memset(&__tv##id, 0, sizeof(__tv##id))
#define profstarttick(id)                                               \
    _rdtsc(&__tv##id[0])
#define profstoptick(id)                                                \
    _rdtsc(&__tv##id[1])
#define proftickdiff(id)                                                \
    (__tv##id[1].u64 - __tv##id[0].u64)

#if defined(__cplusplus)
}
#endif

#endif /* __ZERO_IA32_PROF_H__ */

