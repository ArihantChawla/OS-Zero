#ifndef __ZERO_IA32_PROF_H__
#define __ZERO_IA32_PROF_H__

#include <stdint.h>

union m_tickcnt {
    uint64_t u64val;
    uint32_t u32vals[2];
};

#define PROFTICK(id)                                                    \
    union m_tickcnt __tc##id[2]
#define profinittick(id)                                                \
    memset(&__tc##id, 0, sizeof(__tc##id))
#define profstarttick(id)                                               \
    _gettickcnt(&__tc##id[0])
#define profstoptick(id)                                                \
    _gettickcnt(&__tc##id[1])
#define proftickdiff(id)                                                \
    (__tc##id[1].u64val - __tc##id[0].u64val)
#define Z_PROF_TICK_FORMAT "%llu"

#define _gettickcnt(ptr)                                         \
    __asm__("rdtsc; movl %%eax, %0; movl %%edx, %1"              \
            : "=m" ((ptr)->u32vals[0]), "=m" ((ptr)->u32vals[1]) \
            :                                                    \
            : "eax", "edx");

#if 0
__inline__ uint64_t
_rdpmc(union _tickcnt *cnt, int id)
{
    __asm__("movl %0, %%ecx ; rdpmc ; mov %%eax, %1 ; mov %%edx, %2"
            : "=rm" (cnt->u32vals[0]), "=rm" (cnt->u32vals[1])
            : "rm" (id)
            : "eax", "edx");

    return (cnt->u64val);
}
#endif

#endif /* __ZERO_IA32_PROF_H__ */

