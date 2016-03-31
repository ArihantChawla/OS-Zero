#ifndef __ZPM_ZPM_H__
#define __ZPM_ZPM_H__

#include <stdint.h>

typedef int8_t   zpmbyte;
typedef uint8_t  zpmubyte;
typedef int16_t  zpmword;
typedef uint16_t zpmuword;
typedef int32_t  zpmlong;
typedef uint32_t zpmulong;
#if defined(ZPM64BIT)
typedef int64_t  zpmquad;
typedef uint64_t zpmuquad;
#endif

/* INSTRUCTION SET */

/* no operation */
#define ZPM_NOP     0x00        // no operation
/* ALU (arithmetic-logical unit) instructions */
/* bitwise operations */
#define ZPM_NOT     0x01        // 2's complement (reverse all bits)
#define ZPM_AND     0x02        // logical bitwise AND
#define ZPM_OR      0x03        // logical bitwise OR
#define ZPM_XOR     0x04        // logical bitwise XOR (exclusive OR)
#define ZPM_SHL     0x05        // shift left
#define ZPM_SHR     0x06        // logical shift right (fill with zero)
#define ZPM_SAR     0x07        // arithmetic shift right (fill with sign-bit)
#define ZPM_ROL     0x08        // rotate left
#define ZPM_ROR     0x09        // rotate right
/* arithmetic operations */
#define ZPM_INC     0x0a        // increment by one
#define ZPM_DEC     0x0b        // decrement by one
#define ZPM_ADD     0x0c        // addition
#define ZPM_SUB     0x0d        // subtraction
#define ZPM_CMP     0x0e        // compare; subtract and set flags
#define ZPM_MUL     0x0f        // multiplication
#define ZPM_DIV     0x10        // division; also yields modulus
/* branch operations */
#define ZPM_JMP     0x11        // branch unconditionally
#define ZPM_BZ      0x12        // branch if zero (ZF == 1)
#define ZPM_BNZ     0x13        // branch if non-zero (ZF == 0)
#define ZPM_BLT     0x14        // branch if less than
#define ZPM_BLE     0x15        // branch if less than or equal
#define ZPM_BGT     0x16        // branch if greater than
#define ZPM_BGE     0x17        // branch if greater than or equal
#define ZPM_BO      0x18        // branch if overflow set
#define ZPM_BNO     0x19        // branch if overflow not set
#define ZPM_BC      0x1a        // branch if carry set
#define ZPM_BNC     0x1b        // branch if carry not set
/* stack operations */
#define ZPM_POP     0x1c        // pop from stack
#define ZPM_PUSH    0x1d        // push on stack
#define ZPM_PUSHA   0x1e        // push all registers on stack
/* load and store operations */
#define ZPM_LDA     0x1f        // load accumulator (register)
#define ZPM_STA     0x20        // store accumulator
/* subroutine operations */
#define ZPM_CALL    0x21        // call subroutine
#define ZPM_ENTER   0x22        // subroutine prologue
#define ZPM_LEAVE   0x23        // subroutine epilogue
#define ZPM_RET     0x24        // return from subroutine
/* thread operations */
#define ZPM_THR     0x25        // launch new thread
#define ZPM_LTB     0x26        // load base address for thread-local storage
/* system operations */
#define ZPM_LDR     0x27        // load special register
#define ZPM_STR     0x28        // store special register
#define ZPM_RST     0x29        // reset
#define ZPM_HLT     0x2a        // halt
/* I/O operations */
#define ZPM_IN      0x2b        // read data from port
#define ZPM_OUT     0x2c        // write data to port
#define ZPM_NALU_OP 0x2d        // number of ALU operations

/* accumulator (general-purpose register) IDs */
#define ZPM_REG0    0x00
#define ZPM_REG1    0x01
#define ZPM_REG2    0x02
#define ZPM_REG3    0x03
#define ZPM_REG4    0x04
#define ZPM_REG5    0x05
#define ZPM_REG6    0x06
#define ZPM_REG7    0x07
#define ZPM_REG8    0x08
#define ZPM_REG9    0x09
#define ZPM_REGA    0x0a
#define ZPM_REGB    0x0b
#define ZPM_REGC    0x0c
#define ZPM_REGD    0x0d
#define ZPM_REGE    0x0e
#define ZPM_REGF    0x0f
#define ZPM_NGENREG 16
/* special register IDs */
#define ZPM_MSW     0x00        // machine status word
#define ZPM_PC      0x01        // program counter i.e. instruction pointer
#define ZPM_FP      0x02        // frame pointer
#define ZPM_SP      0x03        // stack pointer
#define ZPM_PDB     0x04        // page director base address register
#define ZPM_NSYSREG 16

struct zpm {
};

/* argument type flags */
#define ZPM_REG_ARG (1 << 0)    // argument is a register
#define ZPM_IMM_VAL (1 << 1)    // argument is an immediate value
#define ZPM_IMM_ADR (1 << 2)    // argument is an immediate address
#define ZPM_IMM_NDX (1 << 3)    // argument is an immediate index
#define ZPM_REG_ADR (1 << 4)    // argument is an address in register
#define ZPM_REG_NDX (1 << 5)    // argumetn is an index in register
struct zpmop {
    unsigned int code  : 6;
    unsigned int unit  : 2;     // ALU, FPU, SYS, IO
    unsigned int reg1  : 4;     // argument #1 register ID
    unsigned int reg2  : 4;     // argument #2 register ID
    unsigned int argt  : 6;     // arguemnt types
    unsigned int argsz : 2;     // argument size is 8 << argsz
    unsigned int imm8  : 8;     // immediate argument such as shift count
};

#endif /* __ZPM_ZPM_H__ */

