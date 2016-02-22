#ifndef __ZPF_VM_H__
#define __ZPF_VM_H__

#include <stddef.h>
#include <zpm/conf.h>
#include <zpf/zpf.h>

#define ZPF_MEM_NWORD    16

/* msw (machine status word) bits */
#define ZPF_TF_BIT       (1L << 0) // traps enabled
#define ZPF_ZF_BIT       (1L << 1) // result is zero
#define ZPF_CF_BIT       (1L << 2) // carry-bit
#if !defined(ZPF_FAIL_OVERFLOW)
#define ZPF_OF_BIT       (1L << 3) // overflow
#endif
#if !defined(ZPF_FAIL_UNDERFLOW)
#define ZPF_UF_BIT       (1L << 4) // underflow
#endif
/* stat values; error codes for exit() */
#define ZPF_VM_SUCCESS   0x00   // succesful run-status
#define ZPF_VM_FAILURE   0x01   // unspecified failure
#define ZPF_INVAL        0x02   // invalid arguments or such problems
#define ZPF_NOMEM        0x03   // out of memory
#define ZPF_LOOP         0x04   // illegal branch backwards
#define ZPF_DIVZERO      0x05   // division or modulus with k == 0
#define ZPF_OVERFLOW     0x06   // arithmetic overflow
#define ZPF_UNDERFLOW    0x07   // arithmetic underflow

#define ZPF_MEM_NOP      4096

/* flag-bits for virtual machine funtions */
#define ZPF_A_REG_BIT    0x0001
#define ZPF_X_REG_BIT    0x0002
#define ZPF_K_ARG_BIT    0x0004
#define ZPF_MEM_BIT      0x0008
#define ZPF_PKT_BIT      0x0010
#define ZPF_PKT_LEN_BIT  0x0020
#define ZPF_OFS_BIT      0x0040
#define ZPF_MUL4_BIT     0x0080
#define ZPF_JT_BIT       0x0100
#define ZPF_JF_BIT       0x0200
/* need room for bits for two arguments */
typedef uint32_t         zpfbits_t;
/* flag-bits stored in high bigts of the k-field */

/* modes for the virtual machine */
#define ZPF_VM_NEW       0
#define ZPF_VM_RUN       1
#define ZPF_VM_STOP      2
#define ZPF_VM_IDLE      3
#define ZPF_VM_HALT      4

/* flg-member bits; virtual machine flags */
#define ZPF_VM_INIT_BIT  (1 << 0)
#define ZPF_VM_HALT_BIT  (1 << 1)
#define ZPF_VM_TERM_BIT  (1 << 2)
#define ZPF_VM_ABORT_BIT (1 << 3)
#define ZPF_VM_DUMP_BIT  (1 << 4)
#define ZPF_VM_CRASH_BIT (1 << 5)
#define ZPF_VM_SYNC_BIT  (1 << 6)
#define ZPF_VM_PROF_BIT  (1 << 7)
#define ZPF_VM_TRACE_BIT (1 << 8)
#define zpfvmsuccess(vm) ((vm)->exit == ZPF_VM_SUCCESS)
#define __STRUCT_ZPFVM_PAD                                              \
    (rounduppow2(sizeof(struct zpfvm), CLSIZE)) // padded to cacheline-boundary
struct zpfvm {
    zpfword_t         mem[ZPF_MEM_NWORD];
    zpfword_t         flg;      // virtual machine feature-bits
    zpfword_t         msw;      // machine-status word
    zpfword_t         pc;       // program counter aka instrution pointer
    zpfword_t         a;        // accumulator A
    zpfword_t         x;        // index register X
    zpfbits_t        *bits;     // flags for running program's instructions
    zpfword_t         len;      // #of bytes in pkt
    uint8_t          *pkt;      // network packet address
    struct zpfopinfo *opinfo;   // information on instructions
    zpfword_t         nop;      // number of 64-bit opcodes in prog
    struct zpfop     *prog;     // program image
    zpfword_t         mode;     // see above
    int               stat;     // exit status for exit()
};

#endif /* __ZPF_VM_H__ */

