#ifndef __MACH_ASM_H__
#define __MACH_ASM_H__

#include <stddef.h>
#include <stdint.h>

/*
 * TODO
 * ----
 * - m_cmpswapbit() - bit-level atomic compare and swap
 */

#if defined(_WIN64)
#include <zero/msc/win64.h>
#elif defined(_WIN32)
#include <zero/msc/win32.h>
#elif defined(__x86_64__) || defined(__amd64__)
#include <mach/x86/asm.h>
#include <mach/x86-64/asm.h>
#elif (defined(__i386__) || defined(__i486__)                           \
       || defined(__i586__) || defined(__i686__))
#include <mach/x86/asm.h>
#include <mach/ia32/asm.h>
#elif defined(__arm__)
#include <mach/arm/asm.h>
#elif defined(__ppc__)
#include <mach/ppc/asm.h>
#endif

/* volatile types */
typedef m_reg_t   m_atomic_t;
typedef int8_t    m_atomic8_t;
typedef int16_t   m_atomic16_t;
typedef int32_t   m_atomic32_t;
typedef int64_t   m_atomic64_t;
typedef void     *m_atomicptr_t;
typedef int8_t   *m_atomicptr8_t;
typedef intptr_t  m_atomicadr_t;

/* for systems without GNU C, we define these in e.g. <mach/ia32/asm.h> */
#if defined(__GNUC__)
/* stack frames */
#define m_setretadr(p)                                                  \
    ((p) = (void *)__builtin_frob_return_address(__builtin_return_address(0)))
#define m_getretadr(p)                                                  \
    (__builtin_extract_return_addr(__builtin_return_address(0)))
#define m_getfrmadr(p)                                                  \
    (__builtin_frame_address(0))
/* atomic operations - FIXME: are the barriers used correctly? */
#define m_atomread(a)      (*((typeof(a))(a)))
#define m_atomwrite(a, v)  (*((typeof(v) *)(a)) = (v))
#define m_syncread(a, res)                                              \
    do {                                                                \
        m_memwrbar();                                                   \
        (res) = m_atomread(a);                                          \
    } while (0)
#define m_syncwrite(a, val)                                             \
    do {                                                                \
        m_memwrbar();                                                   \
        m_atomwrite(a, val);                                            \
    } while (0)
#endif

#endif /* __MACH_ASM_H__ */

