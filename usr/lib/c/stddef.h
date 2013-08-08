#ifndef __STDDEF_H__
#define __STDDEF_H__

#include <stdint.h>

#define NULL ((void *)0x00000000L)

typedef long          ptrdiff_t;
typedef unsigned long size_t;
typedef int32_t       wchar_t;

#define offsetof(t, m) ((size_t)(&((t *)0)->m))

#endif /* __STDDEF_H__ */

