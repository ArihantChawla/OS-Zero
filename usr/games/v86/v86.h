#ifndef __V86_V86_H__
#define __V86_V86_H__

#include <stdint.h>
#include <zero/cdefs.h>

/* pseudo-machine parameters */

#define V86_OPERAND_SIZE     32

/* pseudo-machine types */

#if (V86_OPERAND_SIZE == 32)
typedef int32_t  v86word;
typedef uint32_t v86uword;
typedef uint32_t v86adr;
#endif

/* instruction set */

#define V86_NOP              0x00
#define V86_NOT              0x01
#define V86_AND              0x02
#define V86_OR               0x03
#define V86_XOR              0x04
#define V86_SHL              0x05
#define V86_SHR              0x06
#define V86_SAR              0x07
#define V86_ADD              0x08
#define V86_SUB              0x09
#define V86_CMP              0x0a
#define V86_MUL              0x0b
#define V86_DIV              0x0c
#define V86_JMP              0x0d
#define V86_JE               0x0e
#define V86_JNE              0x0f
#define V86_JB               0x10
#define V86_JLT              0x11
#define V86_JLE              0x12
#define V86_JGT              0x13
#define V86_JGE              0x14
#define V86_JC               0x15
#define V86_JNC              0x16
#define V86_JO               0x17
#define V86_JNO              0x18
#define V86_CALL             0x19
#define V86_RET              0x1a
#define V86_LDR              0x1b
#define V86_STR              0x1c
#define V86_PUSH             0x1d
#define V86_PUSHA            0x1e
#define V86_POP              0x1f
#define V86_POPA             0x20
#define V86_IN               0x21
#define V86_OUT              0x22
#define V86_HLT              0x23
/* codes through 0x3f reserved for future use */
#define V86_MAX_OPERATIONS   64         // maximum # of operations supported

/* operand type flags */
#define V86_REGISTER_OPERAND (1 << 0)   // register
#define V86_CONSTANT_OPERAND (1 << 1)   // constant
/* addressing mode flags */
#define V86_DIRECT_ADDRESS   (1 << 2)   // direct addressing; ldr adr, %r
#define V86_INDIRECT_ADDRESS (1 << 3)   // indirect addressing; ldr *%ar, %r
#define V86_INDEXED_ADDRESS  (1 << 4)   // indexed addressing; ldr ofs(%ar), %r
#define V86_ARGUMENT_MASK                                               \
    (V86_CONSTANT_OPERAND | V86_INDIRECT_ADDRESS | V86_INDEXED_ADDRESS)

/* user-accessible register IDs */
#define V86_AX_REGISTER      0x00       // general purpose register AX
#define V86_BX_REGISTER      0x01       // general purpose register BX
#define V86_CX_REGISTER      0x02       // general purpose register CX
#define V86_DX_REGISTER      0x03       // general purpose register DX
#define V86_USER_REGISTERS   4          // # of user-accessible registers
/* system register IDs */
#define V86_PC_REGISTER      0x04       // program counter (instruction pointer)
#define V86_FP_REGISTER      0x05       // frame pointer
#define V86_SP_REGISTER      0x06       // stack pointer
#define V86_MSW_REGISTER     0x07       // machine status word
#define V86_REGISTERS        8

/* opcode format; 32 bits + possible 32-bit argument */
struct v86op {
    unsigned int code : 6;      // operation ID
    unsigned int oper : 5;      // operand type
    unsigned int sreg : 2;      // source register ID
    unsigned int dreg : 2;      // destination register ID
    unsigned int _res : 1;      // reserved bits for future extension
    unsigned int imm  : 16;     // 16-bit immediate offset or constant
    uint32_t     arg[EMPTY];    // possible 32-bit constant, address, or offset
};

#endif /* __V86_V86_H__ */

