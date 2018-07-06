#ifndef __SHARE_NULL_H__
#define __SHARE_NULL_H__

#if defined(_ZERO_SOURCE)

#include <mach/param.h>
#include <stdint.h>

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

#endif /* _ZERO_SOURCE */

#endif /* __SHARE_NULL_H__ */

