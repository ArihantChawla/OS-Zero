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
#define V86_JMP              0x0d       // JMP r/m      - unconditional
#define V86_JZ               0x0e       // JE r/m       - ZF == 1
#define V86_JE               V86_JZ     // JZ r/m       - ZF == 1
#define V86_JNZ              0x0f       // JNE r/m      - ZF == 0
#define V86_JNE              V86_JNZ    // JNZ r/m      - ZF == 0
#define V86_JC               0x10       // JC r/m       - CF == 1
#define V86_JLT              V86_JC     // JLT r/m      - CF == 1
#define V86_JNC              0x11       // JNC r/m      - CF == 0
#define V86_JO               0x12       // JO r/m       - OF == 1
#define V86_JNO              0x13       // JNO r/m      - OF == 0
#define V86_JLE              0x14       // JLE r/m      - SF != OF || ZF == 1
#define V86_JGT              0x15       // JGT r/m      - ZF == 0 || SF == OF
#define V86_JGE              0x16       // JGE r/m      - SF == OF
#define V86_CALL             0x17       // CALL r/m
#define V86_RET              0x18       // RET i
#define V86_LDR              0x19       // LDR i/m, r
#define V86_STR              0x1a       // STR r, i/m
#define V86_PUSH             0x1b       // PUSH r/i
#define V86_PUSHA            0x1c       // PUSHA m
#define V86_POP              0x1d       // POP r
#define V86_POPA             0x1e       // POPA
#define V86_IN               0x1f       // IN r/i, r
#define V86_OUT              0x20       // OUT r, r/i
#define V86_HLT              0x21       // HLT
/* codes through 0x3f reserved for future use */
#define V86_OPERATIONS       0x22
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
#define V86_AX_REGISTER       0x00      // general purpose register AX
#define V86_BX_REGISTER       0x01      // general purpose register BX
#define V86_CX_REGISTER       0x02      // general purpose register CX
#define V86_DX_REGISTER       0x03      // general purpose register DX
#define V86_USER_REGISTERS    4         // # of user-accessible registers
/* system register IDs */
#define V86_PC_REGISTER       0x04      // program counter (instruction pointer)
#define V86_FP_REGISTER       0x05      // frame pointer
#define V86_SP_REGISTER       0x06      // stack pointer
#define V86_MSW_REGISTER      0x07      // machine status word
#define V86_REGISTERS         8

/* opcode format; 32 bits + possible 32-bit argument */
struct v86op {
    unsigned int code   : 6;    // operation ID
    unsigned int opsize : 2;    // operand size is 8 << opsize
    unsigned int oper   : 4;    // operand type
    unsigned int sreg   : 2;    // source register ID
    unsigned int dreg   : 2;    // destination register ID
    unsigned int imm    : 16;   // 16-bit immediate offset or constant
    int32_t      arg[EMPTY];    // possible 32-bit constant, address, or offset
};

#endif /* __V86_V86_H__ */

