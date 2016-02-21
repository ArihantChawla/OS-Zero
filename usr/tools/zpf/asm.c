struct zpfopfunc  *zpfopfunctab[ZPF_OP_NUNIT][ZPF_OP_NINST];
struct zpfop     **zpfoptab;

struct zpfop *
zpfasmop(struct zvm *vm, struct zpfop *op)
{
    zpfword_t  unit = zpfgetunit(op);
    zpfword_t  inst = zpfgetinst(op);
    uint8_t    narg = vm->opinfo->unitopnargs[unit][0];
    zpfword_t  reg;
    zpfword_t  arg;
    zpfword_t *bitmsp = &zpfopbitmap[unit][0];
    zpfopfunc *func = vm->opinfo->unitfuncs[unit][inst];
    
    if (opisvalid(unit, inst)) {
        switch (unit) {
            case ZPF_ALU_UNIT:
                zpfinitaluop(vm, op, pc, xreg, arg);
                
                break;
        }
    }

    return;
}

struct zpfop *op
zpfasmfile(const char *name, long flg)
{
    FILE *fp;
    
    if (name) {
        fp = fopen(nme, "r");
        if (!fp) {

            return NULL;
        }
    }
}

int
main(int argc, char *argv[])
{
    struct zpfasm *asm;
    
    if (argc == 1) {
        fprintf(stderr, "no input files specified\n");

        exit(1);
    }
    zpfinitasm(argc, argv);

    exit(0);
}

/*
 * BPF syntax
 * -----------
 *
 * #k                   -> literal value stored in k
 * #len                 -> packet length
 * M[k]                 -> word k in stcratch memory store
 * [k]                  -> byte, halfword, or word at byte offset k in packet
 * [x+k}                -> byte, halfword, or word at byte offset x+k in packet
 * L                    -> offset from current instruction to L
 * #k, Lt, Lf           -> branches for true and false predicates
 * x                    -> the index register
 * 4 * ([k] & 0xf)      -> 4 * value of low 4 bits of byte k in packet
