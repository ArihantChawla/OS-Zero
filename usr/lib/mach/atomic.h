#ifndef __MACH_ATOMIC_H__
#define __MACH_ATOMIC_H__

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

#endif /* __MACH_ATOMIC_H__ */

