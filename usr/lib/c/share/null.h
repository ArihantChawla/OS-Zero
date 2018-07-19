#ifndef __SHARE_NULL_H__
#define __SHARE_NULL_H__

#include <stdint.h>
#include <mach/param.h>

#if !defined(NULL)
#if (PTRSIZE == 8)
#define NULL ((void *)UINT64_C(0))
#elif (PTRSIZE == 4)
#define NULL ((void *)UINT32_C(0))
#elif defined(_MSC_VER) && defined(_WIN64)
#define NULL ((void *)0ULL)
#else
#define NULL ((void *)0UL))
#endif
#endif

#endif /* __SHARE_NULL_H__ */

