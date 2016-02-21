#ifndef __ZPF_VM_H__
#define __ZPF_VM_H__

#include <stddef.h>
#include <zpm/conf.h>
#include <zpf/zpf.h>

#define ZVM_MEM_NWORD    16

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

/* flg-member bits; virtual machine flags */
#define ZPF_VM_INIT_BIT  (1 << 31)
#define ZPF_VM_HALT_BIT  (1 << 30)
#define ZPF_VM_TERM_BIT  (1 << 29)
#define ZPF_VM_ABORT_BIT (1 << 28)
#define ZPF_VM_DUMP_BIT  (1 << 27)
#define ZPF_VM_CRASH_BIT (1 << 26)
#define ZPF_VM_SYNC_BIT  (1 << 25)
#define zpfvmsuccess(vm) ((vm)->exit == ZPF_VM_SUCCESS)
struct zpfvm {
    zpfword_t         mem[ZPF_MEM_NWORD];
    zpfword_t         a;        // accumulator A
    zpfword_t         x;        // index register X
    zpfword_t         pc;       // program counter aka instrution pointer
    zpfword_t         flg;      // virtual machine feature-bits
    struct zpfsym    *sym;      // current symbol/label
    zpfword_t         len;      // #of bytes in pkt
    uint8_t          *pkt;      // network packet address
    zpfword_t         msw;      // machine-status word
    zpfword_t         nop;      // number of struct zpfopd in img
    struct zpfop     *img;      // program image
    struct zpfopinfo *opinfo;   // information on instructions
#if (ZPFDEBUGVM)
    struct zpfop     *op;       // current operation for debugging
#endif
    int               stat;     // exit status for exit()
};

#endif /* __ZPF_VM_H__ */

