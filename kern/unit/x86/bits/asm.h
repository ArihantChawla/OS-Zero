#ifndef __KERN_UNIT_X86_ASM_H__
#define __KERN_UNIT_X86_ASM_H__

#if defined(__x86_64__) || defined(__amd64__)
#include <kern/unit/x86-64/bits/asm.h>
#elif (defined(__i386__) || defined(__i486__)                           \
       || defined(__i586__) || defined(__i686__))
#include <kern/unit/ia32/bits/asm.h>
#endif

#endif /* __KERN_UNIT_X86_ASM_H__ */

