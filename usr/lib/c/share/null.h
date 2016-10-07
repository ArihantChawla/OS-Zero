#ifndef __SHARE_NULL_H__
#define __SHARE_NULL_H__

#if defined(_ZERO_SOURCE)
#include <zero/param.h>
#include <stdint.h>
#if (PTRSIZE == 8)
#if !defined(NULL)
#define NULL ((void *)UINT64_C(0))
#endif
#elif (PTRSIZE == 4)
#if !defined(NULL)
#define NULL ((void *)UINT32_C(0))
#endif
#elif defined(_MSC_VER) && defined(_WIN64)
#if !defined(NULL)
#define NULL ((void *)0ULL)
#endif
#else
#if !defined(NULL)
#define NULL ((void *)0UL))
#endif
#endif

#endif /* __SHARE_NULL_H__ */

