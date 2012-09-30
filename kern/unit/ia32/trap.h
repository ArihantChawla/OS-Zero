#ifndef __KERN_UNIT_IA32_TRAP_H__
#define __KERN_UNIT_IA32_TRAP_H__

#include <kern/conf.h>

#if !defined(__ASSEMBLY__)
#include <stdint.h>
#include <zero/asm.h>
#endif
#include <kern/unit/ia32/boot.h>

#define NIDT 256

#if !defined(__ASSEMBLY__)

/* func is routine; ring is TRAPUSER or TRAPSYS */
#define trapsetgate(p, func, ring)                                      \
    do {                                                                \
        uint64_t _func = (uint32_t)(func);                              \
        *(p) = _mkidtdesc(TEXTSEL, _func, ring);                        \
    } while (0)
#define _mkidtdesc(sel, f64, ring)                                      \
    (((f64) & 0x0000ffff)                                               \
     | ((sel) << 16)                                                    \
     | (ring)                                                           \
     | (((f64) & 0xffff0000) << 32)                                     \
     | TRAPDEFBITS)

static __inline__ void
m_lidt(struct m_farptr *fp)
{
    __asm__ __volatile__ ("lidt (%0)\n" : : "r" (fp));
}

#define TRAPGATE    UINT64_C(0x00000e0000000000)
#define TRAPUSER    UINT64_C(0x0000600000000000)
#define TRAPSYS     UINT64_C(0x0000000000000000)
#define TRAPPRES    UINT64_C(0x0000800000000000)
#define TRAPDEFBITS (TRAPGATE | TRAPPRES)

#define TRAPSYSCALL 0x80        // traditional INT 0x80 for system calls

//#define traphaserr(i, f) bitset(f, i)
#define trapisflt(i, f)  bitset(trapfltmask, i)
#define trapistrp(i, f)  bitset(traptrpmask, i)
//#define trapisirq(i)     ((i) & TRAPIRQMASK)
//#define trapirqid(i)     ((i) | TRAPIRQBIT)
#define trapirqid(i)     (IRQBASE + (i))
#if 0
#define trapisirq(i)     ((i) & TRAPIRQBIT)
#define trapirqnum(i)    ((i) & 0x0f)
#endif

#define TRAPERRBITS                                                     \
    ((1U << TRAPDF)                                                     \
     | (1U << TRAPTS)                                                   \
     | (1U << TRAPNP)                                                   \
     | (1U << TRAPSS)                                                   \
     | (1U << TRAPGP)                                                   \
     | (1U << TRAPPF)                                                   \
     | (1U << TRAPMF)                                                   \
     | (1U << TRAPAC))

#define TRAPFLTBITS                                                     \
    ((1U << TRAPDE)                                                     \
     | (1U << TRAPBR)                                                   \
     | (1U << TRAPUD)                                                   \
     | (1U << TRAPNM)                                                   \
     | (1U << TRAPDF)                                                   \
     | (1U << TRAPTS)                                                   \
     | (1U << TRAPGP)                                                   \
     | (1U << TRAPPF)                                                   \
     | (1U << TRAPMF)                                                   \
     | (1U << TRAPMC)                                                   \
     | (1U << TRAPXF))
#define TRAPBITS ((1U << TRAPBP) | (1U << TRAPOF))

#endif /* __ASSEMBLY__ */

/*
 * fault: %eip points to fault instruction
 * trap:  %eip points to next instruction
 */
/* cpu traps */
#define TRAPDE   0x00	// divide error; fault
#define TRAPDB   0x01	// reserved; fault/trap
#define TRAPNMI  0x02	// non-maskable interrupt; interrupt
#define TRAPBP   0x03	// breakpoint; trap
#define TRAPOF   0x04	// overflow; trap
#define TRAPBR   0x05	// bound range exceeded; fault
#define TRAPUD   0x06	// invalid opcode; fault
#define TRAPNM   0x07	// no fpu; fault
#define TRAPDF   0x08	// double-fault; fault, error == 0
#define TRAPRES1 0x09	// reserved
#define TRAPTS   0x0a	// invalid tss; fault + error
#define TRAPNP   0x0b	// segment not present; fault + error
#define TRAPSS   0x0c	// stack-segment fault; fault + error
#define TRAPGP   0x0d	// general protection; fault + error
#define TRAPPF   0x0e	// page-fault; fault + error
#define TRAPRES2 0x0f	// reserved
#define TRAPMF   0x10	// fpu error; fault + number
#define TRAPAC   0x11	// alignment check; fault, error == 0
#define TRAPMC   0x12	// machine check; abort
#define TRAPXF   0x13	// simd exception; fault
/* traps 0x14 through 0x1f are reserved */
#define TRAPNCPU 0x20

/* IRQs; names listed below */
#define IRQBIT   0x20   // set if IRQ
#define IRQBASE  0x20   // first IRQ
#define IRQ0     0x20
#define IRQ1     0x21
#define IRQ2     0x22
#define IRQ3     0x23
#define IRQ4     0x24
#define IRQ5     0x25
#define IRQ6     0x26
#define IRQ7     0x27
#define IRQ8     0x28
#define IRQ9     0x29
#define IRQ10    0x2a
#define IRQ11    0x2b
#define IRQ12    0x2c
#define IRQ13    0x2d
#define IRQ14    0x2e
#define IRQ15    0x2f
#define TRAPMASK 0xff

/* irq numbers */
#define IRQTIMER    0
#define IRQKBD      1
#define IRQCASCADE  2
#define IRQCOM2OR4  3
#define IRQCOM1OR3  4
#define IRQLPT      5
#define IRQFD       6
#define IRQRTC      8
#define IRQMOUSE    12
#define IRQFPU      13
#define IRQIDE0     14
#define IRQIDE1     15
#define NIRQ        16
#if (SMP)
#define IRQERROR    19
#define IRQIPI      29
#define IRQSPURIOUS 31
#endif

#endif /* __KERN_UNIT_IA32_TRAP_H__ */

