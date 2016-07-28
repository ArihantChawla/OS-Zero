#ifndef __ZERO_ATOMIC_H__
#define __ZERO_ATOMIC_H__

#include <zero/atomic.h>

#if ((((defined(__x86_64__) || defined(__amd64__))                      \
       && (defined(__GNUC__) && __GNUC__ >= 4))                         \
      || defined(_WIN64))                                               \
     && !defined(__ILP32__))
#include <xmmintrin.h>
typedef __m128             ZERODWORD;
#elif (defined(__x86_64__) || defined(__amd64__))
typedef __uint128_t        ZERODWORD;
#elif defined(__aarch64__)
typedef unsigned __int128  ZERODWORD;
#elif (defined(_WIN32) && !defined(__GNUC__))
typedef unsigned __int64   ZERODWORD;
#else
typedef unsigned long long ZERODWORD;
#endif

#endif /* __ZERO_ATOMIC_H__ */

