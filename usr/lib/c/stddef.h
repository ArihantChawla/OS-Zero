#ifndef __STDDEF_H__
#define __STDDEF_H__

#include <features.h>
#include <stdint.h>
#include <share/size.h>

#if !defined(NULL)
#define NULL               ((void *)0L)
#endif

#if defined(_MSC_VER) && defined(_WIN64)
typedef long long          ptrdiff_t;
#else
typedef long               ptrdiff_t;
#endif
#if !defined(__size_t_defined)
#include <share/size.h>
#endif
#if !defined(__wchar_t_defined)
#include <share/wchar.h>
#endif

#if (_ISOC11_SOURCE)
typedef long long      max_align_t;
#endif /* _ISOC11_SOURCE */

#define offsetof(t, m) ((size_t)(&((t *)0)->m))

#endif /* __STDDEF_H__ */

