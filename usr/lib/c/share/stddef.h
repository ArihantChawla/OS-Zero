#ifndef __SHARE_STDDEF_H__
#define __SHARE_STDDEF_H__

#if defined(_ZERO_SOURCE)
#if !defined(NULL)
#include <share/null.h>
#endif
#if !defined(__ptrdiff_t_defined)
#include <share/ptrdiff_t.h>
#endif
#if !defined(__size_t_defined)
#include <share/size.h>
#endif
#if !defined(__wchar_t_defined)
#include <share/wchar.h>
#endif
#endif /* _ZERO_SOURCE */

#endif /* __SHARE_STDDEF_H__ */

