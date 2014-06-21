#ifndef __ZPU_ZPU_H__
#define __ZPU_ZPU_H__

#include <stdint.h>

#include <zero/cdecl.h>
#include <zero/param.h>
#include <zpu/conf.h>

#define ZPUNREG     16
#define ZPUNGENREG  12
#define ZPUFPREG    12
#define ZPUSPREG    13
#define ZPUPCREG    14
#define ZPUMSWREG   15
#define ZPUZEROREG  0x00

/* 4-bit unit ID */
#define OP_LOGIC    0x00
#define OP_SHIFT    0x01
#define OP_ARITH    0x02
#define OP_COMPAR   0x03
#define OP_STACK    0x04
#define OP_LDSTR    0x05
#define OP_FLOW     0x06
#define OP_SREG     0x07
#define OP_FPU      0x08
#define ZPUNUNITBIT 4
#define ZPUNUNIT    (1U << ZPUNUNITBIT)

/* logical bitwise instructions */
#define OP_NOT      0x01
#define OP_AND      0x02
#define OP_OR       0x03
#define OP_XOR      0x04

/* shift instructions */
#define OP_SHR      0x05
#define OP_SAR      0x06
#define OP_SHL      0x07
#define OP_ROR      0x08
#define OP_ROL      0x09

/* arithmetical instructions */
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
#define OP_BNO      0x19
#define OP_BC       0x1a
#define OP_BNC      0x1b

/* stack operations */
#define OP_POP      0x1c
#define OP_PUSH     0x1d
#define OP_PUSHA    0x1e

/* load/store operations */
#define OP_MOV      0x1f
#if 0
#define OP_MOVB     0x20
#define OP_MOVW     0x21
#define OP_MOVQ     0x22
#endif

/* flow control */
#define OP_JMP      0x20
#define OP_CALL     0x21
#define OP_ENTER    0x22
#define OP_LEAVE    0x23
#define OP_RET      0x24

/* machine status word */
#define OP_LMSW     0x25
#define OP_SMSW     0x26

/* maximum number of instructions supported */
#define ZPUNOP      (1U << ZPUNOPBIT)
#define ZPUNOPBIT   7
#define ZPUNINSTBIT (ZPUNUNITBIT + ZPUNOPBIT)
#define ZPUNINST    (1U << (ZPUNINSTBIT))

/* invalid instruction PC return value */
#define OP_INVAL    0xffffffffU

/* TODO: FPU instruction set with trigonometry etc. */
/* SIN, COS, TAN, ASIN, ACOS, ATAN, SINH, COSH, TANH, LOG, POW, ... */

/* machine status long-word */
#define MSW_CF      0x01
#define MSW_IF      0x02
#define MSW_VF      0x04
#define MSW_ZF      0x08
#define MSWNBIT     4

#define ARG_INDIR   0x01        // indirect addressing
#define ARG_INDEX   0x02        // indexed addressing
#define ARG_IMMED   0x04        // immediate argument
#define ARG_ADR     0x08        // address argument
#define ARG_REG     0x10        // register argument

/* opcode is 32-bit with extra arguments following where necessary */
#define zpusetinst(id, unit, func)                                      \
    (zpuopfunctab[((unit) << ZPUNOPBIT) + (id)] = (func))
struct zpuop {
    unsigned unit    : 4;       // unit ID
    unsigned inst    : 7;       // numerical instruction ID
    unsigned sflg    : 5;       // INDIR, INDEX, IMMED, ADR, REG
    unsigned src     : 4;       // 4-bit source register ID
    unsigned dflg    : 5;       // INDIR, INDEX, IMMED, ADR, REG
    unsigned dest    : 4;       // 4-bit destination register
    unsigned argsz   : 3;       // operation size is 1 << (opsize + 1)
    int32_t  args[EMPTY];       // optional arguments
} PACK();

struct zpurat {
    int32_t nom;
    int32_t denom;
};

#define zpuchkintr(zpu, i) ((zpu)->intrmask & (1U << (i)))
#define zpusetintr(zpu, i) ((zpu)->intrmask |= 1U << (i))
struct zpuctx {
    int64_t  regs[ZPUNREG];     // register values
#if 0
    uint32_t fp;                // frame pointer
    uint32_t sp;                // stack pointer
    uint32_t pc;                // program counter ("instruction pointer")
    int32_t  msw;               // machine status long-word
#endif
    uint32_t intrmask;          // mask of pending interrupts
} PACK();

typedef void  ZPUOPRET;
struct zpu;
typedef ZPUOPRET zpuopfunc(struct zpu *, struct zpuop *);
struct zpu {
    zpuopfunc     **functab;
    struct zpuctx   ctx;
    uint8_t        *core;
    uint8_t         exitflg;
};

static __inline__ void
zpusetmsw(struct zpu *zpu, int64_t val)
{
    long msw = 0;

    if (val & INT64_C(0xffffffff00000000)) {
        msw |= (MSW_CF | MSW_VF);
    } else if (!val) {
        msw |= MSW_ZF;
    }
    zpu->ctx.regs[ZPUMSWREG] |= msw;

    return;
}

#endif /* __ZPU_ZPU_H__ */

