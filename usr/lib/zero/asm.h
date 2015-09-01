#ifndef __ZERO_ASM_H__
#define __ZERO_ASM_H__

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
 * m_fetadd()   	- atomic fetch and add
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

#endif /* __ZERO_ASM_H__ */

