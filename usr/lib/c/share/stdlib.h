#ifndef __SHARE_STDLIB_H__
#define __SHARE_STDLIB_H__

#if defined(_ZERO_SOURCE)
#include <zero/param.h>
#include <stdint.h>
#if (PTRSIZE == 8)
typedef int64_t            ptrdiff_t;
#elif (PTRSIZE == 4)
#endif
#elif defined(_MSC_VER) && defined(_WIN64)
typedef long long          ptrdiff_t;
#else
typedef long               ptrdiff_t;
#endif
#if !defined(NULL)
#include <share/null.h>
#endif
#define __ptrdiff_t_defined 1
#if !defined(__size_t_defined)
#include <share/size.h>
#endif
#if !defined(__wchar_t_defined)
#include <share/wchar.h>
#endif

#endif /* __SHARE_STDLIB_H__ */

