#ifndef __ZERO_ASM_H__
#define __ZERO_ASM_H__

#include <stddef.h>

/*
 * TODO
 * ----
 * - m_memddbar()   - data dependency barrier
 * - m_memrdbar()   - read barrier
 * - m_memwrbar()   - write barrier
 * - m_cmpswapbit() - bit-level atomic compare and swap
 */

/*
 * machine-specific asm.h headers should declare the following:
 * m_membar()   	- memory barrier for serialised access
 * m_waitint()  	- pause until interrupt received
 * m_cmpswap()  	- atomic compare and swap operation
 * m_fetchadd()   	- atomic fetch and add
 * m_getretadr(pp)      - get return address of current function to *(pp)
 * m_setretadr(pp)      - store return address of current function to *(pp)
 */
#if defined(_MSC_VER)
#if defined(_WIN64)
#include <zero/msc/win64.h>
#elif defined(_WIN32)
#include <zero/msc/win32.h>
#endif
#elif defined(__x86_64__) || defined(__amd64__)
#include <zero/x86-64/asm.h>
#elif defined(__i386__)
#include <zero/ia32/asm.h>
#elif defined(__arm__)
#include <zero/arm/asm.h>
#elif defined(__ppc__)
#include <zero/ppc/asm.h>
#endif

/* for systems without GNU C, we define these in e.g. <zero/ia32/asm.h> */
#if defined(__GNUC__)
/* stack frames */
#define m_setretadr(p)                                                  \
    ((p) = (void *)__builtin_frob_return_address(__builtin_return_address(0)))
#define m_getretadr(p)                                                  \
    (__builtin_extract_return_addr(__builtin_return_address(0)))
#define m_getfrmadr(p)                                                  \
    (__builtin_frame_address(0))
/* atomic operations - FIXME: are the barriers used correctly? */
#define m_atomread(v)     ((*(volatile typeof(v) *)&(v)))
#define m_atomwrite(a, v) ((*(volatile typeof(v) *)(a)) = (v))
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

#endif /* __ZERO_ASM_H__ */

