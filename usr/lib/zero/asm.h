#ifndef __ZERO_ASM_H__
#define __ZERO_ASM_H__

/*
 * machine-specific asm.h headers should declare the following:
 * m_membar()   	- memory barrier for serialised access
 * m_waitint()  	- pause until interrupt received
 * m_cmpswap()  	- atomic compare and swap operation
 * m_fetadd()   	- atomic fetch and add
 * m_getretadr(pp)      - get return address of current function to *(pp)
 * m_setretadr(pp)      - store return address of current function to *(pp)
 */
#if defined(__x86_64__) || defined(__amd64__)
#include <zero/x86-64/asm.h>
#elif defined(__i386__)
#include <zero/ia32/asm.h>
#elif defined(__arm__)
#include <zero/arm/asm.h>
#elif defined(__ppc__)
#include <zero/ppc/asm.h>
#endif

#endif /* __ZERO_ASM_H__ */

