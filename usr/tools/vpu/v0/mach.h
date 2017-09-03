#ifndef __VPU_V0_MACH_H__
#define __VPU_V0_MACH_H__

#include <stdint.h>

#define V0_NINST_MAX 256

/* Valhalla 0 (V0) processor machine interface */

/* bits for FR0 feature register */
#define V0_FR0_MMU (1U << 0) // memory management unit present
#define V0_FR0_FPU (1U << 1) // floating-point processor present
#define V0_FR0_GPU (1U << 2) // graphics processor present
#define V0_FR0_DSP (1U << 3) // digital signal processor present
#define V0_FR0_FPM (1U << 4) // fixed-point processor present

/* processor sub-units */
#define V0_BITS    0x00 // bitwise operations such as logical functions
#define V0_SHIFT   0x10 // shift operations (later on, probably rotates as well)
#define V0_ARITH   0x20 // basic arithmetics
#define V0_FLOW    0x30 // flow control; jumps, function calls, branches
#define V0_XFER    0x40 // data transfers between registers and memory
#define V0_STACK   0x50 // stack operations
#define V0_IO      0x60 // I/O operations
#define V0_COUNIT  0x80 // co-unit flag-bit
#define V0_MMU     0x80 // memory management unit
#define V0_FPU     0x90 // floating-point unit
#define V0_GPU     0xa0 // graphics processor
#define V0_DSP     0xb0 // digital signal processor
#define V0_FPM     0xc0 // fixed-point mathematics
#define V0_COPROC  0xf0 // miscellaneous co-processors (separate op-words)

/* sub-unit instructions */

/* NOP is declared as all 0-bits */
#define V0_NOP     (UINT32_C(~0))
#define v0instisnop(op)                                                 \
    (*(uint32_t *)op == V0_NOP)

/* arg1 -> register or val-field immediate argument */

/* V0_BITS */
#define V0_NOT     0x00 // reg1 = ~reg1;
#define V0_AND     0x01 // reg2 &= arg1;
#define V0_OR      0x02 // reg2 |= arg1;
#define V0_XOR     0x03 // reg2 ^= arg1;

/* V0_SHIFT */
#define V0_SHL     0x00 // reg2 <<= arg1;
#define V0_SHR     0x01 // reg2 >>= arg1;
#define V0_SAR     0x02 // reg2 >>>= arg1;

/* V0_ARITH */
#define V0_INC     0x00 // reg1++;
#define V0_DEC     0x01 // reg1--;
#define V0_ADD     0x02 // reg2 += arg1;
#define V0_ADC     0x03 // reg2 += arg1; // sets carry-bit
#define V0_SUB     0x04 // reg2 -= arg1; // reg2 += (~arg1) + 1
#define V0_SBB     0x05 // reg2 -= arg1; // sets carry-bit to borrow
#define V0_CMP     0x06 // set bits for reg2 - arg1
#define V0_MUL     0x07 // reg2 *= arg1;
#define V0_DIV     0x08 // reg2 /= arg1;
#define V0_REM     0x09 // reg2 %= arg1;
//#define V0_CRM     0x0a // calculate reciprocal multiplier for division

/* V0_FLOW */
#define V0_JMP     0x00  // jmp to given address
#define V0_CPL     0x01  // call prologue
#define V0_CALL    0x02  // call subroutine
#define V0_ENTER   0x03  // create stack frame for subroutine
#define V0_LEAVE   0x04  // destroy stack frame before RET from subroutine
#define V0_RET     0x05  // return from subroutine
#define V0_BZ      0x06  // branch if MSW_ZF is set
#define V0_BNZ     0x07  // branch if MSW_ZF is zero
#define V0_BC      0x08  // branch if MSW_CF is set
#define V0_BNC     0x09  // branch if MSW_CF is zero
#define V0_BO      0x0a  // branch if MSW_OF is set
#define V0_BNO     0x0b  // branch if MSW_OF is zero
#define V0_BEQ     V0_BZ // branch if equal
/* TODO: flag combinations for BLT and below */
#define V0_BLT     0x0c  // branch if less than
#define V0_BLE     0x0d  // branch if less than or equal
#define V0_BGT     0x0e  // branch if greater than
#define V0_BGE     0x0f  // branch if greater than or equal

/* V0_XFER */
#define V0_LDR     0x00  // load register
#define V0_STR     0x01  // store into memory

/* V0_STACK */
#define V0_PSH     0x00  // push to stack
#define V0_POP     0x01  // pop from stack into register
#define V0_PSHA    0x04  // push all general-purpose registers
#define V0_POPA    0x05  // pop all general-purpose registers

/* V0_IO */
#define V0_IOR     0x00  // read data from I/O port
#define V0_IOW     0x01  // write data to I/O port

#endif /* __VPU_V0_MACH_H__ */

