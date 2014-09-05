#ifndef __STDDEF_H__
#define __STDDEF_H__

#include <features.h>
#include <stdint.h>

#define NULL           ((void *)0L)

typedef long           ptrdiff_t;
typedef unsigned long  size_t;
/* full ISO10646 character set aka Unicode */
#if (_ZERO_SOURCE) || (__STDC_ISO_10646__)
typedef int32_t        wchar_t;
typedef int32_t        wint_t;
#endif

#define offsetof(t, m) ((size_t)(&((t *)0)->m))

#endif /* __STDDEF_H__ */

