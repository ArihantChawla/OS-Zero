#ifndef __ZERO_X86_PROF_H__
#define __ZERO_X86_PROF_H__

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * THANKS
 * ------
 * Thomas 'tommycannady' Cannady for help with testing and fixing the MSVC
 * versions of the macros... :)
 */ 

#include <stdint.h>
#if defined(_MSC_VER)
#undef  __inline__
#define __inline__ inline
#endif

#if 0
union _tickval {
    uint64_t u64;
    uint32_t u32v[2];
};
#endif
struct _tickval {
    union {
        uint64_t u64;
        uint32_t u32v[2];
    } u;
};
    
#if defined(_MSC_VER)
#define _rdtsc(tp)                                                      \
    do {                                                                \
        uint64_t _cnt;                                                  \
                                                                        \
        _cnt = rdtsc();                                                 \
        tp->u.u64 = _cnt;                                               \
    } while (0)
#define _rdpmc(tp)                                                      \
    do {                                                                \
        uint64_t _cnt;                                                  \
                                                                        \
        _cnt = rdpmc();                                                 \
        tp->u.u64 = _cnt;                                               \
    } while (0)
#else /* !defined(_MSC_VER) */
static __inline__ uint64_t
_rdtsc(struct _tickval *tp)
{
    unsigned long      lo;
    unsigned long      hi;
    unsigned long long ret;
    
    m_membar();
    __asm__("rdtsc\n"
            : "=a" (lo), "=d" (hi)
            :
            : "eax", "edx");
    ret = hi;
    ret <<= 32;
    ret |= lo;
    if (tp) {
        tp->u.u64 = ret;
    }

    return ret;
}

#if 0
/* read TSC (time stamp counter) */
#define _rdtsc(tp)  
    __asm__("rdtsc\n"                                                   \
            "movl %%eax, %0\n"                                          \
            "movl %%edx, %1\n"                                          \
            : "=m" ((tp)->u.u32v[0]), "=m" ((tp)->u.u32v[1])            \
            :                                                           \
            : "eax", "edx")
#endif

/* read performance monitor counter */
static __inline__ uint64_t
_rdpmc(struct _tickval *tp, int id)
{
    __asm__("movl %0, %%ecx\n"
            "rdpmc\n"
            "mov %%eax, %1\n"
            "mov %%edx, %2"
            : "=rm" (tp->u.u32v[0]), "=rm" (tp->u.u32v[1])
            : "rm" (id)
            : "eax", "edx");

    return (tp->u.u64);
}
#endif

#define PROFDECLTICK(id)                                                \
    struct _tickval __tv##id[2]
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

#endif /* __ZERO_X86_PROF_H__ */

