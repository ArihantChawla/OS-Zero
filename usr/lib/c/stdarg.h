#ifndef __STDARG_H__
#define __STDARG_H__

#if defined(__GNUC__)
#if !defined(__GNUC_VA_LIST)
typedef __builtin_va_list __gnuc_va_list;
#endif
typedef __gnuc_va_list    va_list;
#endif
#define va_start(v, l)    __builtin_va_start(v, l)
#define va_end(v)         __builtin_va_end(v)
#define va_arg(v, l)      __builtin_va_arg(v, l)
#if defined(__STDC_VERSION) && (__STDC_VERSION__ >= 199901L)
/* C99 addition */
#define va_copy(d, s)     __builtin_va_copy(d, s)
#elif defined(_MSC_VER)
#include <va_list.h>
/* courtesy of Microsoft */
#define _INTSIZEOF(n)	  ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#define va_start(ap,v)	  (ap = (va_list)&v + _INTSIZEOF(v))
#define va_arg(ap,t)	  (*(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#define va_end(ap)	  (ap = (va_list)0)
#endif

#if defined(__VARARGS_H__)
/* remove <varargs.h> functionality; thanks to Microsoft for this idea */
#undef va_alist
#undef va_dcl
#undef va_start
#undef va_end
#undef va_arg
#endif

#endif /* __STDARG_H__ */

