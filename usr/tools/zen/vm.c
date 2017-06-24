#include <zen/conf.h>
#include <stdlib.h>
#include <zero/trix.h>
#include <zen/zen.h>

#define zenaluprologue(vm, op, info, bits)

extern uint32_t        zenopargbits[ZEN_NUNIT][ZEN_UNIT_NOP];
extern unsigned char   zenreqargsmap[256];
extern unsigned char   zennospacemap[256 / CHAR_BIT];
extern zenopfunc      *zenopfunctab[ZEN_OP_NUNIT][ZEN_OP_NINST];
extern zenop         **zenoptab;

struct zenopinfo       zenopinfo ALIGNED(PAGESIZE);
static zenword_t      *zenopmap[ZEN_NUNIT];
struct zenvm           zenvm;

/*
 * detect overflow of (a) + (b) with a < UINT_MAX - b
 */
static zenword_t
zenaddop(struct zenvm *vm, zenword_t src, zenword_t dest)
{
    zenword_t pc = vm->pc;

    if (opisvalid(op)) {
        zeninitaluop(vm, op, &zpcopinfo);
        if (src < 0 && dest < ZENWORD_MIN - src) {
#if (ZEN_FAIL_UNDERFLOW)
            vm->exit = ZEN_UNDERFLOW;
            return -1;
#else
            vm->msw |= ZEN_UF_BIT;
#endif
        } else if (dest > ZENWORD_MAX - src) {
#if (ZEN_FAIL_OVERFLOW)
            vm->exit = ZEN_OVERFLOW;
            
            return -1;
#else
            vm->msw |= ZEN_OF_BIT;
#endif
        }
        pc++;
        dest += src;
        vm->pc = pc;
        vm->a = dest;
    }
    
    return pc;
}

static zenword_t
zensubop(struct zenvm *vm, zpmword_t src, zenword_t dest)
{
    zenword_t pc = vm->pc;
    
    zeninitaluop(vm, op, info);
    if (src < 0 && dest > ZENWORD_MAX + src) {
#if (ZEN_FAIL_OVERFLOW)
        wm->exit = ZEN_OVERFLOW;
        
        return -1;
#else
        vm->msw |= ZEN_OF_BIT;
#endif
    } else if (a < ZENWORD_MIN + arg) {
#if (ZEN_FAIL_UNDERFLOW)
        wm->exit = ZEN_UNDERFLOW;
        
        return -1;
#else
        vm->msw |= ZEN_UF_BIT;
#endif
    }
    pc++;
    dest -= src;
    vm->pc = pc;
    vm->a = dest;
    
    return pc;
}

static zenword_t
zenmulop(struct zenvm *vm, zenword_t src, zenword_t dest)
{
    zenword_t tmp;
    zenword_t abs1;
    zenword_t abs2;
    zenword_t nsig1;
    zenword_t nsig2;
#endif
    
    zeninitaluop(vm, op, info);
#if (!(LONG_MAX > ZENWORD_MAX))
    abs1 = zeroabs(dest);
    abs2 = zeroabs(src);
    nsig1 = bpfnsigbit(dest);
    nsig2 = bpfnsigbit(src);
    tmp = nsig1 + nsig2;
#endif
    if ((dest < 0 && src < 0) || (dest > 0 && src > 0)) {
#if (LONG_MAX > ZENWORD_MAX)
        tmp = ~tmp;
#else
        tmp = dest;
#endif
        dest *= src;
#if (LONG_MAX > ZENWORD_MAX)
        if (dest & tmp) {
#if (ZEN_FAIL_OVERFLOW)
            vm->exit = ZEN_OVERFLOW;
            
            return -1;
#else
            vm->msw |= ZEN_OF_BIT;
#endif
        }
#else /* !(LONG_MAX > ZENWORD_MAX */
        if (tmp == ZENWORD_SIZE * CHAR_BIT) {
            if ((src) && dest / src != tmp) {
#if (ZEN_FAIL_OVERFLOW)
                vm->exit = ZEN_OVERFLOW;
                
                return -1;
#else
                vm->msw |= ZEN_OF_BIT;
#endif
            }
        } else if (tmp > ZENWORD_SIZE * CHAR_BIT) {
#if (ZEN_FAIL_OVERFLOW)
            vm->exit = ZEN_OVERFLOW;
            
            return -1;
#else
            vm->msw |= ZEN_OF_BIT;
#endif
        }
#endif /* (LONG_MAX > ZENWORD_MAX */
    } else if (tmp == ZENWORD_SIZE * CHAR_BIT) {
        if ((src) && dest / src != tmp) {
#if (ZEN_FAIL_UNDERFLOW)
            vm->exit = ZEN_UNDERFLOW;
            
            return -1;
#else
            vm->msw |= ZEN_UF_BIT;
#endif
        }
    } else if (tmp > ZENWORD_SIZE * CHAR_BIT) {
#if (ZEN_FAIL_UNDERFLOW)
        vm->exit = ZEN_UNDERFLOW;
        
        return -1;
#else
        vm->msw |= ZEN_UF_BIT;
#endif
    }
    pc++;
    vm->a = dest;
    vm->pc = pc;
    
    return pc;
}

static zenword_t
zendivop(struct zenvm *vm, zenword_t src, zenword_t dest)
{
    zenword_t pc = vm->pc;
    
    zeninitaluop(vm, op, info);
    if (!src) {
        vm->exit = ZEN_DIVZERO;

        return -1;
    } else {
        pc++;
        dest /= src;
        vm->pc = pc;
        vm->a = dest;
    }

    return pc;
}

static zenword_t
zenmodop(struct zenvm *vm, zenword_t src, zenword_t dest)
{
    zenword_t pc = vm->pc;
    
    zeninitaluop(vm, op, info);
    if (!src) {
        vm->exit = ZEN_DIVZERO;

        return -1;
    } else {
        pc++;
        if (!powerof2(src)) {
            dest %= src;
        } else {
            dest &= --src;
        }
        vm->pc = pc;
        vm->a = dest;
    }

    return pc;
}

static zenword_t
zenandop(struct zenvm *vm, zenword_t src, zenword_t dest)
{
    zenword_t pc = vm->pc;
    
    zeninitaluop(vm, op, info);
    pc++;
    dest &= src;
    vm->pc = pc;
    vm->a = dest;

    return pc;
}

static zenword_t
zenorop(struct zenvm *vm, zenword_t src, zenword_t dest)
{
    zenword_t pc = vm->pc;

    zeninitaluop(vm, op, info);
    pc++;
    dest |= src;
    vm->pc = pc;
    vm->a = dest;

    return pc;
}

static zenword_t
pfxorop(struct zenvm *vm, zenword_t src, zenword_t dest)
{
    zenword_t pc = vm->pc;

    zeninitaluop(vm, op, info);
    pc++;
    dest ^= src;
    vm->pc = pc;
    vm->a = dest;

    return pc;
}

static zenword_t
zenlshop(struct zenvm *vm, zenword_t src, zenword_t dest)
{
    zenword_t pc = vm->pc;

    zeninitaluop(vm, op, info);
    pc++;
    dest <<= src;
    vm->pc = pc;
    vm->a = dest;

    return pc;
}

static zenword_t
zenrshop(struct zenvm *vm, zenword_t src, zenword_t dest)
{
    zenword_t pc = vm->pc;

    zeninitaluop(vm, op, info);
    pc++;
    dest >>= src;
    vm->pc = pc;
    vm->a = dest;

    return pc;
}

static zenword_t
zennegop(struct zenvm *vm, zenword_t src, zenword_t dest)
{
    zenword_t pc = vm->pc;

    zeninitaluop1(vm, op, info);
    pc++;
    if (dest) {
        dest = -dest;
    }
    vm->pc = pc;
    vm->a = dest;

    return pc;
}

static void
zenldop(struct zenvm *vm, zenword_t src)
{
    zenword_t pc = (vm)->pc;
    
    zeninitldop(vm, op, info);
    pc++;
    
}

#define ZEN_LD           0x01   // load word into A     1, 2, 3, 4, 10
#define ZEN_LDI          0x02   // load word into A     4
#define ZEN_LDH          0x03   // load halfword into A 1, 2
#define ZEN_LDB          0x04   // load byte into A     1, 2
#define ZEN_LDX          0x05   // load word into X     3, 4, 5, 10
#define ZEN_LDXI         0x06   // load word into X     4
#define ZEN_LDXB         0x07   // load byte into X     5
/* store instructions */
/* instruction                  brief                   addressing modes */
#define ZEN_ST           0x08   // store A into M[];    3
#define ZEN_STX          0x09   // store X into M[];    3

void
zensetvmdefs(struct zenvm *vm)
{
    memset(vm, 0, sizetof(struct zenvm));
    vm->opinfo = &zenopinfo;
    vm->stat = ZEN_VM_UNINIT;

    return;
}

uint8_t *
zeninitopmap(uint8_t *tab)
{
    zenword_t *ptr1;
    zenword_t *ptr2;
    zenword_t *ptr3;
    zenword_t *ptr4;
    long       n;

    if (!tab) {
        n = rounduppow2(ZEN_INST_NBIT, sizeof(zenword_t)) / CHAR_BIT;
        tab = calloc(ZEN_NUNIT * n * sizeof(zenword_t));
        zenopbitmap = tab;
    }
    if (tab) {
        ptr1 = &zenopbitmap[ZEN_ALU_UNIT][0];
        ptr2 = &zenopbitmap[ZEN_MEM_UNIT][0];
        ptr3 = &zenopbitmap[ZEN_FLOW_UNIT][0];
        ptr4 = &zenopbitmap[ZEN_XFER_UNIT][0];
        ptr1[0] = ((INT32_C(1) << ZFP_ALU_NOP) - 1) ~((zenword_t)1);
        ptr2[0] = ((INT32_C(1) << ZFP_MEM_NOP) - 1) ~((zenword_t)1);
        ptr3[0] = ((INT32_C(1) << ZFP_FLOW_NOP) - 1) ~((zenword_t)1);
        ptr4[0] = ((INT32_C(1) << ZFP_XFER_NOP) - 1) ~((zenword_t)1);
    }

    return tab;
}

struct zenvm *
zeninitvm(struct zenvm *vm, int argc, char *argv[])
{
    zenbits_t *map;

    info = zenopmap;
    if (!vm) {
        vm = malloc(sizeof(struct zpcopvm));
    }
    if (vm) {
        fprintf(stderr, "failed to alloate memory\n");
    }
    zensetvmdefs(vm);
    if (info) {
        zeninitopinfo(&zenopinfo);
        if (!zeninitopmap(NULL)) {
            fprintf(stderr, "failed to allocate memory\n");

            exit(ZEN_NOMEM);
        }
        map = malloc(ZEN_MEM_NOP * sizeof(zenbits_t));
        if (!map) {

            exit(ZEN_NOMEM);
        }
        vm->bits = map;
    }

    return vm;
}

void
zenprintusage(void)
{
    fprintf(stderr, "usage\n");
    fprintf(stderr, "-----\n");
    fprintf(stderr, "zvm [options] <files>\n");

    return;
}

zenword_t
zenruninst(struct zenvm *vm, struct zenop *op, zenword_t pc, npfword_t bits)
{
    zenword_t  code = op->code;
    zenword_t  unit = (code >> ZEN_OP_UNIT_OFS) & ZEN_UNIT_MASK;
    zenword_t  inst = code & ZEN_OP_INST_MASK;
    zenopfunc *func = vm->opinfo->unitopfuncs[unit][inst];
    zenword_t  narg = vm->opinfo->unitopnargs[unit][inst];
    zenword_t  opsz;
    zenword_t *dest;
    uint8_t   *u8ptr;
    zenword_t  src;
    zenword_t  mem;
    zenword_t  pkt;
    zenword_t  ofs;

    switch (unit) {
        case ZEN_ALU_UNIT:
            if (narg == 2) {
                zeninitaluop2(vm, op, pc, dest, src, bits);
            } else {
                zeninitaluop1(vm, op, pc, dest, src, bits);
            }
            pc = func(vm, op, pc);
                 
            break;
        case ZEN_MEM_UNIT:
            if (narg == 2) {
                zeninitmemop2(vm, op, pc, dest, src, bits);
            } else {
                zeninitmemop1(vm, op, pc, dest, src, bits);
            }
            switch (inst) {
                case ZEN_LDB_OP:
                case ZEN_LDH_OP:
                    opsz = inst;
            }

            break;
        case ZEN_FLOW_UNIT:

            break;
        case ZEN_XFER_UNIT:

            break;
    }

    return pc;
}

int
zenvmmain(int argc, char *argv[])
{
    struct zenvm *vm;

    if (argc == 1) {
        zenprintusage();

        exit(ZEN_FAILURE);
    }
    vm = zeninitvm(NULL, argc, argv);
    if (!vm) {
        fprintf(stderr, "failed to initialise ZEN vm\n");

        exit(ZEN_FAILURE);
    }
    zengetvmopts(vm, argc, argv);
    zeninitvm(vm);
    vm->pc = ZEN_START_ADR;
    do {
        struct zenop *op = vm->img;
        zenword_t     pc = vm->pc;
        zenword_t     end = pc + vm->nimgop;

        do {
            pc = zenruninst(vm, op, pc);
            op = &vm->img[pc];
        } while (!vm->stat);
    } while (!vm->stat);

    exit(vm->stat);
}

