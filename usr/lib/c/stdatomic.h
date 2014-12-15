#ifndef __STDATOMIC_H__
#define __STDATOMIC_H__

/* https://gcc.gnu.org/onlinedocs/gcc-4.5.0/gcc/Atomic-Builtins.html */
/* https://apr.apache.org/docs/apr/1.5/group__apr__atomic.html */

#undef __STDC_NO_ATOMICS__

/* lock-free type */
typedef long                 atomic_flag;

/* atomic types */
typedef _Bool                atomic_bool;
typedef char                 atomic_char;
typedef signed char          atomic_schar;
typedef unsigned char        atomic_uchar;
typedef short                atomic_short;
typedef unsigned short       atomic_ushort;
typedef int                  atomic_int;
typedef unsigned int         atomic_uint;
typedef long                 atomic_long;
typedef unsigned long        atomic_ulong;
typedef long long            atomic_llong;
typedef unsigned long long   atomic_ullong;
typedef float                atomic_float;
typedef double               atomic_double;
typedef long double          atomic_ldouble;
#if !defined(__STDC_NO_COMPLEX__)
typedef float _Complex       atomic_cfloat;
typedef double _Complex      atomic_cdouble;
typedef long double _Complex atomic_cldouble;
#endif

#endif /* __STDATOMIC_H__ */

