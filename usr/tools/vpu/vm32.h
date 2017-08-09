#ifndef __VPU_VM32_H__
#define __VPU_VM32_H__

/* VIRTUAL MACHINE */

#include <stdint.h>
#include <endian.h>
#include <vpu/types.h>

#define vpuclrmsw(vm)   ((vm)->sysregs[VPU_MSW] = 0)
#define vpusetcf(vm)    ((vm)->sysregs[VPU_MSW] |= VPU_MSW_CF)
#define vpusetzf(vm)    ((vm)->sysregs[VPU_MSW] |= VPU_MSW_ZF)
#define vpusetof(vm)    ((vm)->sysregs[VPU_MSW] |= VPU_MSW_OF)
#define vpusetif(vm)    ((vm)->sysregs[VPU_MSW] |= VPU_MSW_IF)
#define vpucfset(vm)    ((vm)->sysregs[VPU_MSW] & VPU_MSW_CF)
#define vpuzfset(vm)    ((vm)->sysregs[VPU_MSW] & VPU_MSW_ZF)
#define vpuofset(vm)    ((vm)->sysregs[VPU_MSW] & VPU_MSW_OF)
#define vpuifset(vm)    ((vm)->sysregs[VPU_MSW] & VPU_MSW_IF)

#define VPU_NGENREG     16 // number of registers in group (general, system)

/* accumulator (general-purpose register) IDs */
#define VPU_R0_REG      0x00
#define VPU_R1_REG      0x01
#define VPU_R2_REG      0x02
#define VPU_R3_REG      0x03
#define VPU_R4_REG      0x04
#define VPU_R5_REG      0x05
#define VPU_R6_REG      0x06
#define VPU_R7_REG      0x07
#define VPU_R8_REG      0x08
#define VPU_R9_REG      0x09
#define VPU_R10_REG     0x0a
#define VPU_R11_REG     0x0b
#define VPU_R12_REG     0x0c
#define VPU_R13_REG     0x0d
#define VPU_R14_REG     0x0e
#define VPU_R15_REG     0x0f
/* system register IDs */
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define VPU_RET_LO      0x00 // [dual-word] return value register, low word
#define VPU_RET_HI      0x01 // [dual-word] return value register, high word
#else
#define VPU_RET_HI      0x00 // [dual-word] return value register, high word
#define VPU_RET_LO      0x01 // [dual-word] return value register, low word
#endif
#define VPU_MSW_REG     0x00 // machine status word
#define VPU_FP_REG      0x01 // frame pointer
#define VPU_SP_REG      0x02 // stack pointer
#define VPU_PC_REG      0x03 // program counter i.e. instruction pointer
#define VPU_SYSREG_BIT  0x08 // denotes system-only access
#define VPU_IHT_REG     0x08 // interrupt handler table base address
#define VPU_PDB_REG     0x09 // page directory base address register
#define VPU_TID_REG     0x0a // task ID register
#define VPU_TLS_REG     0x0b // thread-local storage base address register
#define VPU_TSR_REG     0x0c // task-structure/state base address
/* values for sysregs[VPU_MSW] */
#define VPU_MSW_ZF_BIT  (1 << 0) // zero-flag
#define VPU_MSW_OF_BIT  (1 << 1) // overflow-flag
#define VPU_MSW_CF_BIT  (1 << 2) // carry-flag
#define VPU_MWS_IF_BIT  (1 << 29) // interrupts pending
#define VPU_MSW_SF_BIT  (1 << 30) // system-mode
#define VPU_MSW_LF_BIT  (1 << 31) // bus lock flag
/* program segments */
#define VPU_TEXT        0x00 // code
#define VPU_RODATA      0x01 // read-only data (string literals etc.)
#define VPU_DATA        0x02 // read-write (initialised) data
#define VPU_BSS         0x03 // uninitialised (zeroed) runtime-allocated data
#define VPU_NSEG        0x04
struct vpu {
    vpureg   genregs[VPU_NREG];
    vpuureg  sysregs[VPU_NREG];
    vpuureg  segs[VPU_NSEG];
    vpuureg  seglims[VPU_NSEG];
    uint8_t *mem;
};

/* OPCODES */

#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define vpumkopid(unit, inst) ((uint8_t)((unit) | ((inst) << 4)))
#else
#define vpumkopid(unit, inst) ((uint8_t)(((unit) << 4) | (inst)))
#endif

/* argument type flags */
#define VPU_INDIR_BIT   0x04 // indirect address
#define VPU_INDEX_BIT   0x02 // index for address
#define VPU_IMMED_BIT   0x01 // immediate argument
#define VPU_REG_ARG     0x00 // register
#define VPU_ARGT_BITS   3

/* addressing modes */
#define VPU_INDEX_ADR   0x00 // ndx(%reg), ndx in opcode imm-field
#define VPU_DIRECT_ADR  0x01 // address word follows opcode
#define VPU_PIC_ADR     0x02 // PC-relative; position-independent code

struct vpuop {
    unsigned int unit : 4;
    unsigned int reg1 : 4;
    unsigned int reg2 : 4;
    unsigned int adr1 : 2;
    unsigned int adr2 : 2;
    unsigned int code : 8;
    int          imm8 : 8;
};

/*
 * special uses for imm8
 * - shift or rotation count
 * - flags
 */
#define VPU_CL_LOCK_BIT  0x40
#define VPU_BUS_LOCK_BIT 0x80 // lock bus for operation

#if 0
struct vpuop {
    unsigned int unit  : 4;     // execution unit
    unsigned int inst  : 4;     // per-unit instruction
    unsigned int reg1  : 4;     // argument #1 register ID
    unsigned int reg2  : 4;     // argument #2 register ID
    unsigned int arg1t : 3;     // argument #1 type
    unsigned int arg2t : 3;     // argument #2 type
    unsigned int argsz : 2;     // argument size is 8 << argsz bits
    uint8_t      imm8;          // immediate argument or flags
    vpureg       imm[EMPTY];    // possible immediate argument
};
#endif

/* predefined I/O ports */
#define VPU_STDIN_PORT  0       // keyboard input
#define VPU_STDOUT_PORT 1       // console or framebuffer output
#define VPU_STDERR_PORT 2       // console or framebuffer output
#define VPU_MOUSE_PORT  3       // mouse input
#define VPU_RTC_PORT    4       // real-time clock
#define VPU_TMR_PORT    5       // timer interrupt configuration

/* framebuffer graphics interface */
#define VPU_FB_BASE     (3UL * 1024 * 1024 * 1024)      // base address

#endif /* __VPU_VM32_H__ */

