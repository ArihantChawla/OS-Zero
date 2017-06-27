#ifndef __ZPM_ZPM_H__
#define __ZPM_ZPM_H__

#include <zpm/conf.h>
#include <stdint.h>
#include <endian.h>
#include <zero/cdefs.h>

typedef int8_t   zpmbyte;
typedef uint8_t  zpmubyte;
typedef int16_t  zpmword;
typedef uint16_t zpmuword;
typedef int32_t  zpmlong;
typedef uint32_t zpmulong;
#if defined(ZPM64BIT)
typedef int64_t  zpmquad;
typedef uint64_t zpmuquad;
typedef zpmquad  zpmreg;
typedef zpmuquad zpmureg;
#else
typedef zpmlong  zpmreg;
typedef zpmulong zpmureg;
#endif
typedef zpmureg  zpmadr;

/* INSTRUCTION SET */

/* ALU (arithmetic-logical unit) instructions */
/* no operation */
#define ZPM_NO_UNIT     0x00
#define ZPM_NO_INST     0x00
#define ZPM_NOP         zpmmkopid(ZPM_NO_UNIT, ZPM_NO_INST) // no operation
/* bitwise operations */
/* logic unit */
#define ZPM_LOGIC       0x01
#define ZPM_NOT         0x00 // 2's complement (reverse all bits)
#define ZPM_AND         0x01 // logical bitwise AND
#define ZPM_OR          0x02 // logical bitwise OR
#define ZPM_XOR         0x03 // logical bitwise XOR (exclusive OR)
/* shifter */
#define ZPM_SHIFTER     0x02
#define ZPM_RIGHT_BIT   0x01
#define ZPM_SHIFT_BIT   0x02
#define ZPM_ROT_BIT     0x04
#define ZPM_SIGN_BIT    0x04
#define ZPM_SHL         ZPM_SHIFT_BIT
#define ZPM_SHR         (ZPM_SHIFT_BIT | ZPM_RIGHT_BIT)
#define ZPM_SAR         (ZPM_SHIFT_BIT | ZPM_RIGHT_BIT | ZPM_SIGN_BIT)
#define ZPM_ROL         ZPM_ROT_BIT
#define ZPM_ROR         (ZPM_ROT_BIT | ZPM_RIGHT_BIT)
/* arithmetic operations */
/* arithmetic unit */
#define ZPM_ARITH       0x03
#define ZPM_INC_BIT     0x01
#define ZPM_MSW_BIT     0x01
#define ZPM_DEC_BIT     0x02
#define ZPM_ADD_BIT     0x02
#define ZPM_CMP_BIT     0x02
#define ZPM_SUB_BIT     0x04
#define ZPM_ADD_INV_BIT 0x04
#define ZPM_XFER        0x00
#define ZPM_INC         ZPM_INC_BIT
#define ZPM_ADD         ZPM_ADD_BIT
#define ZPM_ADC         (ZPM_ADD_BIT | ZPM_MSW_BIT)
#define ZPM_ADI         (ZPM_ADD_INV_BIT) // a + ~b
#define ZPM_SUB         (ZPM_ADD_INV_BIT | ZPM_INC_BIT)
#define ZPM_SBC         (ZPM_ADD_INV_BIT | ZPM_INC_BIT | ZPM_MSW_BIT)
#define ZPM_DEC         (ZPM_SUB_BIT | ZPM_DEC_BIT)
#define ZPM_CMP         (ZPM_SUB_BIT | ZPM_CMP_BIT | ZPM_MSW_BIT)
/* multiplier */
#define ZPM_MULTIPLIER  0x04
#define ZPM_MUL         0x00 // multiplication
/* divider */
#define ZPM_DIVIDER     0x05
#define ZPM_REM_BIT     0x01 // remainder-flag
#define ZPM_DIV         0x00 // division, result in ZPM_RET_LO
#define ZPM_REM         ZPM_REM_BIT // remainder of division in ZPM_RET_HI
/* load and store operations */
#define ZPM_LOAD_STORE  0x06
#define ZPM_LDA         0x00 // load accumulator (register)
#define ZPM_STA         0x01 // store accumulator
/* stack operations */
#define ZPM_STACK       0x07
#define ZPM_GENREGS_BIT 0x10
#define ZPM_POP_BIT     0x01
#define ZPM_PSH         0x00 // pop from stack
#define ZPM_PSHA        ZPM_GENREGS_BIT // pop all genregs
#define ZPM_POP         ZPM_POP_BIT // push on stack
#define ZPM_POPA        (ZPM_POP_BIT | ZPM_GENREGS_BIT) // push all genregs
/* I/O operations */
#define ZPM_IO          0x08
#define ZPM_IN          0x00 // read data from port
#define ZPM_OUT         0x01 // write data to port
/* branch and subroutine operations */
#define ZPM_BRANCH      0x09
#define ZPM_JMP         0x00 // branch unconditionally
#define ZPM_BZ          0x01 // branch if zero (ZF == 1)
#define ZPM_BNZ         0x02 // branch if non-zero (ZF == 0)
#define ZPM_BLT         0x03 // branch if less than
#define ZPM_BLE         0x04 // branch if less than or equal
#define ZPM_BGT         0x05 // branch if greater than
#define ZPM_BGE         0x06 // branch if greater than or equal
#define ZPM_BO          0x07 // branch if overflow set
#define ZPM_BNO         0x08 // branch if overflow not set
#define ZPM_BC          0x09 // branch if carry set
#define ZPM_BNC         0x0a // branch if carry not set
#define ZPM_CALL        0x0b // trigger function call
#define ZPM_THR         0x0c // launch new thread
#define ZPM_ENTER       0x0d // function prologue
#define ZPM_LEAVE       0x0e // function epilogue
#define ZPM_RET         0x0f // return from subroutine or thread
/* system operations */
#define ZPM_SYS         0x0a
#define ZPM_WRM         0x00 // write data to memory-mapped devices
#define ZPM_LDR         0x01 // load special register
#define ZPM_STR         0x02 // store special register
#define ZPM_RST         0x03 // reset
#define ZPM_HLT         0x04 // halt
/* processor parameters */
#define ZPM_NALU_OP     0xff // max number of ALU operations
#define ZPM_NALU_RES    256
#define ZPM_FPU         0x0e
#define ZPM_COPROC      0x0f // special unit ID to dispatch execution

/* VIRTUAL MACHINE */

#define zpmclrmsw(vm)   ((vm)->sysregs[ZPM_MSW] = 0)
#define zpmsetcf(vm)    ((vm)->sysregs[ZPM_MSW] |= ZPM_MSW_CF)
#define zpmsetzf(vm)    ((vm)->sysregs[ZPM_MSW] |= ZPM_MSW_ZF)
#define zpmsetof(vm)    ((vm)->sysregs[ZPM_MSW] |= ZPM_MSW_OF)
#define zpmcfset(vm)    ((vm)->sysregs[ZPM_MSW] & ZPM_MSW_CF)
#define zpmzfset(vm)    ((vm)->sysregs[ZPM_MSW] & ZPM_MSW_ZF)
#define zpmofset(vm)    ((vm)->sysregs[ZPM_MSW] & ZPM_MSW_OF)

/* accumulator (general-purpose register) IDs */
#define ZPM_REG0        0x00
#define ZPM_REG1        0x01
#define ZPM_REG2        0x02
#define ZPM_REG3        0x03
#define ZPM_REG4        0x04
#define ZPM_REG5        0x05
#define ZPM_REG6        0x06
#define ZPM_REG7        0x07
#define ZPM_REG8        0x08
#define ZPM_REG9        0x09
#define ZPM_REG10       0x0a
#define ZPM_REG11       0x0b
#define ZPM_REG12       0x0c
#define ZPM_REG13       0x0d
#define ZPM_REG14       0x0e
#define ZPM_REG15       0x0f
#define ZPM_NGENREG     16
/* system register IDs */
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define ZPM_RET_LO      0x00 // [dual-word] return value register, low word
#define ZPM_RET_HI      0x01 // [dual-word] return value register, high word
#else
#define ZPM_RET_HI      0x00 // [dual-word] return value register, high word
#define ZPM_RET_LO      0x01 // [dual-word] return value register, low word
#endif
#define ZPM_MSW         0x00 // machine status word
#define ZPM_PC          0x01 // program counter i.e. instruction pointer
#define ZPM_FP          0x02 // frame pointer
#define ZPM_SP          0x03 // stack pointer
#define ZPM_PDB         0x04 // page directory base address register
#define ZPM_TLB         0x05 // thread-local storage base address register
#define ZPM_NSYSREG     16
/* values for sysregs[ZPM_MSW] */
#define ZPM_MSW_CF      (1 << 0) // carry-flag
#define ZPM_MSW_ZF      (1 << 1) // zero-flag
#define ZPM_MSW_OF      (1 << 2) // overflow-flag
#define ZPM_MSW_LF      (1 << 31) // bus lock flag
/* program segments */
#define ZPM_TEXT        0x00 // code
#define ZPM_RODATA      0x01 // read-only data (string literals etc.)
#define ZPM_DATA        0x02 // read-write (initialised) data
#define ZPM_BSS         0x03 // uninitialised (zeroed) runtime-allocated data
#define ZPM_TLS         0x04 // thread-local storage
#define ZPM_NSEG        0x08
struct zpm {
    zpmreg   genregs[ZPM_NGENREG];
    zpmureg  sysregs[ZPM_NSYSREG];
    zpmureg *segs[ZPM_NSEG];
    zpmureg  seglims[ZPM_NSEG];
    uint8_t *mem;
};

/* OPCODES */

#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define zpmmkopid(unit, inst) ((uint8_t)((unit) | ((inst) << 4)))
#else
#define zpmmkopid(unit, inst) ((uint8_t)(((unit) << 4) | (inst)))
#endif

/* argument type flags */
#define ZPM_MEM_BIT      (1 << 2) // memory address argument (default register)
#define ZPM_IMM_ARG      0x00 // immediate argument value
#define ZPM_ADR_ARG      0x01 // address argument
#define ZPM_NDX_ARG      0x02 // index argument
#define ZPM_ARGT_BITS    3
struct zpmop {
    uint8_t      code;          // unit/ZPM_COPROC in first 4 bits, inst in last
    unsigned int reg1  : 4;     // argument #1 register ID
    unsigned int reg2  : 4;     // argument #2 register ID
    unsigned int argt  : 6;     // argument types
    unsigned int argsz : 2;     // argument size is ARG_MIN << argsz
    uint8_t      imm8;          // immediate argument
    zpmreg       imm[EMPTY];    // possible immediate argument
};

/* predefined I/O ports */
#define ZPM_STDIN_PORT  0       // keyboard input
#define ZPM_STDOUT_PORT 1       // console or framebuffer output
#define ZPM_STDERR_PORT 2       // console or framebuffer output
#define ZPM_MOUSE_PORT  3       // mouse input
#define ZPM_RTC_PORT    4       // real-time clock
#define ZPM_TMR_PORT    5       // timer interrupt configuration

/* framebuffer graphics interface */
#define ZPM_FB_BASE     (3UL * 1024 * 1024 * 1024)      // base address

/* operation function prototypes */

#endif /* __ZPM_ZPM_H__ */

