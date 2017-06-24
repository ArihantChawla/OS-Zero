#ifndef __ZEN_ASM_H__
#define __ZEN_ASM_H__

#define ZEN_REQ_REG_BIT   0x01
#define ZEN_REQ_VAR_BIT   0x02
#define ZEN_REQ_SYM_BIT   0x04
#define ZEN_REQ_ALPHA_BIT 0x08
#define ZEN_REQ_DIGIT_BIT 0x10
#define ZEN_REQ_IMMED_BIT 0x20
#define ZEN_REQ_IND_BIT   0x40
#define ZEN_REQ_OFS_BIT   0x80

#define zengetarg1(vm, op, argt, argp, bits, req)                       \
    do {                                                                \
        zenword_t _inst = (op)->code & ZEN_OP_INST_MASK;                \
        zenword_t _arg = ZEN_ARG_NONE;                                  \
                                                                        \
        switch (argt) {                                                 \
            case ZEN_ARG_NONE:                                          \
                                                                        \
                break;                                                  \
            case ZEN_ARG_REG:                                           \
                if ((bits) & (ZEN_REG_X_BIT)) {                         \
                    (op)->args.val1 = (vm)->x;                          \
                } else {                                                \
                    (op)->args.val1 = (vm)->a;                          \
                }                                                       \
                break;                                                  \
            case ZEN_ARG_IMMED:                                         \
                (op)->args[0] = (op)->k;                                \
                                                                        \
                break;                                                  \
            case ZEN_ARG_MEM:                                           \
                                                                        \
                break;                                                  \
            case ZEN_ARG_PKT:                                           \
                                                                        \
                break;                                                  \
            case ZEN_ARG_MUL4:                                          \
                                                                        \
                break;                                                  \
            case ZEN_ARG_SYM:                                           \
                                                                        \
                break;                                                  \
            case ZEN_ARG_JMP:                                           \
                                                                        \
                break;                                                  \
            case ZEN_ARG_JMP:                                           \
                                                                        \
                break;                                                  \
            default:                                                    \
                                                                        \
                break;                                                  \
        }                                                               \
    } while (0)

#define zensetargs(op)                                                  \
    do {                                                                \
        uint32_t _arg1t = (zengetargbyte(op, ZEN_ARG1_NDX)              \
                           | zengetargbits(op, ZEN_ARG1_NDX));
        uint32_t _arg2t = (zengetargbyte(op, ZEN_ARG2_NDX)              \
                           | zengetargbits(op, ZEN_ARG2_NDX));
        uint32_t _arg3t = (zengetargbyte(op, ZEN_ARG3_NDX)              \
                           | zengetargbits(op, ZEN_ARG3_NDX));
    } while (0)

#define ZEN_TRAPVEC_ADR    0
#define ZEN_NTRAP          256
struct zentrap {
    uint16_t flg;
};
#define ZEN_START_ADR      ZEN_NTRAP
#define ZEN_SYM_NONE       0    // 0 is always start
#define ZEN_SYM_UNRESOLVED (~(zenword_t)0)
struct zensym {
    struct zensym *chain;       // hash-table chain
    size_t         key;         // symbol hash key
    const char    *name;        // symbol/label name
    zenword_t      adr;         // symbol address or ZEN_SYM_UNRESOLVED
    long           flg;         // SYM_RESOLVED
};

struct zenasmfile {
    const char     *name;       // file name
    int             fd;         // file descriptor
    size_t          nop;        // number of ops in use
    size_t          noptab;     // number of ops allocated in optab
    struct zenop   *optab;      // program image
    size_t          nsym;       // # of symbols in use
    size_t          nsymtab;    // # of symbols allocated in symtab
    struct zensym **symtab;     // symbol/label hash table
};

struct zenasm {
    size_t            nfile;
    struct zeniofile *filetab;
}

#endif /* __ZEN_ASM_H__ */

