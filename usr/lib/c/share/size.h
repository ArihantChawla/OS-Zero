#ifndef __SHARE_SIZE_H__
#define __SHARE_SIZE_H__

#include <mach/param.h>

#if (PTRSIZE == 8)
typedef unsigned long long size_t;
#elif (PTRSIZE == 4)
typedef unsigned long size_t;
#endif

#endif /* __SHARE_SIZE_H__ */

