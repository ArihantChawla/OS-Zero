#ifndef __KERN_ASM_H__
#define __KERN_ASM_H__

#if defined(__x86_64__) || defined(__amd64__)
#include <kern/unit/x86-64/asm.h>
#endif
#if (defined(__i386__) || defined(__i486__)                             \
     || defined(__i586__) || defined(__i686__))
#include <kern/unit/ia32/asm.h>
#endif

#endif /* __KERN_ASM_H__ */

