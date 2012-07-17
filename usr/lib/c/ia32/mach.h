#ifndef __MACH_IA32_MACH_H__
#define __MACH_IA32_MACH_H__

#define getretadr(r)                                                    \
    __asm__ __volatile__ ("movl 4(%%ebp), %0\n" : "=r" (r))
/* C call frame */
struct m_cframe {
    /* automatic variables go here */
    int32_t fp;         // caller frame pointer
    int32_t pc;         // return address
    /* call parameters on stack go here in 'reverse order' */
};

/* far pointer structure */
struct m_farptr {
    uint16_t lim;
    uint32_t adr;
} PACK();

/* return stack for IRET */
struct m_trapframe {
    int32_t eip;	// old instruction pointer
    int16_t cs;		// code segment selector
    int16_t pad1;	// pad to 32-bit boundary
    int32_t eflags;	// machine status word
    /* present in case of privilege transition */
    int32_t uesp;	// user stack pointer
    int16_t uss;	// user stack segment selector
    int16_t pad2;	// pad to 32-bit boundary
};

struct m_pusha {
    int32_t edi;
    int32_t esi;
    int32_t esp;
    int32_t ebp;
    int32_t ebx;
    int32_t edx;
    int32_t ecx;
    int32_t eax;
};

struct m_tss {
    uint16_t link, __linkhi;
    uint32_t esp0;
    uint16_t ss0, __ss0hi;
    uint32_t esp1;
    uint16_t ss1, __ss1hi;
    uint32_t esp2;
    uint16_t ss2, __ss2hi;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint16_t es, __eshi;
    uint16_t cs, __cshi;
    uint16_t ss, __sshi;
    uint16_t ds, __dshi;
    uint16_t fs, __fshi;
    uint16_t gs, __gshi;
    uint16_t ldt, __ldthi;
    uint16_t trace;
    uint16_t iomapofs;
//    uint8_t  iomap[8192] ALIGNED(CLSIZE);
} PACK();

#endif /* __MACH_IA32_MACH_H__ */

