#IFNDEF __ZPF_OP_H__
#define __ZPF_OP_H__

#include <limits.h>
#include <zpf/zpf.h>
#include <zpf/vm.h>

/* operation dispatch function */

#define ZPF_OP_FAILURE     (-1L)
typedef long zpfopfunc(struct zpfvm *vm,
                       struct zpfop *op,
                       struct zpfopinfo *info);

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
 * EXT                 extension       BPF extension
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
#define ZPF_REG_ARG         0   // source and destination registers
#define ZPF_IMMED_ARG       1   // immediate argument in k-field
#define ZPF_MEM_ARG         2   // 4-bit acratch memory area offset   
#define ZPF_PKT_ARG         3   // address relative to packet
#define ZPF_SYM_ARG         4   // symbol [label] address for branches
#define ZPF_JMP_ARG         5   // branch offset from pc for true predicate
#define ZPF_BIG_WORD_ARG    6
#define ZPF_EXT_ARG         7  // BPF extension
#define ZPF_OP_FLG_NBIT     8
#if 0
/* flag-bits for these in high bits of op->args.val[1|2|3] */
#define ZPF_REG_BIT         (1L << ZPF_REG_ARG)
#define ZPF_IMMED_BIT       (1L << ZPF_IMMED_ARG)
#define ZPF_MEM_BIT         (1L << ZPF_MEM_ARG)
#define ZPF_PKT_BIT         (1L << ZPF_PKT_ARG)
#define ZPF_SYM_BIT         (1L << ZPF_SYM_ARG)
#define ZPF_JMP_BIT         (1L << ZPF_JMP_ARG)
#define ZPF_BIG_WORD_BIT    (1L << ZPF_BIG_WORD_ARG)
#define ZPF_EXT_BIT         (1L << ZPF_EXT_ARG)
#endif
#define ZPF_REG_X_BIT       (1L << 31)  // else REG_A
#define ZPF_OFS_BIT         (1L << 30)  // for PKT or MEM
#define ZPF_PKT_MUL4_BIT    (1L << 29)  // for PKT + 4 * ([k] & 0x0f)
#define ZPF_JMP_TRUE_BIT    (1L << 28)  // branch offset for true predicate
#define ZPF_JMP2_BIT        (1L << 27)  // branch offsets for false and true
#define ZPF_PKT_LEN_BIT     (1L << 26)  // current packet lenght in VM
#define ZPF_K_FLG_NBIT      6
#define ZPF_K_FLG_NBIT      0
 // arguments in high bits of (vm)->k
#define ZPF_K_IMMED_NBIT    (1L << (32 - ZPF_K_FLG_NBIT))
#define ZPF_K_VAL_MASK      ((1UL << (32 - ZPF_K_FLG_NBIT)) - 1)
#define ZPF_OP_FLG_LO_NBIT  0
#define ZPF_OP_ADR_NONE     0   // os is always optional trap vector or start
#define ZPF_OP_FLG_HI_NONE  0
#define ZPF_OP_FLG_NONE     0

/* ALU instructions */

/* instruction                  brief                   addressing modes */
#define ZPF_ALU_UNIT     0x00
#define ZPF_ADD          0x01   // A + <x>              0, 4
#define ZPF_SUB          0x02   // A - <x>              0, 4
#define ZPF_MUL          0x03   // A * <x>              0, 4
#define ZPF_DIV          0x04   // A / <x>              0, 4
#define ZPF_MOD          0x05   // A % <x>              0, 4
#define ZPF_AND          0x06   // A & <x>              0, 4
#define ZPF_OR           0x07   // A | <x>              0, 4
#define ZPF_XOR          0x08   // A ^ <x>              0, 4
#define ZPF_LSH          0x09   // A << <x>             0, 4
#define ZPF_RSH          0x0a   // A >> <x>             0, 4
#define ZPF_NEG          0x0b   // -A                   0, 4
#define ZPF_ALU_NOP      12     // # of table items for unit

/* memory instructions */

/* load instructions */
/* instruction                  brief                   addressing modes */
#define ZPF_MEM_UNIT     0x01
#define ZPF_LDB          0x01   // load byte into A     1, 2
#define ZPF_LDH          0x02   // load halfword into A 1, 2
#define ZPF_LDI          0x03   // load word into A     4
#define ZPF_LD           0x04   // load word into A     1, 2, 3, 4, 10
#define ZPF_LDX          0x05   // load word into X     3, 4, 5, 10
#define ZPF_LDXI         0x06   // load word into X     4
#define ZPF_LDXB         0x07   // load byte into X     5
/* store instructions */
/* instruction                  brief                   addressing modes */
#define ZPF_ST           0x08   // store A into M[];    3
#define ZPF_STX          0x09   // store X into M[];    3
#define ZPF_MEM_NOP      10     // # of table items for unit

/* control flow */

/* branch instructions */
/* instruction                  brief                   addressing modes */
#define ZPF_FLOW_UNIT    0x02
#define ZPF_JMP          0x01   // jump to label        6
#define ZPF_JA           0x02   // jump to label        6
#define ZPF_JEQ          0x03   // jump if k == A       7, 8
#define ZPF_JNE          0x04   // jump if k != A       8
#define ZPF_JNEQ         ZPF_JNE // jump if k != A       8
#define ZPF_JLT          0x05   // jump if k < A        8
#define ZPF_JLE          0x06   // jump if k <= A       8
#define ZPF_JGT          0x07   // jump if k > A        7, 8
#define ZPF_JGE          0x08   // jump if k >= A       7, 8
#define ZPF_JSET         0x09   // jump if k & A        7, 8
#define ZPF_RET          0x0a   // return               4, 9
#define ZPF_FLOW_NOP     11     // # of table items for unit

/* miscellaneous instructions */

/* register transfer */
/* instruction                  brief */
#define ZPF_XFER_UNIT    0x03
#define ZPF_TAX          0x01   // copy A into X
#define ZPF_TXA          0x02   // copy X into A
#define ZPF_XFER_NOP     3      // # of table items for unit

/* operation decoding */

#define ZPF_OP_NBIT      16
#define ZPF_INST_NBIT    4      // per-unit instruction IDs
#define ZPF_UNIT_NBIT    2      // unit IDs
#define ZPF_OP_UNIT_OFS  ZPF_INST_NBIT
#define ZPF_OP_INST_MSSK ((1L << ZPF_INST_NBIT) - 1)
#define ZPF_OP_UNIT_MSSK ((1L << ZPF_UNIT_NBIT) - 1)
#define zpfgetinst(op)   ((op)->code & ZPF_INST_MASK)
#define zpfgetunit(op)   (((op)->code >> ZPF_OP_UNIT_OFS) & ZPF_UNIT_MASK)
#define zpfgetargs(op)   ((op)->code & ZPF_OP_FLG_MASK)
//#define ZPF_OP_UNIT_NBIT 2
#define ZPF_OP_FLG_NBIT  (ZPF_OP_NBIT - ZPF_INST_NBIT - ZPF_UNIT_NBIT)
//#define ZPF_OP_ADR_NBIT  (16 - ZPF_OP_NBIT - ZPF_UNIT_NBIT)

#define ZPF_JMP_OFS_NBIT 8
#define ZPF_JMP_MASK     ((1L << ZPF_JMP_OFS_NBIT) - 1)
/* set operation true and false branch offsets; evaluate to (op)->jt */
#define zpfputopjmpjt(op, jt, jf, k)                                    \
    ((op)->jt = (jt) & ZPF_JMP_MASK,                                    \
     (op)->jf = (jf) & ZPF_JMP_MASK,                                    \
     (op)->k = (k) & ZPF_K_VAL_MASK,                                    \
     (op)->jt)
/* set operation true and false branch offsets; evaluate to (op)->jf */
#define zpfputopjmpjf(op, jt, jf, k)                                    \
    ((op)->jt = (jt) & ZPF_JMP_MASK,                                    \
     (op)->jf = (jf) & ZPF_JMP_MASK,                                    \
     (op)->k = (k) & ZPF_K_VAL_MASK,                                    \
     (op)->jf)
#define zpfgetjmptadr(vm, op) ((vm)->pc + (op)->jt)
#define zpfgetjmpfadr(vm, op) ((vm)->pc + (op)->jf)
#define zpfgetsymadr(vm, op)  (zpfgetadrbit((op)->k))
#define ZPF_OP_K_NBIT    32

/*
 * code field format
 * -----------------
 * high 8 bits: addressing mode
 * next 4 bits: unit ID
 * low  4 bits: instruction ID
 */
/* instruction in bits [0, ZPF_OP_FLG_NLO_BIT - 1], low 8 bits in (op)->code */
#define ZPF_OP_INST_OFS     0
#define ZPF_OP_INST_NBIT    8
#define ZPF_OP_INST_MASK    ((1L << ZPF_OP_NINST_BIT) - 1)
/* unit ID in bits [ZPF_OP_UNIT_OFS, ZPF_OP_UNIT_OFS + ZPF_OP_UNIT_BITS - 1] */
#define ZPF_OP_UNIT_OFS     ZPF_OP_NINST_BIT
#define ZPF_OP_UNIT_NBIT    4
#define ZPF_OP_UNIT_MASK    (((1L << ZPF_OP_NUNIT_BIT) - 1)             \
                             << ZPF_OP_UNIT_OFS)
#define ZPF_UNIT_MASK       ((1L << ZPF_OP_UNIT_NBIT) - 1)
#define ZPF_FLG_LO_MASK     (ZPF_INST_MASK | ZPF_UNIT_MASK)
#define ZPF_OP_ADR_OFS      (ZPF_OP_UNIT_OFS + ZPF_OP_NUNIT_BIT)
#define ZPF_OP_ADR_MASK     (((1L << ZPF_OP_ADR_NBIT) - 1) << ZPF_OP_ADR_OFS)
#define ZPF_ADR_NONE         0
#define ZPF_ADR_NBIT        (16 - ZPF_OP_NBIT - ZPF_OP_UNIT_NBIT)
#define ZPF_FLG_MASK        (ZPF_OP_INST_MASK                           \
                             | ZPF_OP_UNIT_MASK                         \
                             | ZPF_OP_ADR_MASK                          \
                             | ZPF_FLG_HI_MASK)
#define ZPF_EXT_BIT         (1L << 31)
#define BPF_LEN_BIT         (1L << 30)
#define ZPF_OP_FLG_HI_NBIT  2
#define ZPF_OP_FLG_HI_OFS   (32 - ZPF_OP_FLG_HI_NBIT)
#define ZPF_OP_FLG_HI_MASK  (((1L << ZPF_OP_FLG_HI_NBIT) - 1)            \
                             << ZPF_OP_ADR_HI_OFS)
#define zpfgetarg(op)       (((op)->code & ZPF_FLG_LO_MASK)
#define zpfgetinst(op)      ((op)->code & ((1 << ZPF_NOP_BIT) - 1))
#define zpfgetunit(op)      (((op)->code >> ZPF_UNIT_OFS) & ZPF_UNIT_MASK)
#define zpfgetadrmode(op, i) (zpgetadrlobits(op) & (1L << (i)))
#define zpfgetadrbits(op)   ((op)->code & ZPF_OP_ADR_MASK)
#define zpfgetargbits(op)   (((op)->code & ZPF_OP_FLG_MASK)             \
                             | ((op)->k & ZPF_OP_FLG_HI_MASK))
#define zpfgetarglo(op, i)  (zpfgetargbits(op) >> (ZPF_OP_ADR_OFS + (i)))
#define zpfgetarghibits(op) ((op)->k & ZPF_OP_FLG_HI_MASK)
#define zpfgetarghi(op, i)                                              \
    ((op)->k >> (32 - ZPF_OP_FLG_HI_OFS + (i) - ZPF_FLG_LO_NBIT))
#define zpfgetarg(op, i)                                                \
    (((i) < ZPF_FLG_LO_NBIT)                                            \
     ? (zpfgetarglo(op, i))                                             \
     : (zpfgetarghi(op, i)))
#define zpfgetadrbit(op, i) ((op)->code & (1L << (ZPF_OP_ADR_OFS + (i))))
#define zpfgetadrbits(op)   ((op)->code & ZPF_OP_ADR_MASK)
#define zpfgetadr(op)       ((op)->code >> ZPF_OP_ADR_OFS)
#if (LONG_SIZE == 8)
#define zpfopuninit(op)     (*((uint64_t *)op) == UINT64_C(0))
#else
#define zpfopuninit(op)     (!(op)->code & !(op)->jt && !(op)->jf & !(op)->k)
#endif
#define zpfinstisnone(op)   (!zpfgetinst(op))
#define zpfargisnone(op)    (!zpfgetargbits(op))
#define zpfadrbitsnone(op)  (!zpfgetadrbits(op))
#define zpfargisreg(op)     (zpfgetargbit(op, ZPF_REG_BIT))
#define zpfargisimmed(op)   (zpfgetargbit(op, ZPF_IMMED_BIT))
#define zpfargismem(op)     (zpfgetargbit(op, ZPF_MEM_BIT))
#define zpfargismul4(op)    (zpfgetargbit(op, ZPF_MUL4_BIT))
#define zpfargissym(op)     (zpfgetargbit(op, ZPF_SYM_BIT))
#define zpfargistjmp(op)    (zpfgetargbit(op, ZPF_TJMP_BIT))
#define zpfargisfjmp(op)    (zpfgetargbit(op, ZPF_FJMP_BIT))
struct zpfop {
    uint16_t  code;     // bits 0..15: 4-bit OP, 4-bit UNIT, 8 address mode bits
    uint8_t   jt;       // 8-bit offset from PC to true-branch
    uint8_t   jf;       // 8-bit offset rom PC to false-branch
    zpfword_t k;        // currently, 32-bit miscellaneous argument
};
#define ZPF_FLG1_NDX        0
#define ZPF_FLG2_NDX        1
#define ZPF_FLG3_NDX        2
#define zpfgetargbyte(op, ndx)                                          \
    (((op)->args.vals[(ndx)])                                           \
     >> (((ndx) * ZPF_OP_FLG_NBIT)                                      \
         & ((1L << ZPF_OP_FLG_NBIT) - 1)))
#define zpfgetargbits(op, ndx)                                          \
    ((((op)->args.vals[(ndx)]) >> ((ndx) * ZPF_OP_FLG_NBIT)) & ZPF_OP_FLG_MASK)
#define zpfgetargbit(op, ndx, i)                                        \
    ((((op)->args.vals[(ndx)]) >> ((ndx) * ZPF_OP_FLG_NBIT + (i))) & 0x01)
#define zpfsetargbyte(op, ndx, b)                                       \
    ((op)->args.vals[(ndx)] = (b))
#define zpfsetargbits(op, ndx, b)                                       \
    ((op)->args.vals[(ndx)] |= (b))
#define zpfsetargbit(op, ndx, i)                                        \
    ((op)->args.vals[(ndx)] |= (1L << (i)))
#define zpfsetargbit(op, ndx, i)                                        \
    ((op)->args.vals[ndx] |= (1L << (i)))
#define zpfclrargbit(op, ndx, i)                                        \
    ((op)->args.vals[ndx] &= ~(1L << (i)))
#define zpfflipbit(op, ndx, i)                                          \
    ((op)->flg ^ (1L << (i)))
#define zpfsetflgbit(info


struct zpfopargs {
    /*
     * val[0], val[1], val[2]
     * ----------------
     * hi bits: REG, INMED, MEM, PKT, MUL4, SYM, TJMP, FJMP
     */
    zpfword_t vals;  // arguments #1, #2, #3 with flags
    zpfword_t flg;   // per-argument flag bits
    zpfword_t adr;      // label or branch address
    zpfword_t pc;       // program counter for relative addressing
    zpfword_t sym;      // target label for branches
    zpfword_t ofs;      // offset for certain addressing modes
    uint32_t  type;     // argument bits or types for up to 3 arguments
};

#define ZPF_NARG 3

#define __STRUCT_ZPFOPDATA_SIZE                                         \
    (sizeof(struct zpfop) + ZPF_NARG * sizeof(struct zpfopargs))
#define __STRUCT_ZPFOPDATA_PAD                                          \
    (rounduppow2(__STRUCT_ZPFOPDATA_SIZE, CLSIZE) - __STRUCT_ZPFOPDATA_SIZE)
struct zpfopdata {
    struct zpfop     op;
    struct zpfopargs args[ZPF_NARG];
#if (__STRUCT_ZPFOPDATA_PAD)
    uint8_t          _pad[__STRUCT_ZPFOPDATA_PAD];
#endif
};

struct zpfopinfo {
    zpfopfunc **unitfuncs[ZPF_NUNIT];
    uint16_t  **unitadrbits[ZPF_NUNIT];
    uint8_t    *unitopnargs[ZPF_NUNIT];
    zpfopfunc  *alufunctab[(1L << ZPF_ALU_NBIT)];
    zpfopfunc  *memfunctab[(1L << ZPF_MEM_NBIT)};
    zpfopfunc  *flowfunctab[ZPF_NFLOW_NBIT];
    zpfopfunc  *xferfunctab[ZPF_NXFER_NBIT];
    uint16_t    aluargbits[ZPF_UNIT_NOP];
    uint16_t    memargbits[ZPF_NMEM_OP];
    uint16_t    flowargbits[ZPF_NFLOW_OP];
    uint16_t    xferargbits[ZPF_NXFER_OP];
    uint8_t     alunargtab[ZPF_NALU_OP];
    uint8_t     memnargbit[ZPF_NMEM_OP];
    uint8_t     flownargbit[ZPF_NMEM_OP];
    uint8_t     xfernargbit[ZPF_NXFER_OP];
};

#endif /* __ZPF_OP_H__ */

