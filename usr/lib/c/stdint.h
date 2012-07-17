#ifndef __STDINT_H__
#define __STDINT_H__

#include <zero/param.h>

#define INT32_C(i)  i
#define UINT32_C(i) i##U
#if (LONGSIZE == 4)
#define INT64_C(i)  i##LL
#define UINT64_C(i) i##ULL
#elif (LONGSIZE == 8)
#define INT64_C(i)  i##L
#define UINT64_C(i) i##UL
#endif

typedef char               int8_t;
typedef unsigned char      uint8_t;
typedef short              int16_t;
typedef unsigned short     uint16_t;
typedef int                int32_t;
typedef unsigned           int uint32_t;
#if (LONGSIZE == 4)
typedef long long          int64_t;
typedef unsigned long long uint64_t;
#elif (LONGSIZE == 8)
typedef long               int64_t;
typedef unsigned long      uint64_t;
#endif
typedef char               int_fast8_t;
typedef unsigned char      uint_fast8_t;
typedef short              int_fast16_t;
typedef unsigned short     uint_fast16_t;
typedef int                int_fast32_t;
typedef unsigned int       uint_fast32_t;
#if (LONGSIZE == 4)
typedef int32_t            intptr_t;
typedef uint32_t           uintptr_t;
#elif (LONGSIZE == 8)
typedef int64_t            intptr_t;
typedef uint64_t           uintptr_t;
#endif
#if (LONGSIZE == 4)
typedef long long          int_fast64_t;
typedef unsigned long long uint_fast64_t;
#elif (LONGSIZE == 8)
typedef long               int_fast64_t;
typedef unsigned long      uint_fast64_t;
#endif

#define __int8_t_defined
#define __uint8_t_defined
#define __int16_t_defined
#define __uint16_t_defined
#define __int32_t_defined
#define __uint32_t_defined
#define __int64_t_defined
#define __uint64_t_defined
#define __intptr_t_defined
#define __uintptr_t_defined

#endif /* __STDINT_H__ */

