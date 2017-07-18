#ifndef __ZPM_VM32_H__
#define __ZPM_VM32_H__

/* VIRTUAL MACHINE */

#include <stdint.h>
#include <endian.h>
#include <zpm/types.h>

#define zpmclrmsw(vm)   ((vm)->sysregs[ZPM_MSW] = 0)
#define zpmsetcf(vm)    ((vm)->sysregs[ZPM_MSW] |= ZPM_MSW_CF)
#define zpmsetzf(vm)    ((vm)->sysregs[ZPM_MSW] |= ZPM_MSW_ZF)
#define zpmsetof(vm)    ((vm)->sysregs[ZPM_MSW] |= ZPM_MSW_OF)
#define zpmsetif(vm)    ((vm)->sysregs[ZPM_MSW] |= ZPM_MSW_IF)
#define zpmcfset(vm)    ((vm)->sysregs[ZPM_MSW] & ZPM_MSW_CF)
#define zpmzfset(vm)    ((vm)->sysregs[ZPM_MSW] & ZPM_MSW_ZF)
#define zpmofset(vm)    ((vm)->sysregs[ZPM_MSW] & ZPM_MSW_OF)
#define zpmifset(vm)    ((vm)->sysregs[ZPM_MSW] & ZPM_MSW_IF)

#define ZPM_NGENREG     16 // number of registers in group (general, system)

/* accumulator (general-purpose register) IDs */
#define ZPM_R0_REG      0x00
#define ZPM_R1_REG      0x01
#define ZPM_R2_REG      0x02
#define ZPM_R3_REG      0x03
#define ZPM_R4_REG      0x04
#define ZPM_R5_REG      0x05
#define ZPM_R6_REG      0x06
#define ZPM_R7_REG      0x07
#define ZPM_R8_REG      0x08
#define ZPM_R9_REG      0x09
#define ZPM_R10_REG     0x0a
#define ZPM_R11_REG     0x0b
#define ZPM_R12_REG     0x0c
#define ZPM_R13_REG     0x0d
#define ZPM_R14_REG     0x0e
#define ZPM_R15_REG     0x0f
/* system register IDs */
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define ZPM_RET_LO      0x00 // [dual-word] return value register, low word
#define ZPM_RET_HI      0x01 // [dual-word] return value register, high word
#else
#define ZPM_RET_HI      0x00 // [dual-word] return value register, high word
#define ZPM_RET_LO      0x01 // [dual-word] return value register, low word
#endif
#define ZPM_MSW_REG     0x00 // machine status word
#define ZPM_FP_REG      0x01 // frame pointer
#define ZPM_SP_REG      0x02 // stack pointer
#define ZPM_PC_REG      0x03 // program counter i.e. instruction pointer
#define ZPM_SYSREG_BIT  0x08 // denotes system-only access
#define ZPM_IHT_REG     0x08 // interrupt handler table base address
#define ZPM_PDB_REG     0x09 // page directory base address register
#define ZPM_TID_REG     0x0a // task ID register
#define ZPM_TLS_REG     0x0b // thread-local storage base address register
#define ZPM_TSR_REG     0x0c // task-structure/state base address
/* values for sysregs[ZPM_MSW] */
#define ZPM_MSW_ZF_BIT  (1 << 0) // zero-flag
#define ZPM_MSW_OF_BIT  (1 << 1) // overflow-flag
#define ZPM_MSW_CF_BIT  (1 << 2) // carry-flag
#define ZPM_MWS_IF_BIT  (1 << 29) // interrupts pending
#define ZPM_MSW_SF_BIT  (1 << 30) // system-mode
#define ZPM_MSW_LF_BIT  (1 << 31) // bus lock flag
/* program segments */
#define ZPM_TEXT        0x00 // code
#define ZPM_RODATA      0x01 // read-only data (string literals etc.)
#define ZPM_DATA        0x02 // read-write (initialised) data
#define ZPM_BSS         0x03 // uninitialised (zeroed) runtime-allocated data
#define ZPM_NSEG        0x04
struct zpm {
    zpmreg   genregs[ZPM_NREG];
    zpmureg  sysregs[ZPM_NREG];
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
#define ZPM_INDIR_BIT   0x04 // indirect address
#define ZPM_INDEX_BIT   0x02 // index for address
#define ZPM_IMMED_BIT   0x01 // immediate argument
#define ZPM_REG_ARG     0x00 // register
#define ZPM_ARGT_BITS   3

/*
 * special uses for imm8
 * - shift or rotation count
 * - flags
 */
#define ZPM_CL_LOCK_BIT  0x40
#define ZPM_BUS_LOCK_BIT 0x80 // lock bus for operation

struct zpmop {
    unsigned int unit  : 4;     // execution unit
    unsigned int inst  : 4;     // per-unit instruction
    unsigned int reg1  : 4;     // argument #1 register ID
    unsigned int reg2  : 4;     // argument #2 register ID
    unsigned int arg1t : 3;     // argument #1 type
    unsigned int arg2t : 3;     // argument #2 type
    unsigned int argsz : 2;     // argument size is 8 << argsz bits
    uint8_t      imm8;          // immediate argument or flags
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

#endif /* __ZPM_VM32_H__ */

