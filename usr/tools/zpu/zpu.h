#ifndef __ZPU_ZPU_H__
#define __ZPU_ZPU_H__

#include <zero/cdecl.h>
#include <zero/param.h>
#include <zpu/conf.h>

#define ZPUNREG     8
#define ZPUZEROREG  0x00

/* logical bitwise instructions */
#define OP_NOT      0x01
#define OP_AND      0x02
#define OP_OR       0x03
#define OP_XOR      0x04

/* shift instructions */
#define OP_SHR      0x05
#define OP_SHRA     0x06
#define OP_SHL      0x07
#define OP_ROR      0x08
#define OP_ROL      0x09

/* ALU instructions */
#define OP_INC      0x0a
#define OP_DEC      0x0b
#define OP_ADD      0x0c
#define OP_SUB      0x0d
#define OP_CMP      0x0e
#define OP_MUL      0x0f
#define OP_DIV      0x10
#define OP_MOD      0x11

/* comparative instructions */
#define OP_BZ       0x12
#define OP_BNZ      0x13
#define OP_BLT      0x14
#define OP_BLE      0x15
#define OP_BGT      0x16
#define OP_BGE      0x17
#define OP_BO       0x18
#define OP_BC       0x19
#define OP_BNC      0x1a

/* stack operations */
#define OP_POP      0x1b
#define OP_PUSH     0x1c
#define OP_PUSHA    0x1d

/* load/store operations */
#define OP_MOV      0x1e
#define OP_MOVB     0x1f
#define OP_MOVW     0x20

/* flow control */
#define OP_JMP      0x21
#define OP_CALL     0x22
#define OP_ENTER    0x23
#define OP_LEAVE    0x24
#define OP_RET      0x25

/* machine status word */
#define OP_LMSW     0x26
#define OP_SMSW     0x27

/* floating point unit intstruction flag */
#define OP_FPU      0x40

/* maximum number of instructions supported */
#define ZPUNINST    128

/* invalid instruction PC return value */
#define OP_INVAL    0xffffffffU

/* TODO: FPU instruction set with trigonometry etc. */
/* SIN, COS, TAN, ASIN, ACOS, ATAN, SINH, COSH, TANH, LOG, POW, ... */

/* machine status long-word */
#define MSW_CF      0x01
#define MSW_IF      0x02
#define MSW_VF      0x04
#define MSW_ZF      0x08

#define REG_INDIR   0x01        // indirect addressing
#define REG_INDEX   0x02        // indirect addressing
#define REG_DOUBLE  0x04        // 1 if 64-bit IEEE double

#define ARG_NONE    0x00        // argument not present
#define ARG_IMMED   0x01        // immediate argument
#define ARG_ADR     0x02        // address argument
#define ARG_REG     0x03        // register argument
struct zpuop {
    unsigned inst    : 7;       // numerical instruction ID
    unsigned group   : 3;       // instruction group
    unsigned src     : 6;       // 3-bit register ID + INDIR + INDEX + DOUBLE
    unsigned dest    : 6;       // similar to src
    unsigned arg1    : 2;       // NONE, IMMED, ADR, REG
    unsigned arg2    : 2;       // NONE, IMMED, ADR, REG
    unsigned immed   : 6;       // small immediate constant like MSW flags
    int64_t  args[EMPTY];       // optional arguments
} PACK();

struct zpurat {
    int32_t nom;
    int32_t denom;
};

struct zpuctx {
    int64_t  regs[ZPUNREG];     // register values
    uint32_t fp;                // frame pointer
    uint32_t sp;                // stack pointer
    uint32_t pc;                // program counter ("instruction pointer")
    int32_t  msw;               // machine status long-word
} PACK();

typedef uint32_t zpuinstfunc(struct zpuop *);

#endif /* __ZPU_ZPU_H__ */

