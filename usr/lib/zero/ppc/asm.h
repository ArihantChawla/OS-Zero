#ifndef __ZERO_PPC_ASM__
#define __ZERO_PPC_ASM__

#define m_membar() __asm__ __volatile__ ("lwsync\n" : : : "memory")

#endif /* __ZERO_PPC_ASM__ */

