#ifndef __SHARE_PTRDIFF_T_H__
#define __SHARE_PTRDIFF_T_H__

#if defined(_ZERO_SOURCE)
#include <zero/param.h>
#include <stdint.h>
#include <share/null.h>
#if (PTRSIZE == 8)
typedef int64_t            ptrdiff_t;
#elif (PTRSIZE == 4)
typedef int32_t            ptrdiff_t;
#endif
#elif defined(_MSC_VER) && defined(_WIN64)
typedef long long          ptrdiff_t;
#else
typedef long               ptrdiff_t;
#endif
#define __ptrdiff_t_defined 1

#endif /* __SHARE_PTRDIFF_T_H__ */

