#ifndef __VPU_V0_MACH_H__
#define __VPU_V0_MACH_H__

#include <stdint.h>

#define V0_NINST_MAX 256

/* Valhalla 0 (V0) processor machine interface */

/* register IDs */
#define V0_REG_BIT(r) (1U << V0_##r) // bitmap indices for PSHM, POPM
/* 16 general purpose registers R0..R15 */
#define V0_R0      0x00
#define V0_R1      0x01
#define V0_R2      0x02
#define V0_R3      0x03
#define V0_R4      0x04
#define V0_R5      0x05
#define V0_R6      0x06
#define V0_R7      0x07
#define V0_R8      0x08
#define V0_R9      0x09
#define V0_R10     0x0a
#define V0_R11     0x0b
#define V0_R12     0x0c
#define V0_R13     0x0d
#define V0_R14     0x0e
#define V0_R15     0x0f
#define V0_XREG    0x10 // special/system registers
#define V0_MSW     0x10 // machine status word
#define V0_PC      0x11 // program counter (instruction pointer)
#define V0_FP      0x12 // [stack] frame pointer
#define V0_SP      0x13 // stack pointer
#define V0_ROREG   0x18 // read-only/hardwired registers
#define V0_FR0     0x19 // feature register #0; flags indicating processor setup

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
#define V0_CALL    0x01  // call subroutine
#define V0_RET     0x02  // return from subroutine
#define V0_BZ      0x03  // branch if MSW_ZF is set
#define V0_BNZ     0x04  // branch if MSW_ZF is zero
#define V0_BC      0x05  // branch if MSW_CF is set
#define V0_BNC     0x06  // branch if MSW_CF is zero
#define V0_BO      0x07  // branch if MSW_OF is set
#define V0_BNO     0x08  // branch if MSW_OF is zero
#define V0_BEQ     V0_BZ // branch if equal
/* TODO: flag combinations for BLT and below */
#define V0_BLT     0x09  // branch if less than
#define V0_BLE     0x0a  // branch if less than or equal
#define V0_BGT     0x0b  // branch if greater than
#define V0_BGE     0x0c  // branch if greater than or equal

/* V0_XFER */
#define V0_LDR     0x00  // load register from memory
#define V0_STR     0x01  // store register into memory
//#define V0_LDX     0x02  // load special-register from memory
//#define V0_STX     0x03  // store special-register into memory

/* V0_STACK */
#define V0_PSH     0x00  // push from register
#define V0_POP     0x01  // pop into register
#define V0_PSHA    0x04  // push all general-purpose registers
#define V0_POPA    0x05  // pop all general-purpose reigsters
#define V0_PSHM    0x06  // push set of registers [bitmap follows opcode]
#define V0_POPM    0x07  // pop set of registers [bitmap follows opcode]

/* V0_IO */
#define V0_IOR     0x00  // read data from I/O port
#define V0_IOW     0x01  // write data to I/O port

#endif /* __VPU_V0_MACH_H__ */

