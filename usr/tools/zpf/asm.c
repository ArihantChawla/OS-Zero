struct zpfopfunc  *zpfopfunctab[ZPF_OP_NUNIT][ZPF_OP_NINST];
struct zpfop     **zpfoptab;

struct zpfop *
zpfasmop(struct zvm *vm, struct zpfop *op)
{
    zpfword_t   code = op->code;
    zpfopfunc **functab = vm->opinfo->unitfunc[unit];
    uint8_t     narg = vm->opinfo->unitopnargs[unit][inst];
    zpfword_t   areg;
    zpfword_t   xreg;
    zpfword_t   arg;
    zpfword_t   unit = (code & OP_UNIT_MASK) >> OP_UNIT_OFS;
    zpfword_t   inst = code & OP_INST_MASK;
#if 0
    zpfword_t   jt = op->jt;
    zpfword_t   jf = op->jf;
    zpfword_t   argb1 = zpfgetarg1byte(op);
    zpfword_t   argb2 = zpfgetarg2byte(op);
    zpfword_t   argb3 = zpfgetarg3byte(op);
#endif
    zpfopfunc  *func = functab[inst];

    switch (unit) {
        case ZPF_ALU_UNIT:
            if (vm->opinfo->unitopnargs[inst] == 2) {
                zpfinitaluop2(vm, op, pc, xreg, arg);
            } else {
                zpfinitaluop1(vm, op, pc, arg);
            }
            
            break;
    }

    return;
}

