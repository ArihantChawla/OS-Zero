#ifndef __V86_V86_H__
#define __V86_V86_H__

#include <stdint.h>
#include <zero/cdefs.h>

/* pseudo-machine parameters */

#define V86_WORD_SIZE        4  // in bytes
#define V86_CACHELINE_SIZE   16 // in bytes
#define V86_CACHELINE_WORDS  (V86_CACHELINE_SIZE / V86_WORD_SIZE)

/* pseudo-machine types */

#if (V86_WORD_SIZE == 32)
typedef int32_t  v86word;
typedef uint32_t v86uword;
typedef uint32_t v86adr;
#endif

/* instruction set */

#define V86_NOP              0x00       // no operation
#define V86_NOT              0x01       // NOT r/i, r
#define V86_AND              0x02       // AND r/i, r
#define V86_OR               0x03       // OR r/i, r
#define V86_XOR              0x04       // XOR r/i, r
#define V86_SHL              0x05       // SHL r/i, r
#define V86_SHR              0x06       // SHR r/i, r
#define V86_SAR              0x07       // SAR r/i, r
#define V86_ADD              0x08       // ADD r/i, r
#define V86_SUB              0x09       // SUB r/i, r
#define V86_CMP              0x0a       // CMP r/i, r - set flags
#define V86_MUL              0x0b       // MUL r/i, r
#define V86_DIV              0x0c       // DIV r/i, r
#define V86_JMP              0x0d       // JMP i8       - short jump with offset
#define V86_LJMP             0x0e       // LJMP r/m     - long jump
#define V86_JZ               0x0f       // JE r/m       - ZF == 1
#define V86_JE               V86_JZ     // JZ r/m       - ZF == 1
#define V86_JNZ              0x10       // JNE r/m      - ZF == 0
#define V86_JNE              V86_JNZ    // JNZ r/m      - ZF == 0
#define V86_JC               0x11       // JC r/m       - CF == 1
#define V86_JLT              V86_JC     // JLT r/m      - CF == 1
#define V86_JNC              0x12       // JNC r/m      - CF == 0
#define V86_JO               0x13       // JO r/m       - OF == 1
#define V86_JNO              0x14       // JNO r/m      - OF == 0
#define V86_JLE              0x15       // JLE r/m      - SF != OF || ZF == 1
#define V86_JGT              0x16       // JGT r/m      - ZF == 0 || SF == OF
#define V86_JGE              0x17       // JGE r/m      - SF == OF
#define V86_CALL             0x18       // CALL r/m
#define V86_RET              0x19       // RET i
#define V86_LDR              0x1a       // LDR i/m, r
#define V86_STR              0x1b       // STR r, i/m
#define V86_PUSH             0x1c       // PUSH r/i
#define V86_PUSHA            0x1d       // PUSHA m
#define V86_POP              0x1e       // POP r
#define V86_POPA             0x1f       // POPA
#define V86_IN               0x20       // IN r/i, r
#define V86_OUT              0x21       // OUT r, r/i
#define V86_HLT              0x22       // HLT
/* codes through 0x3f reserved for future use */
#define V86_OPERATIONS       0x23
#define V86_MAX_OPERATIONS   64         // maximum # of operations supported

/* operand type flags; 0 for register */
#define V86_IMMEDIATE_OPERAND (1 << 0)  // immediate
/* addressing mode flags */
#define V86_DIRECT_ADDRESS    (1 << 1)  // direct addressing; ldr adr, %r
#define V86_INDIRECT_ADDRESS  (1 << 2)  // indirect addressing; ldr *%ar, %r
#define V86_INDEXED_ADDRESS   (1 << 3)  // indexed addressing; ldr ofs(%ar), %r
#define V86_ARGUMENT_MASK                                               \
    (V86_IMMEDIATE_OPERAND | V86_INDIRECT_ADDRESS | V86_INDEXED_ADDRESS)

/* user-accessible register IDs */
#define V86_R0_REGISTER       0x00      // general purpose register R9
#define V86_R1_REGISTER       0x01      // general purpose register R1
#define V86_R2_REGISTER       0x02      // general purpose register R2
#define V86_R3_REGISTER       0x03      // general purpose register R3
#define V86_R4_REGISTER       0x04      // general purpose register R4
#define V86_R5_REGISTER       0x05      // general purpose register R5
#define V86_R6_REGISTER       0x06      // general purpose register R6
#define V86_R7_REGISTER       0x07      // general purpose register R7

#define V86_USER_REGISTERS    8         // # of user-accessible registers
/* system register IDs */
#define V86_PC_REGISTER       0x04      // program counter (instruction pointer)
#define V86_FP_REGISTER       0x05      // frame pointer
#define V86_SP_REGISTER       0x06      // stack pointer
#define V86_MSW_REGISTER      0x07      // machine status word
#define V86_REGISTERS         8

/* opcode format; 32 bits + possible 32-bit argument */
struct v86op {
    unsigned int unit   : 4;    // processor unit
    unsigned int mode   : 1;    // unit mode; 32/64-bit, float/rational, ...
    unsigned int code   : 6;    // operation ID
    unsigned int opsize : 3;    // operand size is 8 << opsize (max 1024-bit)
    unsigned int opflg  : 4;    // operand type flag-bits
    unsigned int sreg   : 3;    // source register ID
    unsigned int dreg   : 3;    // destination register ID
    unsigned int imm    : 8;    // 8-bit immediate offset, constant, or port
    int32_t      arg[EMPTY];    // possible 32-bit constant, address, or offset
};
#if 0
struct v86op {
    unsigned int code   : 6;    // operation ID
    unsigned int opsize : 2;    // operand size is 8 << opsize
    unsigned int oper   : 4;    // operand type
    unsigned int sreg   : 2;    // source register ID
    unsigned int dreg   : 2;    // destination register ID
    unsigned int imm    : 16;   // 16-bit immediate offset, constant, or port
    int32_t      arg[EMPTY];    // possible 32-bit constant, address, or offset
};
#endif

#endif /* __V86_V86_H__ */

