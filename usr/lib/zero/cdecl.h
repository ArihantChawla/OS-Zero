#ifndef __ZERO_CDECL_H__
#define __ZERO_CDECL_H__

/* size for 'empty' array (placeholder) */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#   define EMPTY
#else
#   define EMPTY   0
#endif

/* __func__ for non-C99 compilers; MSVC should do this, probably =) */
#if !defined(__STDC_VERSION__) || (__STDC_VERSION__ < 199901L) \
    && (defined(__GNUC__) || defined(_MSC_VER) || defined(_WIN32))
#define __func__ __FUNCTION__
#endif

#if defined(__GNUC__)

/* align variables, aggregates, and tables to boundary of a */
#define ALIGNED(a)  __attribute__ ((__aligned__(a)))
/* ALWAYS inline the function */
#define INLINE      __inline__ __attribute__ ((always_inline))
#define WEAK        __attribute__ ((weak))
/*
 * AMD64 passes first six arguments in rdi, rsi, rdx, rcx, r8, and r9; the rest
 * are pushed to stack in reverse order
 *
 * IA-32 can pass up to 3 register arguments in eax, edx, and ecx
 */
#define REGPARM(n)  __attribute__ ((regparm(n)))
#if defined(__x86_64__) || defined(__amd64__) || defined(__arm__)
#define FASTCALL    /* at least 3 arguments passed in registers by default */
#elif defined(__i386__) || defined(__i486__) || defined(__i586__) \
     || defined(__i686__)
#define FASTCALL    REGPARM(3)
#endif

/* pass all arguments on stack for assembly-linkage */
#define ASMLINK     __attribute__ ((__regparm__(0)))

/* declare function with no return (e.g., longjmp()) */
#define NORET       __attribute__ ((__noreturn__))
#define PURE        __attribute__ ((__pure__))

#define isimmed(x)  (__builtin_constant_p(x))
#define likely(x)   (__builtin_expect(!!(x), 1))
#define unlikely(x) (__builtin_expect(!!(x), 0))
#define isconst(x)  (__builtin_constant_p(x))

#elif defined(_MSC_VER) /* !defined(__GNUC__) */
    
/* Microsoft */
#define ALIGNED(a)   __declspec(align((a)))
#define __inline__   inline
#define INLINE       __forceinline

#define ASMLINK      __cdecl

#endif /* __GNUC__ */

#endif /* __ZERO_CDECL_H__ */

