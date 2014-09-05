#ifndef __STDDEF_H__
#define __STDDEF_H__

#include <stdint.h>

#define NULL           ((void *)0L)

typedef long           ptrdiff_t;
typedef unsigned long  size_t;
/* full ISO10646 character set aka Unicode */
typedef int32_t        wchar_t;
typedef int32_t        wint_t;

#define offsetof(t, m) ((size_t)(&((t *)0)->m))

#endif /* __STDDEF_H__ */

