#ifndef __STDDEF_H__
#define __STDDEF_H__

#include <features.h>
#include <stdint.h>
#include <share/stddef.h>

#if (_ISOC11_SOURCE)
typedef long long      max_align_t;
#endif /* _ISOC11_SOURCE */

#define offsetof(t, m) ((size_t)(&((t *)0)->m))

#endif /* __STDDEF_H__ */

