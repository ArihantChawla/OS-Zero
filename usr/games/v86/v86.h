#ifndef __V86_V86_H__
#define __V86_V86_H__

#include <stdint.h>
#include <zero/cdefs.h>

/* pseudo-machine parameters */

#define V86_OPERAND_SIZE     32 // in bits
#define V86_CACHELINE_SIZE   16 // in bytes

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
#define V86_JLT              0x10
#define V86_JLE              0x11
#define V86_JGT              0x12
#define V86_JGE              0x13
#define V86_JC               0x14
#define V86_JNC              0x15
#define V86_JO               0x16
#define V86_JNO              0x17
#define V86_CALL             0x18
#define V86_RET              0x19
#define V86_LDR              0x1a
#define V86_STR              0x1b
#define V86_PUSH             0x1c
#define V86_PUSHA            0x1d
#define V86_POP              0x1e
#define V86_POPA             0x1f
#define V86_IN               0x20
#define V86_OUT              0x21
#define V86_HLT              0x22
/* codes through 0x3f reserved for future use */
#define V86_MAX_OPERATIONS   64         // maximum # of operations supported

/* operand type flags; 0 for register */
#define V86_CONSTANT_OPERAND (1 << 0)   // constant
/* addressing mode flags */
#define V86_DIRECT_ADDRESS   (1 << 1)   // direct addressing; ldr adr, %r
#define V86_INDIRECT_ADDRESS (1 << 2)   // indirect addressing; ldr *%ar, %r
#define V86_INDEXED_ADDRESS  (1 << 3)   // indexed addressing; ldr ofs(%ar), %r
#define V86_IMMEDIATE_MASK                                              \
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
    unsigned int code   : 6;    // operation ID
    unsigned int opsize : 2;    // operand size is 8 << opsize
    unsigned int oper   : 4;    // operand type
    unsigned int sreg   : 2;    // source register ID
    unsigned int dreg   : 2;    // destination register ID
    unsigned int imm    : 16;   // 16-bit immediate offset or constant
    uint32_t     arg[EMPTY];    // possible 32-bit constant, address, or offset
};

/* INSTRUCTION TIMINGS (prof) */

/* REFERENCE: http://www.agner.org/optimize/instruction_tables.pdf */
/* instruction timings in cycles (as listed for pentium [mmx] in REFERENCE */

/*
 * MEMORY
 * ------
 * - cacheline fetch: 4 + CLSIZE clock cycles (size on 32-bit words)
 * - instruction fetch: 1 clock cycle
 * - instruction decode: 1 clock cycle
 * - immediate operand fetch: 2 clock cycles
 * - memory read/write: 4 clock cycles
 *
 */
/* NOP
 * ---
 * - 1 clock cycle
 */
/* NOT
 * ---
 * - 2 clock cycles
 */
/* AND, OR, XOR
 * ------------
 * - 2 clock cycles+ operands
 */
/* SHR, SHL, SAR, ROR, ROL
 * -----------------------
 * - 3 clock cycles + 1 for immediate operand
 */
/* INC, DEC
 * --------
 * - 2 clock cycles
 */
/* ADD, SUB
 * --------
 * - 2 clock cycles if both are registers
 * - 3 clock cycles with immediate operand
 */
/* CMP
 * ---
 * - 2 clock cycles if both are registers
 * - 3 clock cycles with immediate operand
 */
/* MUL
 * ---
 * - 8 clock cycles
 */
/* DIV
 * ---
 * - 32 clock cycles
 */
/* LDA, STA
 * --------
 * - src and dest are registers: 2
 * - src/dest is immediate operand: 3
 * - src/dest is a memory operand: 4
 */
/* PUSH, POP
 * ---------
 * - 4 cycles if src/dest is a register
 */
/* PUSHA, POPA
 * - 4 cycles + 1 cycle per register
 */
/*
 * JMP, branches
 * -------------
 * - 8 clock cycles for the jump
 */
/*
 * IN, OUT
 * - 8 clock cycles
 */

#endif /* __V86_V86_H__ */

