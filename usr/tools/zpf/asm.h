#ifndef __ZPF_ASM_H__
#define __ZPF_ASM_H__

#define zpfgetarg1(vm, op, argt, argp, bits, req)                       \
    do {                                                                \
        zpfword_t _inst = (op)->code & ZPF_OP_INST_MASK;                \
        zpfword_t _arg = ZPF_ARG_NONE;                                  \
                                                                        \
        switch (argt) {                                                 \
            case ZPF_ARG_NONE:                                          \
                                                                        \
                break;                                                  \
            case ZPF_ARG_REG:                                           \
                if ((bits) & (ZPF_REG_X_BIT))                           \
                (op)->args.val1 = (vm)->a;                              \
                                                                        \
                break;                                                  \
            case ZPF_ARG_IMMED:                                         \
                (op)-> = ZPC_SYM_UNRESOLVED;                            \
                                                                        \
                break;                                                  \
            case ZPF_ARG_MEM:                                           \
                                                                        \
                break;                                                  \
            case ZPF_ARG_PKT:                                           \
                                                                        \
                break;                                                  \
            case ZPF_ARG_MUL4:                                          \
                                                                        \
                break;                                                  \
            case ZPF_ARG_SYM:                                           \
                                                                        \
                break;                                                  \
            case ZPF_ARG_JMP:                                           \
                                                                        \
                break;                                                  \
            case ZPF_ARG_JMP:                                           \
                                                                        \
                break;                                                  \
            default:                                                    \
                                                                        \
                break;                                                  \
        }                                                               \
    } while (0)

#define zpfsetargs(op)                                                  \
    do {                                                                \
        uint32_t _arg1t = (zpfgetargbyte(op, ZPF_ARG1_NDX)              \
                           | zpfgetargbits(op, ZPF_ARG1_NDX));
        uint32_t _arg2t = (zpfgetargbyte(op, ZPF_ARG2_NDX)              \
                           | zpfgetargbits(op, ZPF_ARG2_NDX));
        uint32_t _arg3t = (zpfgetargbyte(op, ZPF_ARG3_NDX)              \
                           | zpfgetargbits(op, ZPF_ARG3_NDX));
    } while (0)

#define ZPF_TRAPVEC_ADR    0
#define ZPF_NTRAP          256
struct zpftrap {
    uint16_t flg;
};
#define ZPF_START_ADR      ZPF_NTRAP
#define ZPF_SYM_NONE       0    // 0 is always start
#define ZPF_SYM_UNRESOLVED (~(zpfword_t)0)
struct zpfsym {
    const char    *name;        // symbol/label name
    zpfhashkey     key;         // symbol hash key
    zpfword_t      adr;         // symbol address or ZPF_SYM_UNRESOLVED
    struct zpfsym *chain;       // hash-table chain
};

struct zpfasmfile {
    const char     *name;       // file name
    int             fd;         // file descriptor
    size_t          nop;        // number of ops in use
    size_t          noptab;     // number of ops allocated in optab
    struct zpmop   *optab;      // program image
    size_t          nsym;       // # of symbols in use
    size_t          nsymtab;    // # of symbols allocated in symtab
    struct zpmsym **symtab;     // symbol/label hash table
};

#endif /* __ZPF_ASM_H__ */

