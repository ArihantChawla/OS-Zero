#ifndef __KERN_UNIT_X86_ASM_H__
#define __KERN_UNIT_X86_ASM_H__

#define k_disabintr() __asm__ __volatile__ ("cli\n" : : : "memory")
#define k_enabintr()  __asm__ __volatile__ ("sti\n" : : : "memory")
#define k_waitint()   __asm__ __volatile__ ("sti\nhlt\n" : : : "memory")

#endif /* __KERN_UNIT_X86_ASM_H__ */

