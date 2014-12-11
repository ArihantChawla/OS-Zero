#ifndef __ZERO_CDECL_H__
#define __ZERO_CDECL_H__

/* size for 'empty' array (placeholder) */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#   define EMPTY
#else
#   define EMPTY   0
#endif

/* __FUNCTION__ for non-C99 compilers */
#if !defined(__STDC_VERSION__) || (__STDC_VERSION__ < 199901L)
#if defined(__GNUC__)
#define __func__   __FUNCTION__
#else
#endif
#endif

#if defined(__GNUC__)

/* align variables, aggregates, and tables to boundary of a */
#define ALIGNED(a)  __attribute__ ((__aligned__(a)))
/* pack aggregate fields and table items */
#define PACK()      __attribute__ ((__packed__))
/*
 * AMD64 passes first six arguments in rdi, rsi, rdx, rcx, r8, and r9; the rest
 * are pushed to stack in reverse order
 *
 * IA-32 can pass up to 3 register arguments in eax, edx, and ecx
 */
#define REGPARM(n)  __attribute__ ((regparm(n)))
#if defined(__x86_64__) || defined(__amd64__) || defined(__arm__)
#define FASTCALL    /* at least 3 arguments passed in registers by default */
#elif defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__)
#define FASTCALL    REGPARM(3)
#endif

/* pass all arguments on stack for assembly-linkage */
#define ASMLINK     __attribute__ ((__regparm__(0)))

/* declare function with no return (e.g., longjmp()) */
#define NORET       __attribute__ ((__noreturn__))

#define likely(x)   __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)
#define isconst(x)  __builtin_constant_p(x)

#else /* !defined(__GNUC__) */

/* Microsoft */
#if defined(_MSC_VER)
#define ALIGNED(a)  __declspec(align((a)))
#endif

#endif /* __GNUC__ */

#endif /* __ZERO_CDECL_H__ */

