#ifndef __ZEN_OP_H__
#define __ZEN_OP_H__

#include <limits.h>
#include <zen/zen.h>
#include <zen/vm.h>

/* operation dispatch function */

#define ZEN_REG_BIT   (1 << 0)
#define ZEN_A_BIT     (1 << 1)
#define ZEN_X_BIT     (1 << 2)
#define ZEN_K_BIT     (1 << 3)
#define ZEN_MEM_BIT   (1 << 4)
#define ZEN_LABEL_BIT (1 << 5)
#define ZEN_PKT_BIT   (1 << 6)
#define ZEN_OFS_BIT   (1 << 7)
#define ZEN_LEN_BIT   (1 << 8)
#define ZEN_MUL4_BIT  (1 << 9)
#define ZEN_JMPT_BIT  (1 << 10)
#define ZEN_JMP2_BIT  (1 << 11)
typedef uint32_t zenopargbits_t;

#define ZEN_OP_FAILURE     (-1L)
typedef zenword_t zenopfunc(struct zenvm *vm, zenword_t src, zenword_t dest);
/*
 * addressing modes
 * ----------------
 *
 * addressing mode      syntax          description
 *
 * REG + REG_X_BIT      x/%x            X-register
 * REG + REG_A_BIT      a/%a            accumulator A
 * IMMED                #k              literal value stored in k
 * MEM                  M[k]            word at offset k in M[]
 * PKT                  [k]             BHW at byte offset k in the packet
 * PKT + OFS_BIT        [x + k]         BHW at the offset x + k in the packet
 * PKT + MUL4_BIT       4*([k]&0xf)     low 4 bits * 4 at byte offset k in pkt
 * SYM                  L               jump label L
 * JMP + JMP2_BIT       #k,Lt,Lf        jump to Lt if true, otherwise jump to Lf
 * JMP + JMPT_BIT       #k,Lt           jump to Lt if predicate is true
 * EXT                 extension        BPF
 extension
 */
/* 8-bit instruction ID in low 8 bits of op->code */
/* bits 28..31 in op->k */
/*
 * instruction set
 *
 * instruction  addressing mode         description
 *
 * ALU instructions
 * ----------------
 * add              0+REG_X,1            A + <x>
 * sub              0+REG_X,1            A - <x>
 * mul              0+REG_X,1            A * <x>
 * div              0+REG_X,1            A / <x>
 * mod              0+REG_X,1            A % <x>
 * and              0+REG_X,1            A & <x>
 * or               0+REG_X,1            A | <x>
 * xor              0+REG_X,1            A ^ <x>
 * lsh              0+REG_X,1            A << <x>
 * rsh              0+REG_X,1            A >> <x>
 * neg              0+REG_X,1            !A
*
 * load instructions
 * -----------------
 * ldb              3,3+OFS              load byte into A
 * ldh              3,3+OFS              load half-word into A
 * ldi              1                    load word into A
 * ld               1,2,3,3+OFS,EXT      load word into A
 * ldx              1,2,3+MUL4,EXT       load word into X
 * ldxi             1                    load word into X
 * ldxb             3+MUL4               load byte into X
 *
 * store instructions
 * ------------------
 * st               2                    store A into M[]
 * stx              2                    store X into M[]
 *
 * branch instructions
 * -------------------
 * jmp              4                    jump to label
 * ja               4                    jump to label
 * jeq              6+JMP2,6+TRUE        jump on k == A
 * jneq             6+TRUE               jump on k != A
 * jne              6+TRUE               jump on k != A
 * jlt              6+TRUE               jump on k < A
 * jle              6+TRUE               jump on k <= A
 * jgt              6+JMP2,6+TRUE        jump on k > A
 * jge              6+JMP2,6+TRUE        jump on k >= A
 * jset             6+JMP2,6+TRUE        jump on k & A
 *
 * return instructions
 * -------------------
 * ret              1,0+REG_A            return
 *
 * miscellaneous instructions
 * --------------------------
 * tax              N/A                  copy A into X
 * txa              N/A                  copy X into A
 */

/* first 8 addressing modes can be encoded in op->code */
/* argument types */
#define ZEN_REG_ARG         0   // source and destination registers
#define ZEN_IMMED_ARG       1   // immediate argument in k-field
#define ZEN_MEM_ARG         2   // 4-bit acratch memory area offset   
#define ZEN_PKT_ARG         3   // address relative to packet
#define ZEN_SYM_ARG         4   // symbol [label] address for branches
#define ZEN_JMP_ARG         5   // branch offset from pc for true predicate
#define ZEN_BIG_WORD_ARG    6   // full 32-bit word without flag-bits in k
#define ZEN_LEN_ARG         7   // packet length argument
#define ZEN_OP_ARG_NBIT     8   // max # of argument-bits in opcode */
#define ZEN_ARG_TYPE_NBIT   3   // room reserved for future extensions
#defin4 ZEN_ARG_NTYPE       8
#define ZEN_BIG_WORD_NBIT   32  // full 32-bit k-field without flag-bits
#define ZEN_BIG_WORD_MASK   (0xffffffffU)
#define ZEN_BIG_WORD_MIN    INT32_MIN
#define ZEN_BIG_WORD_MAX    INT32_MAX
 // arguments in high bits of (vm)->k
#define ZEN_K_IMMED_NBIT    (32 - ZEN_K_ARG_NBIT)
#define ZEN_K_VAL_MASK      ((1UL << ZEN_K_ARG_OFS) - 1)
#define ZEN_OP_ARG_NONE     (((1L << ZEN_OP_ARG_NBIT) - 1) << ZEN_OP_ARG_OFS)
#define ZEN_K_ARG_NONE      (((1L << ZEN_K_ARG_NBIT) - 1) << ZEN_K_ARG_OFS)

/* ALU instructions */

/* instruction                  brief                   addressing modes */
#define ZEN_ALU_UNIT     0x00
#define ZEN_ADD          0x01   // A + <x>              0, 4
#define ZEN_SUB          0x02   // A - <x>              0, 4
#define ZEN_MUL          0x03   // A * <x>              0, 4
#define ZEN_DIV          0x04   // A / <x>              0, 4
#define ZEN_MOD          0x05   // A % <x>              0, 4
#define ZEN_AND          0x06   // A & <x>              0, 4
#define ZEN_OR           0x07   // A | <x>              0, 4
#define ZEN_XOR          0x08   // A ^ <x>              0, 4
#define ZEN_LSH          0x09   // A << <x>             0, 4
#define ZEN_RSH          0x0a   // A >> <x>             0, 4
#define ZEN_NEG          0x0b   // -A                   0, 4
#define ZEN_ALU_NOP      12     // # of table items for unit

/* memory instructions */

/* load instructions */
/* instruction                  brief                   addressing modes */
#define ZEN_REG_UNIT     0x01
#define ZEN_LDB          0x01   // load byte into A     1, 2
#define ZEN_LDH          0x02   // load halfword into A 1, 2
#define ZEN_LDI          0x03   // load word into A     4
#define ZEN_LD           0x04   // load word into A     1, 2, 3, 4, 10
#define ZEN_LDX          0x05   // load word into X     3, 4, 5, 10
#define ZEN_LDXI         0x06   // load word into X     4
#define ZEN_LDXB         0x07   // load byte into X     5
/* store instructions */
/* instruction                  brief                   addressing modes */
#define ZEN_ST           0x08   // store A into M[];    3
#define ZEN_STX          0x09   // store X into M[];    3
#define ZEN_REG_NOP      10     // # of table items for unit

/* control flow */

/* branch instructions */
/* instruction                  brief                   addressing modes */
#define ZEN_FLOW_UNIT    0x02
#define ZEN_JMP          0x01   // jump to label        6
#define ZEN_JA           0x02   // jump to label        6
#define ZEN_JEQ          0x03   // jump if k == A       7, 8
#define ZEN_JNE          0x04   // jump if k != A       8
#define ZEN_JNEQ         ZEN_JNE // jump if k != A       8
#define ZEN_JLT          0x05   // jump if k < A        8
#define ZEN_JLE          0x06   // jump if k <= A       8
#define ZEN_JGT          0x07   // jump if k > A        7, 8
#define ZEN_JGE          0x08   // jump if k >= A       7, 8
#define ZEN_JSET         0x09   // jump if k & A        7, 8
#define ZEN_RET          0x0a   // return               4, 9
#define ZEN_FLOW_NOP     11     // # of table items for unit

/* miscellaneous instructions */

/* register transfer */
/* instruction                  brief */
#define ZEN_XFER_UNIT    0x03
#define ZEN_TAX          0x01   // copy A into X
#define ZEN_TXA          0x02   // copy X into A
#define ZEN_XFER_NOP     3      // # of table items for unit

#define ZEN_NUNIT        4

/* operation decoding */

#define ZEN_UNIT_NOP     16
#define ZEN_OP_NBIT      16
#define ZEN_INST_NBIT    4      // per-unit instruction IDs
#define ZEN_UNIT_NBIT    2      // unit IDs
#define ZEN_OP_UNIT_OFS  ZEN_INST_NBIT
#define ZEN_INST_MSSK    ((1L << ZEN_INST_NBIT) - 1)
#define ZEN_UNIT_MSSK    ((1L << ZEN_UNIT_NBIT) - 1)
#define ZEN_OP_ARG_NBIT  (ZEN_OP_NBIT - ZEN_INST_NBIT - ZEN_UNIT_NBIT)
//#define ZEN_OP_ARG_NBIT  (16 - ZEN_OP_NBIT - ZEN_UNIT_NBIT)

#define ZEN_JMP_OFS_NBIT 8
#define ZEN_JMP_MASK     ((1L << ZEN_JMP_OFS_NBIT) - 1)
/* set operation true and false branch offsets; evaluate to (op)->jt */
#define zenputopjmpjt(op, jt, jf, k)                                    \
    ((op)->jt = (jt) & ZEN_JMP_MASK,                                    \
     (op)->jf = (jf) & ZEN_JMP_MASK,                                    \
     (op)->k = (k) & ZEN_K_VAL_MASK,                                    \
     (op)->jt)
/* set operation true and false branch offsets; evaluate to (op)->jf */
#define zenputopjmpjf(op, jt, jf, k)                                    \
    ((op)->jt = (jt) & ZEN_JMP_MASK,                                    \
     (op)->jf = (jf) & ZEN_JMP_MASK,                                    \
     (op)->k = (k) & ZEN_K_VAL_MASK,                                    \
     (op)->jf)
#define zengetjmptadr(vm, op) ((vm)->pc + (op)->jt)
#define zengetjmpfadr(vm, op) ((vm)->pc + (op)->jf)
#define zengetsymadr(vm, op)  (zengetadrbit((op)->k))
#define ZEN_K_NBIT    32

#define ZEN_UNIT_NOP        16

/*
 * code field format
 * -----------------
 * high 8 bits: addressing mode
 * next 4 bits: unit ID
 * low  4 bits: instruction ID
 */
/* instruction in bits [0, ZEN_OP_ARG_NLO_BIT - 1], low 8 bits in (op)->code */
#define ZEN_OP_INST_OFS     0
#define ZEN_OP_INST_NBIT    8
#define ZEN_OP_INST_MASK    ((1L << ZEN_OP_NINST_BIT) - 1)
/* unit ID in bits [ZEN_OP_UNIT_OFS, ZEN_OP_UNIT_OFS + ZEN_OP_UNIT_BITS - 1] */
#define ZEN_OP_UNIT_OFS     ZEN_OP_NINST_BIT
#define ZEN_OP_UNIT_NBIT    2
#define ZEN_OP_UNIT_MASK    (((1L << ZEN_OP_NUNIT_BIT) - 1)             \
                             << ZEN_OP_UNIT_OFS)
#define ZEN_UNIT_MASK       ((1L << ZEN_OP_UNIT_NBIT) - 1)
#define ZEN_OP_ARG_OFS      (ZEN_OP_UNIT_OFS + ZEN_OP_NUNIT_BIT)
#define ZEN_OP_ARG_MASK     ((1L << ZEN_OP_ARG_NBIT) - 1)
#define ZEN_K_ARG_NBIT      2
#define ZEN_K_ARG_OFS       (32 - ZEN_K_ARG_NBIT)
#define ZEN_K_ARG_MASK      ((1L << ZEN_K_ARG_NBIT) - 1)
#define ZEN_ARG_NONE         0
#define ZEN_ARG_NBIT        (16 - ZEN_OP_NBIT - ZEN_OP_UNIT_NBIT)
#define ZEN_K_ARG_MASK      ((1L << (ZEN_K_ARG_NBIT)) - 1)
#define zengetinst(op)      ((op)->code & ZEN_OP_INST_MASK)
#define zengetunit(op)      (((op)->code >> ZEN_UNIT_OFS) & ZEN_UNIT_MASK)
#define zengetoparg(op)     (((op)->code >> ZEN_OP_ARG_OFS)& ZEN_OP_ARG_MASK)
#define zengetargbit(op, i)                                             \
    (((i) < ZEN_OP_ARG_LO_NBIT)                                         \
     ? ((op)->code >> (ZEN_OP_ARG_OFS + (i)))                           \
     : ((op)->k >> (ZEN_K_ARG_OFS + (i) - ZEN_OP_ARG_NBIT)))
#define zengetkargbit(op, i)                                            \
    ((op)->k >> (ZEN_K_ARG_OFS + (i) - ZEN_OP_ARG_NBIT))
#define zenmergeargbits(op)                                             \
    ((((op)->code >> ZEN_OP_ARG_OFS) & ZEN_OP_ARG_MASK)                 \
     | ((op)->k >> (ZEN_K_ARG_OFS - ZEN_OP_ARG_NBIT)))
#if (LONG_SIZE == 8)
/* read opcode in one 64-bit word :) */
#define zenreadop(op, dest) (*((uint64_t *)dest) = *((uint64_t *)op))
#define zenopuninit(op)     (*((uint64_t *)op) == UINT64_C(0))
#else
/* read opcode in 2 32-bit words */
#define zenreadop(op, dest) (((uint32_t *)dest)[0] = *((uint32_t *)op),   \
                             ((uint32_t *)dest)[1] = (op)->k)
#define zenopuninit(op)     (!*((uint32_t *)(op)) && !(op)->k)
#endif
#define zeninstisok(u, i)    (bitset(&zenopmap[(u)][0], (i)))
#define zengetjtadr(vm, op)  ((vm)->jtadr = (vm)->pc + 1 + (op)->jt)
#define zengetjfadr(vm, op)  ((vm)->jfadr = (vm)->pc + 1 + (op)->jf)
struct zenop {
    uint16_t  code;     // bits 0..15: 4-bit OP, 4-bit UNIT, 8 argument bits
    uint8_t   jt;       // 8-bit offset from PC to true-branch
    uint8_t   jf;       // 8-bit offset rom PC to false-branch
    zenword_t k;        // signed 32-bit miscellaneous argument
};

struct zeninst {
    const char     *name;
    struct zeninst *chain;
    long            key;
    uint32_t        flg;
    uint8_t         unit;
    uint8_t         inst;
};

#define ZEN_VAR_TYPE_INT32 1    // signed 32-bit integer value
#define ZEN_VAR_TYPE_SYM   2    // sym in union u
#define ZEN_VAR_TYPE_JMP2  3    // 8-bit offsets for true and false branches
#define ZEN_VAR_TYPE_LOCK  4    // mutexes and such
struct zenvar {
    const char    *name;
    struct zenvar *chain;
    long           type;
    union {
        /* symbol structure pointer */
        struct zensym *sym;
        int32_t        val;     // arbitrary signed 32-bit value
        struct {
            uint8_t    tofs;    // true-predicate branch
            uint8_t    fofs;    // false-predicate branch
        } jmp;
    } u;
};

/* index values for accessing argument structures */
#define ZEN_ARG1_NDX        0
#define ZEN_ARG2_NDX        1
#define ZEN_ARG3_NDX        2

#define zengetargbyte(op, ndx)                                          \
    ((((op)->args.vals[(ndx)]) >> ((ndx) * ZEN_OP_ARG_NBIT)) & ZEN_OP_ARG_MASK)
         & ((1L << ZEN_OP_ARG_NBIT) - 1)))
#define zengetargbits(op, ndx)                                          \
    ((((op)->args.vals[(ndx)]) >> ((ndx) * ZEN_OP_ARG_NBIT)) & ZEN_OP_ARG_MASK)
#define zengetargbit(op, ndx, i)                                        \
    ((((op)->args.vals[(ndx)]) >> ((ndx) * ZEN_OP_ARG_NBIT + (i))) & 0x01)
#define zensetargbyte(op, ndx, i)                                       \
    ((op)->args.vals[(ndx)] = (i))
#define zensetargbits(op, ndx, i)                                       \
    ((op)->args.vals[(ndx)] |= (i))
#define zensetargbit(op, ndx, i)                                        \
    ((op)->args.vals[(ndx)] |= (1L << (i)))
#define zensetargbit(op, ndx, i)                                        \
    ((op)->args.vals[ndx] |= (1L << (i)))
#define zenclrargbit(op, ndx, i)                                        \
    ((op)->args.vals[ndx] &= ~(1L << (i)))
#define zenflipbit(op, ndx, i)                                          \
    ((op)->flg ^ (1L << (i)))
#define zensetflgbit(info, ndx, i)

#define zengetargtype(args, ndx, tptr)                               \
    
#define zengetarg(args, ndx, tptr)                                   \
    (((tptr))                                                        \
     ? (*(tptr) = zofgetargtype(args, ndx), (args)->vals[(ndx)])     \
     : ((args)->vals[(ndx)]))

#define ZEN_ARG_FLG_NBIT 8

struct zenopargs {
    /*
     * val[0], val[1], val[2]
     * ----------------
     * hi bits: REG, INMED, MEM, PKT, MUL4, SYM, TJMP, FJMP
     */
    zenword_t          type;    // argument bits or types for up to 3 arguments
    zenword_t          flg;     // per-argument flag bits
    zenword_t          vals[ZVM_ARG_MAX]; // arguments #1, #2, #3 with flags
    union {
        struct zensym *sym;     // target label for branches
        zenword_t      arg;     // label or branch address
        zenword_t      pc;      // program counter for relative addressing
        zenword_t      ofs;     // offset for certain addressing modes
    }
};

#define ZEN_NARG 3

#define zengetoparg(data, ndx) ((data)->args[(ndx)])
struct zenopdata {
    struct zenop     *op;
    struct zenopargs  args[ZEN_NARG];
};

struct zenopinfo {
    unsigned char   nospacemap[256 / CHAR_BIT];
    zenopfunc     **unitfuncs[ZEN_NUNIT];
    uint16_t      **unitargbits[ZEN_NUNIT];
    uint8_t        *unitopnargs[ZEN_NUNIT];
    zenopfunc      *alufunctab[ZEN_UNIT_NOP];
    zenopfunc      *memfunctab[ZEN_UNIT_NOP];
    zenopfunc      *flowfunctab[ZEN_UNIT_NOP];
    zenopfunc      *xferfunctab[ZEN_UNIT_NOP];
    uint16_t        aluargbits[ZEN_UNIT_NOP];
    uint16_t        memargbits[ZEN_NMEM_OP];
    uint16_t        flowargbits[ZEN_NFLOW_OP];
    uint16_t        xferargbits[ZEN_NXFER_OP];
    uint8_t         alunargtab[ZEN_NALU_OP];
    uint8_t         memnargbit[ZEN_NMEM_OP];
    uint8_t         flownargbit[ZEN_NMEM_OP];
    uint8_t         xfernargbit[ZEN_NXFER_OP];
};

/* bits-argument */
#define ZEN_UNIT_NBIT     8
#define ZEN_ALU_NBIT      ZEN_UNIT_NBIT
#define ZEN_MEM_NBIT      ZEN_UNIT_NBIT
#define ZEN_FLOW_NBIT     ZEN_UNIT_NBIT
#define ZEN_XFER_NBIT     ZEN_UNIT_NBIT
#define ZEN_UNIT_SRC      (1 << 0)
#define ZEN_UNIT_DEST     (1 << 1)
#define ZEN_UNIT_DPTR     (1 << 2)
#define ZEN_UNIT_SYNC     (1 << 3)
#define ZEN_UNIT_FILT     (1 << 4)
#define ZEN_UNIT_DYN_MEM  (1 << 5)
#define ZEN_UNIT_SEND_EV  (1 << 6)
#define ZEN_UNIT_TERM     (1 << 7)
#define ZEN_UNIT_ARG_NBIT 8

#define zengetfunc(vm, op)                                              \
    ((vm)->opinfo->unitfuncs[zengetunit(op)][zengetinst(op)])

static __inline__ zenword_t
zeninitaluop(struct zenvm *vm, struct zenop *op,
             struct zenopinfo *info)
{
    zenword_t  k = op->k;
    zenword_t  a = wm->a;
    zenword_t  unit = zengetunit(op);
    zenword_t  inst = zengetinst(op);
    zenopfunc *func = zenopfunctab[unit][inst];

    if (instisvalid(unit, inst)) {
        pc = func(vm, k, a);
        vm->pc = pc;
    }

    return pc;
}

static __inline__ zenword_t
zeninitmemop(struct zenvm *vm, struct zenop *op,
             struct zenopinfo *info)
{
    zenword_t    src;
    zenword_t    dest;
    zenword_t    unit = zengetunit(op)
    zenword_t    inst = zengetinst(op)
    zenopfunc_t *func = zenopfunctab[unit][inst];

    if (instisvalid(unit, inst)) {
        pc = func(vm, src, dest);
        vm->pc = pc;
    }

    return pc;
}

static __inline__
zeninitldop1(sttuct zenvm *vm, sturct zenop *op,
             struct zenopinfo *info, long bits)
{
    do {
        zenword_t *_a = (vm)-a;                                         \
        zenword_t  _destb = zengetarg2byte(op);                         \
        zenword_t  _dest = (dest);                                      \
        zenword_t  _srcb = zengetarg1byte(op);                          \
        zenword_t  _src = (src);                                        \
        zenword_t *_dptr = &(dest);                                     \
        zenword_t  _val;                                                \
                                                                        \
        (pc) = vm->pc;                                                  \
        if (bits) {                                                     \
            if (_arg1t & ZEN_IMMED_BIT) {                               \
                (vm)->a = _val;                                         \
            } else if (_arg1t & ZEN_PKT_BIT) {                          \
                _arg += (vm)->pkt;                                      \
                if (_arg1t & ZEN_OFS_FIT) {                             \
                    _arg += (op)->k;                                    \
                }                                                       \
            }                                                           \
        }                                                               \
    } while (0);

#endif /* __ZEN_OP_H__ */

