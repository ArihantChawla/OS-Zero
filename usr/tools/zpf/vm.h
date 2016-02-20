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
/* bits-argument */
#define ZPF_ALU_NBIT     8
#define ZPF_MEM_NBIT     8
#define ZPF_FLOW_NBIT    8
#define ZPF_XFER_NBIT    8
#define ZPF_ALU_SRC      (1 << 0)
#define ZPF_ALU_DEST     (1 << 1)
#define ZPF_ALU_DPTR     (1 << 2)
#define ZPF_ALU_SYNC     (1 << 3)
#define ZPF_ALU_FILT     (1 << 4)
#define ZPF_DYN_MEM      (1 << 5)

#define zpfopinfo(vm, op, bits)                                         \
    ((vm)->opinfo->

static __inline__ zpfword_t
zpfaluop2(struct zpfvm *vm, struct zpfop *op, zpfword_t ip,
          zpfword_t src, zpfword_t dest, zpfword_t n, zpfword_t bits)
{
    zpfword_t  a = (vm)->a;
    zpfword_t  _destb = zpfgetarg2byte(op);
    zpfword_t  _dest = 0;
    zpfword_t  _srcb = zpfgetarg1byte(op);
    zpfword_t  _src = 0;
    zpfword_t *_dptr = ((bits) & ZPF_ALU_DPTR) ? &(dest) : NULL;
    zpfopfunc *_func = zpfopfunctab[(op)->code & ((1L << ZPF_OP_NBIT) - 1)];

    do {
        if (bits) {
            if ((bits) & ZPF_ALU_SRC) {
                _src = (src);
            }
            if ((bits) & ZPF_ALU_DEST) {
                _dest = (dest);
            }
            (ip)++;
            if ((func)
                && (((bits) && (ZPF_ALU_SRC | ZPF_ALU_DEST | ZPF_ALU_DPTR))
                    == (ZPF_ALU_SRC_ | ZPF_ALU_DEST | ZPF_ALU_DPTR))) {
                *_dptr = func(vm, op, _src, _dest, _dptr, bits);
            } else {
                *((uint8_t *)NULL);
            }
            (vm)->pc = (ip);
        }
    } while (n--);
}

/* for the NEG-operation */
static __inline__ zpfword_t
zpfaluop1(struct zpfvm *vm, struct zpfop *op, zpfword_t ip,
          zpmword_t src, zpmword_t *dptr, zpfword_t bits)
{
    zpfword_t  a = (vm)->a;
    zpfword_t  dest = *dptr;
    zpfword_t  srcb = zpfgetarg1byte(op);
    
    (ip) = (vm)->pc;
    if (_srcb & ZPF_REG_BIT) {
        src = (vm)->x;
    } else if (_srcb & ZPF_BIG_WORD_BIT) {
        src = (op)->k & ZPF_K_VAL_MASK;
    }
    (ip)++;
    *dptr = func(vm, op, src, dest, dptr, bits);
    (vm)->pc = (ip);
                  
    return dest;
}

#define zpfinitmemop2(vm, op, ip, src, dest, bits)                      \
    do {                                                                \
        zpfword_     _unit = ZPF_MEM_UNIT;                              \
        zpfword_t    _destb = zpfgetarg2byte(op);                       \
        zpfword_t    _dest = (dest);                                    \
        zpfword_t    _srcb = zpfgetarg1byte(op);                        \
        zpfword_t    _src = (src);                                      \
        zpfword_t   *_dptr = &(dest);                                   \
        zpfopfunc_t *func = zpfopfunctab[_unit][_inst];                 \
                                                                        \
        if (bits) {                                                     \
            if (bits & ZPF_
        }                                                               \
    } while (0)

#define zpfinitldop1(vm, op, ip, dest, bits)                            \
    do {                                                                \
        zpfword_t _val;                                                 \
        zpfword_t _destb = zpfgetarg1byte(op);                          \
        zpfword_t *_aptr = &(vm)-a;                                     \
                                                                        \
        if (bits) {                                                     \
            if (bits & 
          (ip) = vm->pc;                                                \
            if (_arg1t & ZPF_IMMED_BIT) {                               \
                (vm)->a = _val;                                         \
            } else if (_arg1t & ZPF_PKT_BIT) {                          \
                _adr += (vm)->pkt;                                      \
                if (_arg1t & ZPF_OFS_FIT) {                             \
                    _adr += (op)->k;                                    \
                }                                                       \
            }                                                           \
        }                                                               \
    } while (0);

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

