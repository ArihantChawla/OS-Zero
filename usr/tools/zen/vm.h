#ifndef __ZEN_VM_H__
#define __ZEN_VM_H__

#include <stddef.h>
#include <zpm/conf.h>
#include <zen/zen.h>

#define ZEN_MEM_NWORD    16

/* msw (machine status word) bits */
#define ZEN_TF_BIT       (1L << 0) // traps enabled
#define ZEN_ZF_BIT       (1L << 1) // result is zero
#define ZEN_CF_BIT       (1L << 2) // carry-bit
#if !defined(ZEN_FAIL_OVERFLOW)
#define ZEN_OF_BIT       (1L << 3) // overflow
#endif
#if !defined(ZEN_FAIL_UNDERFLOW)
#define ZEN_UF_BIT       (1L << 4) // underflow
#endif
/* stat values; error codes for exit() */
#define ZEN_VM_SUCCESS   0x00   // succesful run-status
#define ZEN_VM_FAILURE   0x01   // unspecified failure
#define ZEN_INVAL        0x02   // invalid arguments or such problems
#define ZEN_NOMEM        0x03   // out of memory
#define ZEN_LOOP         0x04   // illegal branch backwards
#define ZEN_DIVZERO      0x05   // division or modulus with k == 0
#define ZEN_OVERFLOW     0x06   // arithmetic overflow
#define ZEN_UNDERFLOW    0x07   // arithmetic underflow

#define ZEN_MEM_NOP      4096

/* flag-bits for virtual machine funtions */
#define ZEN_A_REG_BIT    0x0001
#define ZEN_X_REG_BIT    0x0002
#define ZEN_K_ARG_BIT    0x0004
#define ZEN_MEM_BIT      0x0008
#define ZEN_PKT_BIT      0x0010
#define ZEN_PKT_LEN_BIT  0x0020
#define ZEN_OFS_BIT      0x0040
#define ZEN_MUL4_BIT     0x0080
#define ZEN_JT_BIT       0x0100
#define ZEN_JF_BIT       0x0200
/* need room for bits for two arguments */
typedef uint32_t         zenbits_t;
/* flag-bits stored in high bigts of the k-field */

/* modes for the virtual machine */
#define ZEN_VM_NEW       0
#define ZEN_VM_RUN       1
#define ZEN_VM_STOP      2
#define ZEN_VM_IDLE      3
#define ZEN_VM_HALT      4

/* flg-member bits; virtual machine flags */
#define ZEN_VM_INIT_BIT  (1 << 0)
#define ZEN_VM_HALT_BIT  (1 << 1)
#define ZEN_VM_TERM_BIT  (1 << 2)
#define ZEN_VM_ABORT_BIT (1 << 3)
#define ZEN_VM_DUMP_BIT  (1 << 4)
#define ZEN_VM_CRASH_BIT (1 << 5)
#define ZEN_VM_SYNC_BIT  (1 << 6)
#define ZEN_VM_PROF_BIT  (1 << 7)
#define ZEN_VM_TRACE_BIT (1 << 8)
#define zenvmsuccess(vm) ((vm)->exit == ZEN_VM_SUCCESS)
#if 0
#define __STRUCT_ZENVM_PAD                                              \
    (rounduppow2(sizeof(struct zenvm), CLSIZE)) // padded to cacheline-boundary
#endif
struct zenvm {
    zenword_t         mem[ZEN_MEM_NWORD];
    zenword_t         flg;      // virtual machine feature-bits
    zenword_t         msw;      // machine-status word
    zenword_t         pc;       // program counter aka instrution pointer
    zenword_t         a;        // accumulator A
    zenword_t         x;        // index register X
    zenbits_t        *bits;     // flags for running program's instructions
    zenword_t         len;      // #of bytes in pkt
    uint8_t          *pkt;      // network packet address
    struct zenopinfo *opinfo;   // information on instructions
    zenword_t         nop;      // number of 64-bit opcodes in prog
    struct zenop     *prog;     // program image
    zenword_t         mode;     // see above
    int               stat;     // exit status for exit()
} ALIGNED(PAGESIZE);

#endif /* __ZEN_VM_H__ */

