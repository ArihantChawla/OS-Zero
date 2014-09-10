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
#define va_copy(d, s)     __builtin_va_copy(d, s)
#endif

#endif /* __STDARG_H__ */

