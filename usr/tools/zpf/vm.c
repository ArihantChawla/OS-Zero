#include <zpf/conf.h>
#include <stdlib.h>
#include <zero/trix.h>
#include <zpf/zpf.h>

struct zpfopinfo  zpcopinfo ALIGNED(PAGESIZE);
static zpfword_t *zpfopmap[ZPF_NUNIT];

/*
 * detect overflow of (a) + (b) with a < UINT_MAX - b
 */
static __inline__ zpfword_t
zpfaddop(struct zpfvm *vm, struct zpfop *op,
         struct zpfopinfo *info, long bits)
{
    zpfword_t a = vm->a;;
    zpfword_t pc;
    zpfword_t arg;
    zpfword_t bits = ZPF_ALU_NBIT;

    if (opisvalid(op)) {
        zpfinitaluop(vm, op, pc, src, dest, &zpcopinfo);
        if (arg < 0 && a < ZPFWORD_MIN - arg) {
#if (ZPF_FAIL_UNDERFLOW)
            vm->exit = ZPF_UNDERFLOW;
            return -1;
#else
            vm->msw |= ZPF_UF_BIT;
#endif
        } else if (a > ZPFWORD_MAX - arg) {
#if (ZPF_FAIL_OVERFLOW)
            vm->exit = ZPF_OVERFLOW;
            
            return -1;
#else
            vm->msw |= ZPF_OF_BIT;
#endif
        }
        pc++;
        a += arg;
        vm->pc = pc;
        vm->a = a;
    }
    
    return pc;
}

zpfword_t
zpfsubop(struct zpfvm *vm, struct zpfop *op,
         struct zpfopinfo *info, long bits)
{
    zpfword_t a;
    zpfword_t pc;
    zpfword_t arg;
    
    zpfinitaluop(vm, op, pc, src, dest, dptr,
                 ZPF_ALU_SRC | ZPF_ALU_DEST | ZPF_ALU_DPTR);
    zpfinitaluop(vm, op, pc, a, arg);
    if (arg < 0 && a > ZPFWORD_MAX + arg) {
#if (ZPF_FAIL_OVERFLOW)
        wm->exit = ZPF_OVERFLOW;
        
        return -1;
#else
        vm->msw |= ZPF_OF_BIT;
#endif
    } else if (a < ZPFWORD_MIN + arg) {
#if (ZPF_FAIL_UNDERFLOW)
        wm->exit = ZPF_UNDERFLOW;
        
        return -1;
#else
        vm->msw |= ZPF_UF_BIT;
#endif
    }
    pc++;
    a -= arg;
    vm->pc = pc;
    vm->a = a;
    
    return pc;
}

zpfword_t
zpfmulop(struct zpfvm *vm, struct zpfop *op,
         struct zpfopinfo *info, long bits)
{
#if (LONG_MAX > ZPFWORD_MAX)
    zpfword_t a;
    zpfword_t arg;
    zpfword_t tmp = ZPFWORD_MAX;
#else
    zpfword_t a;
    zpfword_t arg;
    zpfword_t abs1;
    zpfword_t abs2;
    zpfword_t nsig1;
    zpfword_t nsig2;
#endif
#if (LONG_MAX <= ZPFWORD_MAX)
    zpfword_t tmp;
#endif
    zpfword_t pc;
    
    zpfinitaluop(vm, op, pc, a, arg);
#if (!(LONG_MAX > ZPFWORD_MAX))
        abs1 = zeroabs(a);
        abs2 = zeroabs(arg);
        nsig1 = bpfnsigbit(a);
        nsig2 = bpfnsigbit(arg);
        tmp = nsig1 + nsig2;
#endif
    if ((a < 0 && arg < 0) || (a > 0 && arg > 0)) {
#if (LONG_MAX > ZPFWORD_MAX)
        tmp = ~tmp;
#else
        tmp = a;
#endif
        pc++;
        a *= arg;
        vm->pc = pc;
#if (LONG_MAX > ZPFWORD_MAX)
        if (a & tmp) {
#if (ZPF_FAIL_OVERFLOW)
            vm->exit = ZPF_OVERFLOW;
            
            return -1;
#else
            vm->msw |= ZPF_OF_BIT;
#endif
        }
#else /* !(LONG_MAX > ZPFWORD_MAX */
        if (tmp == ZPFWORD_SIZE * CHAR_BIT) {
            if ((arg) && a / arg != tmp) {
#if (ZPF_FAIL_OVERFLOW)
                vm->exit = ZPF_OVERFLOW;
                
                return -1;
#else
                vm->msw |= ZPF_OF_BIT;
#endif
            }
        } else if (tmp > ZPFWORD_SIZE * CHAR_BIT) {
#if (ZPF_FAIL_OVERFLOW)
            vm->exit = ZPF_OVERFLOW;
            
            return -1;
#else
            vm->msw |= ZPF_OF_BIT;
#endif
        }
#endif /* (LONG_MAX > ZPFWORD_MAX */
    } else if (tmp == ZPFWORD_SIZE * CHAR_BIT) {
        if ((arg) && a / arg != tmp) {
#if (ZPF_FAIL_UNDERFLOW)
            vm->exit = ZPF_UNDERFLOW;
            
            return -1;
#else
            vm->msw |= ZPF_UF_BIT;
#endif
        }
    } else if (tmp > ZPFWORD_SIZE * CHAR_BIT) {
#if (ZPF_FAIL_UNDERFLOW)
        vm->exit = ZPF_UNDERFLOW;
        
        return -1;
#else
        vm->msw |= ZPF_UF_BIT;
#endif
    }
    vm->a = a;
    
    return pc;
}

zpfword_t
zpfdivop(struct zpfvm *vm, struct zpfop *op,
         struct zpfopinfo *info, long bits)
{
    zpfword_t a;
    zpfword_t pc;
    zpfword_t arg;

    zpfinitaluop(vm, op, pc, a, arg);
    if (!arg) {
        vm->exit = ZPF_DIVZERO;

        return -1;
    } else {
        pc++;
        a /= arg;
        vm->pc = pc;
        vm->a = a;
    }

    return pc;
}

zpfword_t
zpfmodop(struct zpfvm *vm, struct zpfop *op,
         struct zpfopinfo *info, long bits)
{
    zpfword_t a;
    zpfword_t pc;
    zpfword_t arg;

    zpfinitaluop(vm, op, pc, a, arg);
    if (!arg) {
        vm->exit = ZPF_DIVZERO;

        return -1;
    } else {
        pc++;
        a %= arg;
        vm->pc = pc;
        vm->a = a;
    }

    return pc;
}

zpfword_t
zpfandop(struct zpfvm *vm, struct zpfop *op,
         struct zpfopinfo *info, long bits)
{
    zpfword_t a;
    zpfword_t pc;
    zpfword_t arg;

    zpfinitaluop(vm, op, pc, a, arg);
    pc++;
    a &= arg;
    vm->pc = pc;
    vm->a = a

    return pc;
}

zpfword_t
zpforop(struct zpfvm *vm, struct zpfop *op,
        struct zpfopinfo *info, long bits)
{
    zpfword_t a;
    zpfword_t pc;
    zpfword_t arg;

    zpfinitaluop(vm, op, pc, a, arg);
    pc++;
    a |= arg;
    vm->pc = pc;
    vm->a = a;

    return pc;
}

zpfword_t
pfxorop(struct zpfvm *vm, struct zpfop *op,
        struct zpfopinfo *info, long bits)
{
    zpfword_t a;
    zpfword_t pc;
    zpfword_t arg;

    zpfinitaluop(vm, op, pc, a, arg);
    pc++;
    a ^= arg;
    vm->pc = pc;
    vm->a = a;

    return pc;
}

zpfword_t
zpflshop(struct zpfvm *vm, struct zpfop *op,
         struct zpfopinfo *info, long bits)
{
    zpfword_t a;
    zpfword_t pc;
    zpfword_t arg;

    zpfinitaluop(vm, op, pc, a, arg);
    pc++;
    a <<= arg;
    vm->pc = pc;
    vm->a = a;

    return pc;
}

zpfword_t
zpfrshop(struct zpfvm *vm, struct zpfop *op,
         struct zpfopinfo *info, long bits)
{
    zpfword_t a;
    zpfword_t pc;
    zpfword_t arg;

    zpfinitaluop(vm, op, pc, a, arg);
    pc++;
    a >>= arg;
    vm->pc = pc;
    vm->a = a;

    return pc;
}

zpfword_t
zpfnegop(struct zpfvm *vm, struct zpfop *op,
         struct zpfopinfo *info, long bits)
{
    zpfword_t a;
    zpfword_t pc;

    zpfinitaluop1(vm, op, pc, a);
    pc++;
    if (a) {
        a = ~a;
    }
    vm->pc = pc;
    vm->a = a;

    return pc;
}

void
zpfldop(struct zpfvm *vm, struct zpfop *op,
        struct zpfopinfo *info, long bits)
{
    zpfword_t k = (vm)->k;
    zpfword_t pc = (vm)->pc;
    
    zpfinitldop(vm, op, pc, a);
}

#define ZPF_LD           0x01   // load word into A     1, 2, 3, 4, 10
#define ZPF_LDI          0x02   // load word into A     4
#define ZPF_LDH          0x03   // load halfword into A 1, 2
#define ZPF_LDB          0x04   // load byte into A     1, 2
#define ZPF_LDX          0x05   // load word into X     3, 4, 5, 10
#define ZPF_LDXI         0x06   // load word into X     4
#define ZPF_LDXB         0x07   // load byte into X     5
/* store instructions */
/* instruction                  brief                   addressing modes */
#define ZPF_ST           0x08   // store A into M[];    3
#define ZPF_STX          0x09   // store X into M[];    3

void
zpfsetvmdefs(struct zpfvm *vm)
{
    memset(vm, 0, sizetof(struct zpfvm));
    vm->opinfo = &zpfopinfo;
    vm->stat = ZPF_VM_UNINIT;

    return;
}

uint8_t *
zpfinitopmap(uint8_t *tab)
{
    zpfword_t *ptr1;
    zpfword_t *ptr2;
    zpfword_t *ptr3;
    zpfword_t *ptr4;
    long       n;

    if (!tab) {
        n = rounduppow2(ZPF_INST_NBIT, sizeof(zpfword_t)) / CHAR_BIT;
        tab = calloc(ZPF_NUNIT * n * sizeof(zpfword_t));
        zpfopbitmap = tab;
    }
    if (tab) {
        ptr1 = &zpfopbitmap[ZPF_ALU_UNIT][0];
        ptr2 = &zpfopbitmap[ZPF_MEM_UNIT][0];
        ptr3 = &zpfopbitmap[ZPF_FLOW_UNIT][0];
        ptr4 = &zpfopbitmap[ZPF_XFER_UNIT][0];
        ptr1[0] = ((INT32_C(1) << ZFP_ALU_NOP) - 1) ~((zpfword_t)1);
        ptr2[0] = ((INT32_C(1) << ZFP_MEM_NOP) - 1) ~((zpfword_t)1);
        ptr3[0] = ((INT32_C(1) << ZFP_FLOW_NOP) - 1) ~((zpfword_t)1);
        ptr4[0] = ((INT32_C(1) << ZFP_XFER_NOP) - 1) ~((zpfword_t)1);
    }

    return tab;
}

struct zpfvm *
zpfinitvm(struct zpfvm *vm, int argc, char *argv[])
{
    info = zpfopmap;
    if (!vm) {
        vm = malloc(sizeof(struct zpcopvm));
    }
    if (vm) {
        fprintf(stderr, "failed to alloate memory\n");
    }
    zpfsetvmdefs(vm);
    if (info) {
        zpfinitopinfo(&zpfopinfo);
        if (!zpfinitopmap(NULL)) {
            fprintf(stderr, "failed to allocate memory\n");

            exit(ZPF_NOMEM);
        }
    }

    return info;
}

void
zpfprintusage(void)
{
    fprintf(stderr, "usage\n");
    fprintf(stderr, "-----\n");
    fprintf(stderr, "zvm [options] <files>\n");

    return;
}

zpfword_t
zpfruninst(struct zpfvm *vm, struct zpfop *op, zpfword_t pc, npfword_t bits)
{
    zpfword_t  code = op->code;
    zpfword_t  unit = (code >> ZPF_OP_UNIT_OFS) & ZPF_UNIT_MASK;
    zpfword_t  inst = code & ZPF_OP_INST_MASK;
    zpfopfunc *func = vm->opinfo->unitopfuncs[unit][inst];
    zpfword_t  narg = vm->opinfo->unitopnargs[unit][inst];
    zpfword_t  opsz;
    zpfword_t *dest;
    uint8_t   *u8ptr;
    zpfword_t  src;
    zpfword_t  mem;
    zpfword_t  pkt;
    zpfword_t  ofs;

    switch (unit) {
        case ZPF_ALU_UNIT:
            if (narg == 2) {
                zpfinitaluop2(vm, op, pc, dest, src, bits);
            } else {
                zpfinitaluop1(vm, op, pc, dest, src, bits);
            }
            pc = func(vm, op, pc);
                 
            break;
        case ZPF_MEM_UNIT:
            if (narg == 2) {
                zpfinitmemop2(vm, op, pc, dest, src, bits);
            } else {
                zpfinitmemop1(vm, op, pc, dest, src, bits);
            }
            switch (inst) {
                case ZPF_LDB_OP:
                case ZPF_LDH_OP:
                    opsz = inst;
            }

            break;
        case ZPF_FLOW_UNIT:

            break;
        case ZPF_XFER_UNIT:

            break;
    }

    return pc;
}

int
zpfvmmain(int argc, char *argv[])
{
    struct zpfvm *vm;

    if (argc == 1) {
        zpfprintusage();

        exit(ZPF_FAILURE);
    }
    vm = zpfinitvm(NULL, argc, argv);
    if (!vm) {
        fprintf(stderr, "failed to initialise ZPF vm\n");

        exit(ZPF_FAILURE);
    }
    zpfgetvmopts(vm, argc, argv);
    zpfinitvm(vm);
    vm->pc = ZPF_START_ADR;
    do {
        struct zpfop *op = vm->img;
        zpfword_t     pc = vm->pc;
        zpfword_t     end = pc + vm->nimgop;

        do {
            pc = zpfruninst(vm, op, pc);
            op = &vm->img[pc];
        } while (!vm->stat);
    } while (!vm->stat);

    exit(vm->stat);
}

