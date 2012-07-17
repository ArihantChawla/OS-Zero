#ifndef __ZERO_ARM_ASM_H__

#define __ZERO_ARM_ASM_H__

/* API declarations */
#define m_membar()  __asm__ __volatile__ ("" : : : "memory")
#if defined(__ARM_ARCH_4__) || defined (__ARM_ARCH_4T__)
#define m_waitint() __asm__ __volatile__ ("nop\n")
#elif defined(__ARM_ARCH_7__)
#define m_waitint() __asm__ __volatile__ ("wfi\n")
#else
#define m_waitint() __asm__ __volatile__ ("mov r0, #0\n"                \
                                          "mcr p15, #0, r0, c7, c0, #4\n")
#endif

static __inline__ long
m_cmpswap(long *p, long want, long val)
{
    long res;
    long tmp;

    __asm__ __volatile__ ("0:\n"
                          "ldr %1, [%2]\n"
                          "mov %0, #0\n"
                          "cmp %1, %4\n"
                          "bne 1f\n"
                          "swp %0, %0, [%2]\n"
                          "cmp %1, %0\n"
                          "swpne %1, %0, [%2]\n"
                          "bne 0b\n"
                          "mov %0, #1\n"
                          "1:\n"
                          : "=&r" (res), "=&r" (tmp)
                          : "r" (p), "r" (val), "r" (want)
                          : "cc", "memory");

    return !res;
}

#endif /* __ZERO_ARM_ASM_H__ */

