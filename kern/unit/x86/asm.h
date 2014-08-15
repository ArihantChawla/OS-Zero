#ifndef __KERN_UNIT_X86_ASM_H__
#define __KERN_UNIT_X86_ASM_H__

#define k_disabintr() __asm__ __volatile__ ("cli\n" : : : "memory")
#define k_enabintr()  __asm__ __volatile__ ("sti\n" : : : "memory")
#define k_waitint()   __asm__ __volatile__ ("sti\nhlt\n" : : : "memory")

static inline uint64_t
k_readmsr(long adr)
{
    uint32_t eax;
    uint32_t edx;
    uint64_t retval;

    __asm__ __volatile__ ("rdmsr"
                          : "=a" (eax), "=d" (edx)
                          : "r" (adr));
    retval = ((uint64_t)edx << 32) | eax;

    return retval;
}

static inline void
k_writemsr(uint32_t adr, uint64_t val)
{
    __asm__ __volatile__ ("wrmsr" : : "c" (adr), "A" (val));
}

#endif /* __KERN_UNIT_X86_ASM_H__ */

