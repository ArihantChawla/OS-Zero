#ifndef __MACH_ATOMIC_H__
#define __MACH_ATOMIC_H__

#if 0
#if ((((defined(__x86_64__) || defined(__amd64__))                      \
       && (defined(__GNUC__) && __GNUC__ >= 4))                         \
      || defined(_WIN64))                                               \
     && !defined(__ILP32__))
#include <xmmintrin.h>
typedef __m128             MACHDWORD;
#elif (defined(__x86_64__) || defined(__amd64__))
typedef __uint128_t        MACHDWORD;
#elif defined(__aarch64__)
typedef unsigned __int128  MACHDWORD;
#elif (defined(_WIN32) && !defined(__GNUC__))
typedef unsigned __int64   MACHDWORD;
#else
typedef unsigned long long MACHDWORD;
#endif
#endif
#include <mach/types.h>

/* atomic types */
typedef m_reg_t    m_atomic_t;
typedef int8_t     m_atomic8_t;
typedef uint8_t    m_atomicu8_t;
typedef int16_t    m_atomic16_t;
typedef uint16_t   m_atomicu16_t;
typedef int32_t    m_atomic32_t;
typedef uint32_t   m_atomicu32_t;
typedef int64_t    m_atomic64_t;
typedef uint64_t   m_atomicu64_t;
typedef void      *m_atomicptr_t;
typedef int8_t    *m_atomicptr8_t;
typedef m_adr_t    m_atomicadr_t;

#endif /* __MACH_ATOMIC_H__ */

