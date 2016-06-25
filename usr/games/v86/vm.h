#ifndef __V86_VM_H__
#define __V86_VM_H__

#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <v86/conf.h>

/* virtual machine register and address sizes in bits */
#if (V86_WORD_SIZE == 4)
typedef int32_t  v86word;
typedef uint32_t v86uword;
#endif

/* pseudo-machine types */

/*
 * use 64-bit math where available and detected... better catches for overflows,
 * for one... :)
 */
#if (WORDSIZE == 8) || (v86_WORD_SIZE == 8)
typedef int64_t  v86reg;
typedef uint64_t v86ureg;
typedef uint64_t v86adr;
#else
typedef int32_t  v86reg;
typedef uint32_t v86ureg;
typedef uint32_t v86adr;
#endif

#define V86_NOP                 0x00    // no operation
#define V86_NOT                 0x01    // NOT r/i, r
#define V86_AND                 0x02    // AND r/i, r
#define V86_OR                  0x03    // OR r/i, r
#define V86_XOR                 0x04    // XOR r/i, r
#define V86_SHL                 0x05    // SHL r/i, r
#define V86_SHR                 0x06    // SHR r/i, r
#define V86_SAR                 0x07    // SAR r/i, r
#define V86_ADD                 0x08    // ADD r/i, r
#define V86_SUB                 0x09    // SUB r/i, r
#define V86_CMP                 0x0a    // CMP r/i, r - set flags
#define V86_MUL                 0x0b    // MUL r/i, r
#define V86_DIV                 0x0c    // DIV r/i, r
#define V86_JMP                 0x0d    // JMP i8       - short jump with offset
#define V86_LJMP                0x0e    // LJMP r/m     - long jump
#define V86_JZ                  0x0f    // JE r/m       - ZF == 1
#define V86_JE                  V86_JZ  // JZ r/m       - ZF == 1
#define V86_JNZ                 0x10    // JNE r/m      - ZF == 0
#define V86_JNE                 V86_JNZ // JNZ r/m      - ZF == 0
#define V86_JC                  0x11    // JC r/m       - CF == 1
#define V86_JLT                 V86_JC  // JLT r/m      - CF == 1
#define V86_JNC                 0x12    // JNC r/m      - CF == 0
#define V86_JO                  0x13    // JO r/m       - OF == 1
#define V86_JNO                 0x14    // JNO r/m      - OF == 0
#define V86_JLE                 0x15    // JLE r/m      - SF != OF || ZF == 1
#define V86_JGT                 0x16    // JGT r/m      - ZF == 0 || SF == OF
#define V86_JGE                 0x17    // JGE r/m      - SF == OF
#define V86_CALL                0x18    // CALL r/m
#define V86_RET                 0x19    // RET i
#define V86_LDR                 0x1a    // LDR i/m, r
#define V86_STR                 0x1b    // STR r, i/m
#define V86_PUSH                0x1c    // PUSH r/i
#define V86_PUSHA               0x1d    // PUSHA m
#define V86_POP                 0x1e    // POP r
#define V86_POPA                0x1f    // POPA
#define V86_IN                  0x20    // IN r/i, r
#define V86_OUT                 0x21    // OUT r, r/i
#define V86_HLT                 0x22    // HLT
/* codes through 0x3f reserved for future use */
#define V86_OPERATIONS          0x23
#define V86_MAX_OPERATIONS      64      // maximum # of operations supported

/* operand type flags; 0 for register */
#define V86_IMMEDIATE_OPERAND    (1 << 0) // immediate
/* operand/addressing mode flag-bits */
#define V86_SOURCE_REGISTER      (1 << 0)
#define V86_DESTINATION_REGISTER (1 << 1)
#define V86_DIRECT_ADDRESS       (1 << 2) // direct addressing; ldr adr, %r
#define V86_INDIRECT_ADDRESS     (1 << 3) // indirect addressing; ldr *adr, %r
#define V86_INDEXED_ADDRESS      (1 << 4) // indexed addressing; ldr ofs(%r), %r
#define V86_ARGUMENT_MASK                                               \
    (V86_DIRECT_ADDRESS | V86_INDIRECT_ADDRESS | V86_INDEXED_ADDRESS)

#define v86opisnop(op) (!(op)->code)
#define v86opsetnop(op) (*(uint32_t *)op = UINT32_C(0))
/* opcode format; 32 bits + possible 32-bit argument */
struct v86op {
    unsigned int unit   : 4;    // processor unit
    unsigned int mode   : 1;    // unit mode; 32/64-bit, float/rational, ...
    unsigned int code   : 6;    // operation ID
    unsigned int opsize : 2;    // operand size is 8 << opsize (max 512-bit)
    unsigned int opflg  : 5;    // operation flags
    unsigned int sreg   : 3;    // source register ID
    unsigned int dreg   : 3;    // destination register ID
    unsigned int imm8   : 8;    // 8-bit immediate offset, constant, or port
    v86word      arg[EMPTY];    // possible 32-bit constant, address, or offset
};
#if 0
struct v86op {
    unsigned int code   : 6;    // operation ID
    unsigned int opsize : 2;    // operand size is 8 << opsize
    unsigned int oper   : 4;    // operand type
    unsigned int sreg   : 2;    // source register ID
    unsigned int dreg   : 2;    // destination register ID
    unsigned int imm    : 16;   // 16-bit immediate offset, constant, or port
    v86word      arg[EMPTY];    // possible 32-bit constant, address, or offset
};
#endif

/* pseudo-machine parameters */

#define V86_WORD_SIZE           4  // in bytes
#define V86_CACHELINE_SIZE      16 // in bytes
#define V86_CACHELINE_WORDS     (V86_CACHELINE_SIZE / V86_WORD_SIZE)

/* user-accessible general-purpose register IDs */
#define V86_R0_REGISTER         0x00    // general purpose register R9
#define V86_R1_REGISTER         0x01    // general purpose register R1
#define V86_R2_REGISTER         0x02    // general purpose register R2
#define V86_R3_REGISTER         0x03    // general purpose register R3
#define V86_R4_REGISTER         0x04    // general purpose register R4
#define V86_R5_REGISTER         0x05    // general purpose register R5
#define V86_R6_REGISTER         0x06    // general purpose register R6
#define V86_R7_REGISTER         0x07    // general purpose register R7
#define V86_USER_REGISTERS      8       // # of user-accessible registers
/* system register IDs; only accessible by system-mode code */
#define V86_PC_REGISTER         0x08    // program counter; instruction pointer
#define V86_FP_REGISTER         0x09    // frame pointer
#define V86_SP_REGISTER         0x0a    // stack pointer
#define V86_MSW_REGISTER        0x1b    // machine status word
#define V86_LINK_REGISTER       0x1c    // link/return address
#define V86_DEBUG_REGISTER      0x1d    // link/return address
#define V86_CONTROL_REGISTER    0x1e    // feature control etc. bits
#define V86_EXCEPTION_REGISTER  0x1f    // bitmap for pending exceptions
#define V86_SYSTEM_REGISTERS    8
#if defined(V86_VM_SUPPORT) && (V86_VM_SUPPORT)
/* page-table entry bits */
#define V86_PTE_USER_BIT        (1 << 0)
#define V86_PTE_PRESENT_BIT     (1 << 1)
#define V86_PTE_EXEC_BIT        (1 << 2)
#define V86_PTE_NOCACHE_BIT     (1 << 3)
#endif
struct v86vm {
    v86reg    usrregs[V86_USER_REGISTERS];
    v86reg    sysregs[V86_SYSTEM_REGISTERS];
#if defined(V86_VM_SUPPORT) && (V86_VM_SUPPORT)
    v86adr   *pagetab;
#else
    uint8_t  *mem;
#endif
    uint64_t  tsc;                      // timestamp/cycle counter [global]
} ALIGNED(PAGESIZE);

#define V86_NO_EXCEPTION        0x00    // no exception specified
#define V86_INST_EXCEPTION      0x01    // illegal instruction
#define V86_MEM_EXCEPTION       0x02    // illegal memory reference
/* inst-exception reason-bits */
#define V86_INST_OPCODE         0x01    // illegal opcode
#define V86_INST_ARG1           0x02    // illegal argument/operand #1
#define V86_INST_ARG2           0x04    // illegal argument/operand #2
#define V86_INST_BREAK          0x08    // breakpoint
#define V86_INST_NOREAD         0x10    // non-readable memory location
#define V86_INST_NOEXEC         0x20    // non-executable memory location
/* [missing] permission bits for perm-field */
#define V86_PERM_USER           0x01
#define V86_PERM_USER_READ      0x02
#define V86_PERM_USER_WRITE     0x04
#define V86_PERM_USER_EXEC      0x08
#define V86_PERM_SYS_READ       0x10
#define V86_PERM_SYS_WRITE      0x20
#define V86_PERM_SYS_EXEC       0x40
struct v86vmxcpt {
    uint8_t type;       // 0 for not specified; otherwise, ID == type - 1
    uint8_t reason;     // error-codes based on exception type
    uint8_t perm;       // violated permission bits
    uint8_t unit;       // processor unit ID
};

#endif /* __V86_VM_H__ */

