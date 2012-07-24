#define PTHREAD 1

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/trix.h>
#if (PTHREAD)
#include <pthread.h>
#include <zero/mtx.h>
#endif
#include <wpm/wpm.h>
#include <wpm/mem.h>
#if (WPMDB)
#include <wpm/asm.h>
#endif

#define WPMDEBUG 0

#if defined(__i386__) || defined(__i486__) || defined(__i586__)         \
    || defined(__i686__) || defined(__x86_64__) || defined(__amd64__)
#define USEASM 1
#include <wpm/ia32/asm.h>
#else
#define USEASM 0
#endif

typedef void ophandler_t(struct wpmopcode *);
typedef void hookfunc_t(struct wpmopcode *);

#if (WPMDB)
extern struct asmline *linehash[];
#endif
extern uint32_t       *mempagetab;

/* logical operations */
static void opnot(struct wpmopcode *op);
static void opand(struct wpmopcode *op);
static void opor(struct wpmopcode *op);
static void opxor(struct wpmopcode *op);
/* shift and rotate */
static void opshl(struct wpmopcode *op);
static void opshr(struct wpmopcode *op);
static void opshrl(struct wpmopcode *op);
static void opror(struct wpmopcode *op);
static void oprol(struct wpmopcode *op);
/* arithmetic operations */
static void opinc(struct wpmopcode *op);
static void opdec(struct wpmopcode *op);
static void opadd(struct wpmopcode *op);
static void opsub(struct wpmopcode *op);
static void opcmp(struct wpmopcode *op);
static void opmul(struct wpmopcode *op);
static void opdiv(struct wpmopcode *op);
static void opmod(struct wpmopcode *op);
/* branch operations */
static void opbz(struct wpmopcode *op);
static void opbnz(struct wpmopcode *op);
static void opblt(struct wpmopcode *op);
static void opble(struct wpmopcode *op);
static void opbgt(struct wpmopcode *op);
static void opbge(struct wpmopcode *op);
static void opbo(struct wpmopcode *op);
static void opbno(struct wpmopcode *op);
static void opbc(struct wpmopcode *op);
static void opbnc(struct wpmopcode *op);
/* stack operations */
static void oppop(struct wpmopcode *op);
static void oppush(struct wpmopcode *op);
/* load/store */
static void opmov(struct wpmopcode *op);
static void opmovb(struct wpmopcode *op);
static void opmovw(struct wpmopcode *op);
/* jump */
static void opjmp(struct wpmopcode *op);
/* function call interface */
static void opcall(struct wpmopcode *op);
static void openter(struct wpmopcode *op);
static void opleave(struct wpmopcode *op);
static void opret(struct wpmopcode *op);
/* machine status word */
static void oplmsw(struct wpmopcode *op);
static void opsmsw(struct wpmopcode *op);
/* reset and shutdown */
static void opreset(struct wpmopcode *op);
static void opnop(struct wpmopcode *op);
static void ophlt(struct wpmopcode *op);
static void opbrk(struct wpmopcode *op);
static void optrap(struct wpmopcode *op);
static void opcli(struct wpmopcode *op);
static void opsti(struct wpmopcode *op);
static void opiret(struct wpmopcode *op);
static void opthr(struct wpmopcode *op);
static void opcmpswap(struct wpmopcode *op);
static void opinb(struct wpmopcode *op);
static void opoutb(struct wpmopcode *op);
static void opinw(struct wpmopcode *op);
static void opoutw(struct wpmopcode *op);
static void opinl(struct wpmopcode *op);
static void opoutl(struct wpmopcode *op);
static void ophook(struct wpmopcode *op);

static void hookpzero(struct wpmopcode *op);
static void hookpalloc(struct wpmopcode *op);
static void hookpfree(struct wpmopcode *op);

static void memstore(int32_t src, uint32_t virt);
static int32_t memfetch(uint32_t virt);

ophandler_t *opfunctab[256] ALIGNED(PAGESIZE)
    = {
    NULL,
    opnot,
    opand,
    opor,
    opxor,
    opshl,
    opshr,
    opshrl,
    opror,
    oprol,
    opinc,
    opdec,
    opadd,
    opsub,
    opcmp,
    opmul,
    opdiv,
    opmod,
    opbz,
    opbnz,
    opblt,
    opble,
    opbgt,
    opbge,
    opbo,
    opbno,
    opbc,
    opbnc,
    oppop,
    oppush,
    opmov,
    opmovb,
    opmovw,
    opjmp,
    opcall,
    openter,
    opleave,
    opret,
    oplmsw,
    opsmsw,
    opreset,
    opnop,
    ophlt,
    opbrk,
    optrap,
    opcli,
    opsti,
    opiret,
    opthr,
    opcmpswap,
    opinb,
    opoutb,
    opinw,
    opoutw,
    opinl,
    opoutl,
    ophook
};

char *opnametab[256]
= {
    NULL,
    "not",
    "and",
    "or",
    "xor",
    "shl",
    "shr",
    "shrl",
    "ror",
    "rol",
    "inc",
    "dec",
    "add",
    "sub",
    "cmp",
    "mul",
    "div",
    "mod",
    "bz",
    "bnz",
    "blt",
    "ble",
    "bgt",
    "bge",
    "bo",
    "bno",
    "bc",
    "bnc",
    "pop",
    "push",
    "mov",
    "movb",
    "movw",
    "jmp",
    "call",
    "enter",
    "leave",
    "ret",
    "lmsw",
    "smsw",
    "reset",
    "nop",
    "hlt",
    "brk",
    "trap",
    "cli",
    "sti",
    "iret",
    "thr",
    "cmpswap",
    "inb",
    "outb",
    "inw",
    "outw",
    "inl",
    "outl",
    "hook"
};

hookfunc_t *hookfunctab[256]
= {
    hookpzero,
    hookpalloc,
    hookpfree
};

char *argnametab[]
= {
    "ARG_NONE",
    "ARG_IMMED",
    "ARG_ADR",
    "ARG_REG",
    "ARG_SYM"
};

#if (PTHREAD)
__thread struct wpm  *wpm;
#else
struct wpm           *wpm;
#endif
static volatile long  atomlk;
static volatile long  intlk;
static volatile long  thrlk;
static volatile long  thrcnt;
static volatile long  thrid;
uint8_t              *physmem;

static uint32_t
wpmgetthrid(void)
{
    uint32_t id;

    id = ++thrid;

    return id;
}

struct wpm *
wpminit(void)
{
    mtxlk(&thrlk, 1);
    if (!physmem) {
        physmem = calloc(1, MEMSIZE);
    }
    if (!wpm) {
        wpm = calloc(1, sizeof(struct wpm));
        if (wpm) {
            wpm->cpustat.sp = MEMSIZE - thrid * THRSTKSIZE;
            wpm->cpustat.fp = MEMSIZE - thrid * THRSTKSIZE;
            wpm->thrid = wpmgetthrid();
        }
    }
    mtxunlk(&thrlk, 1);

    return wpm;
}

#if (PTHREAD)
void
wpminitthr(uint32_t pc)
{
    pthread_t           tid;
    struct wpmcpustate *cpustat = malloc(sizeof(struct wpmcpustate));

    mtxlk(&thrlk, 1);
    thrcnt++;
    mtxunlk(&thrlk, 1);
    memcpy(cpustat, &wpm->cpustat, sizeof(struct wpmcpustate));
    cpustat->pc = pc;
    pthread_create(&tid, NULL, wpmloop, (void *)cpustat);
    pthread_detach(tid);

    return;
}
#endif /* PTHREAD */

void
wpmprintop(struct wpmopcode *op)
{
    fprintf(stderr, "INST: %s, size %d, arg1t == %s, arg2t == %s, reg1 == %x, reg2 == %x, args[0] == %x", opnametab[op->inst], op->size << 2, argnametab[op->arg1t], argnametab[op->arg2t], op->reg1, op->reg2, op->args[0]);
    if (op->arg2t) {
        fprintf(stderr, ", args[1] == %x", op->args[1]);
    }
    fprintf(stderr, "\n");
}

void *
wpmloop(void *cpustat)
{
    ophandler_t      *func;
    struct wpmopcode *op;
#if (WPMTRACE)
    int               i;
#endif
#if (WPMDB)
    struct asmline   *line;
#endif

    wpminit();
#if (WPMTRACE)
    fprintf(stderr, "memory\n");
    fprintf(stderr, "------\n");
    for (i = WPMTEXTBASE ; i < WPMTEXTBASE + 256 ; i++) {
        fprintf(stderr, "%02x ", physmem[i]);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "registers\n");
    fprintf(stderr, "---------\n");
    fprintf(stderr, "---------\n");
    for (i = 0 ; i < NREG ; i++) {
        fprintf(stderr, "r%d:\t%x\n", i, wpm->cpustat.regs[i]);
    }
#endif
    wpm->shutdown = 0;
    memcpy(&wpm->cpustat, cpustat, sizeof(struct wpmcpustate));
    free(cpustat);
    while (!wpm->shutdown) {
        op = (struct wpmopcode *)&physmem[wpm->cpustat.pc];
        if (op->inst == OPNOP) {
            wpm->cpustat.pc++;
        } else {
            wpm->cpustat.pc = align(wpm->cpustat.pc, 4);
            op = (struct wpmopcode *)&physmem[wpm->cpustat.pc];
            func = opfunctab[op->inst];
            if (func) {
//                wpmprintop(op);
#if (WPMDB)
                line = asmfindline(wpm->cpustat.pc);
                if (line) {
                    fprintf(stderr, "%s:%d:\t%s\n", line->file, line->num, line->data);
                }
#endif
                func(op);
            } else {
                fprintf(stderr, "illegal instruction, PC == %x\n", wpm->cpustat.pc);
                wpmprintop(op);
                
                exit(1);
            }
        }
    }
#if (WPMTRACE)
    fprintf(stderr, "memory\n");
    fprintf(stderr, "------\n");
    for (i = WPMTEXTBASE ; i < WPMTEXTBASE + 256 ; i++) {
        fprintf(stderr, "%02x ", physmem[i]);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "registers\n");
    fprintf(stderr, "---------\n");
    for (i = 0 ; i < NREG ; i++) {
        fprintf(stderr, "r%d:\t%x\n", i, wpm->cpustat.regs[i]);
    }
#endif
    mtxlk(&thrlk, 1);
    thrcnt--;
    if (!thrcnt) {
        exit(0);
    }
    mtxunlk(&thrlk, 1);

    return NULL;
}

static void
opnot(struct wpmopcode *op)
{
    uint32_t argt = op->arg1t;
    int32_t  dest = (argt == ARG_REG
                     ? wpm->cpustat.regs[op->reg1]
                     : op->args[0]);
#if (WPMDEBUG)
    fprintf(stderr, "NOT: %x - ", dest);
#endif
#if (USEASM)
    asmnot(dest);
#else
    dest = ~dest;
#endif
#if (WPMDEBUG)
    fprintf(stderr, "%x\n", dest);
#endif
    if (argt == ARG_REG) {
        wpm->cpustat.regs[op->reg1] = dest;
    } else {
        memstore(dest, op->args[0]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opand(struct wpmopcode *op)
{
    uint32_t argt1 = op->arg1t;
    uint32_t argt2 = op->arg2t;
    int32_t  src = (argt1 == ARG_REG
                    ? wpm->cpustat.regs[op->reg1]
                    : op->args[0]);
    int32_t  dest = (argt2 == ARG_REG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARG_REG
                        ? op->args[0]
                        : op->args[1]));

#if (WPMDEBUG)
    fprintf(stderr, "AND: %x, %x - ", src, dest);
#endif
#if (USEASM)
    asmand(src, dest);
#else
    dest &= src;
#endif
#if (WPMDEBUG)
    fprintf(stderr, "%x\n", dest);
#endif
    if (argt2 == ARG_REG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARG_REG) {
        memstore(dest, op->args[0]);
    } else {
        memstore(dest, op->args[1]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opor(struct wpmopcode *op)
{
    uint32_t argt1 = op->arg1t;
    uint32_t argt2 = op->arg2t;
    int32_t  src = (argt1 == ARG_REG
                    ? wpm->cpustat.regs[op->reg1]
                    : op->args[0]);
    int32_t  dest = (argt2 == ARG_REG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARG_REG
                        ? op->args[0]
                        : op->args[1]));

#if (USEASM)
    asmor(src, dest);
#else
    dest |= src;
#endif
    if (argt2 == ARG_REG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARG_REG) {
        memstore(dest, op->args[0]);
    } else {
        memstore(dest, op->args[1]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opxor(struct wpmopcode *op)
{
    uint32_t argt1 = op->arg1t;
    uint32_t argt2 = op->arg2t;
    int32_t  src = (argt1 == ARG_REG
                    ? wpm->cpustat.regs[op->reg1]
                    : op->args[0]);
    int32_t  dest = (argt2 == ARG_REG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARG_REG
                        ? op->args[0]
                        : op->args[1]));

#if (WPMDEBUG)
    fprintf(stderr, "XOR: %x, %x - ", src, dest);
#endif
#if (USEASM)
    asmxor(src, dest);
#else
    dest ^= src;
#endif
#if (WPMDEBUG)
    fprintf(stderr, "%x\n", dest);
#endif
    if (argt2 == ARG_REG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARG_REG) {
        memstore(dest, op->args[0]);
    } else {
        memstore(dest, op->args[1]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opshl(struct wpmopcode *op)
{
    uint32_t argt1 = op->arg1t;
    uint32_t argt2 = op->arg2t;
    int32_t  cnt = (argt1 == ARG_REG
                    ? wpm->cpustat.regs[op->reg1]
                    : (argt1 == ARG_IMMED
                       ? op->args[0]
                       : memfetch(op->args[0])));
    int32_t  dest = (argt2 == ARG_REG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARG_REG
                        ? op->args[0]
                        : op->args[1]));
    
#if (WPMDEBUG)
    fprintf(stderr, "SHL: %x by %d\n", dest, cnt);
#endif
#if (USEASM)
    asmshl(cnt, dest);
#else
    dest <<= cnt;
#endif
#if (WPMDEBUG)
    fprintf(stderr, "%x\n", dest);
#endif
    if (argt2 == ARG_REG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARG_REG) {
        memstore(dest, op->args[0]);
    } else {
        memstore(dest, op->args[1]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opshr(struct wpmopcode *op)
{
    uint32_t argt1 = op->arg1t;
    uint32_t argt2 = op->arg2t;
    int32_t  cnt = (argt1 == ARG_REG
                    ? wpm->cpustat.regs[op->reg1]
                    : (argt1 == ARG_IMMED
                       ? op->args[0]
                       : memfetch(op->args[0])));
    int32_t  dest = (argt2 == ARG_REG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARG_REG
                        ? op->args[0]
                        : op->args[1]));

#if (WPMDEBUG)
    fprintf(stderr, "SHR: %x by %d\n", dest, cnt);
#endif
#if (!USEASM)
    uint32_t sign = dest & 0x80000000;
#endif

#if (USEASM)
    asmshr(cnt, dest);
#else
    sign = (sign) ? 0xffffffff << (32 - cnt) : 0;
    dest >>= cnt;
    dest |= sign;
#endif
#if (WPMDEBUG)
    fprintf(stderr, "%x\n", dest);
#endif
    if (argt2 == ARG_REG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARG_REG) {
        memstore(dest, op->args[0]);
    } else {
        memstore(dest, op->args[1]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opshrl(struct wpmopcode *op)
{
    uint32_t argt1 = op->arg1t;
    uint32_t argt2 = op->arg2t;
    int32_t  cnt = (argt1 == ARG_REG
                    ? wpm->cpustat.regs[op->reg1]
                    : (argt1 == ARG_IMMED
                       ? op->args[0]
                       : memfetch(op->args[0])));
    int32_t  dest = (argt2 == ARG_REG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARG_REG
                        ? op->args[0]
                        : op->args[1]));
#if (!USEASM)
    uint32_t sign = 0xffffffff >> cnt;
#endif
    
#if (WPMDEBUG)
    fprintf(stderr, "SHRL: %x by %d\n", dest, cnt);
#endif
#if (USEASM)
    asmshrl(cnt, dest);
#else
    dest >>= cnt;
    dest &= sign;
#endif
#if (WPMDEBUG)
    fprintf(stderr, "%x\n", dest);
#endif
    if (argt2 == ARG_REG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARG_REG) {
        memstore(dest, op->args[0]);
    } else {
        memstore(dest, op->args[1]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opror(struct wpmopcode *op)
{
    uint32_t argt1 = op->arg1t;
    uint32_t argt2 = op->arg2t;
    int32_t  cnt = (argt1 == ARG_REG
                    ? wpm->cpustat.regs[op->reg1]
                    : (argt1 == ARG_IMMED
                       ? op->args[0]
                       : memfetch(op->args[0])));
    int32_t  dest = (argt2 == ARG_REG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARG_REG
                        ? op->args[0]
                        : op->args[1]));
#if (!USEASM)
    uint32_t mask = 0xffffffff >> (32 - cnt);
    uint32_t bits = dest & mask;
#endif

#if (WPMDEBUG)
    fprintf(stderr, "ROR: 0x%08x, %d - ", dest, cnt);
#endif
#if (USEASM)
    asmror(cnt, dest);
#else
    bits <<= 32 - cnt;
    dest >>= cnt;
    dest |= bits;
#endif
#if (WPMDEBUG)
    fprintf(stderr, "%x\n", dest);
#endif
    if (argt2 == ARG_REG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARG_REG) {
        memstore(dest, op->args[0]);
    } else {
        memstore(dest, op->args[1]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
oprol(struct wpmopcode *op)
{
    uint32_t argt1 = op->arg1t;
    uint32_t argt2 = op->arg2t;
    int32_t  cnt = (argt1 == ARG_REG
                    ? wpm->cpustat.regs[op->reg1]
                    : (argt1 == ARG_IMMED
                       ? op->args[0]
                       : memfetch(op->args[0])));
    int32_t  dest = (argt2 == ARG_REG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARG_REG
                        ? op->args[0]
                        : op->args[1]));
#if (!USEASM)
    uint32_t mask = 0xffffffff << (32 - cnt);
    uint32_t bits = dest & mask;
#endif

#if (WPMDEBUG)
    fprintf(stderr, "ROL: 0x%08x, %d - ", dest, cnt);
#endif
#if (USEASM)
    asmrol(cnt, dest);
#else
    bits >>= 32 - cnt;
    dest <<= cnt;
    dest |= bits;
#endif
#if (WPMDEBUG)
    fprintf(stderr, "%x\n", dest);
#endif
    if (argt2 == ARG_REG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARG_REG) {
        memstore(dest, op->args[0]);
    } else {
        memstore(dest, op->args[1]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opinc(struct wpmopcode *op)
{
    uint32_t argt = op->arg1t;
    int32_t  dest = (argt == ARG_REG
                     ? wpm->cpustat.regs[op->reg1]
                     : op->args[0]);
    
    if (!~dest) {
        wpm->cpustat.msw |= MSW_OF;
    }
#if (USEASM)
    asminc(dest);
#else
    dest++;
#endif
    if (argt == ARG_REG) {
        wpm->cpustat.regs[op->reg1] = dest;
    } else {
        memstore(dest, op->args[0]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opdec(struct wpmopcode *op)
{
    uint32_t argt = op->arg1t;
    int32_t  dest = (argt == ARG_REG
                     ? wpm->cpustat.regs[op->reg1]
                     : op->args[0]);

#if (USEASM)
    asmdec(dest);
#else
    dest--;
#endif
    if (!dest) {
        wpm->cpustat.msw |= MSW_ZF;
    }
    if (argt == ARG_REG) {
        wpm->cpustat.regs[op->reg1] = dest;
    } else {
        memstore(dest, op->args[0]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opadd(struct wpmopcode *op)
{
    uint32_t argt1 = op->arg1t;
    uint32_t argt2 = op->arg2t;
    int32_t  src = (argt1 == ARG_REG
                    ? wpm->cpustat.regs[op->reg1]
                    : op->args[0]);
    int32_t  dest = (argt2 == ARG_REG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARG_REG
                        ? op->args[0]
                        : op->args[1]));

#if (USEASM)
    asmadd(src, dest);
#else
    dest += src;
#endif
    if (argt2 == ARG_REG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARG_REG) {
        memstore(dest, op->args[0]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opsub(struct wpmopcode *op)
{
    uint32_t argt1 = op->arg1t;
    uint32_t argt2 = op->arg2t;
    int32_t  src = (argt1 == ARG_REG
                    ? wpm->cpustat.regs[op->reg1]
                    : op->args[0]);
    int32_t  dest = (argt2 == ARG_REG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARG_REG
                        ? op->args[0]
                        : op->args[1]));

#if (USEASM)
    asmsub(src, dest);
#else
    dest -= src;
#endif
    if (argt2 == ARG_REG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARG_REG) {
        memstore(dest, op->args[0]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opcmp(struct wpmopcode *op)
{
    uint32_t msw = wpm->cpustat.msw;
    uint32_t argt1 = op->arg1t;
    uint32_t argt2 = op->arg2t;
    int32_t  src = (argt1 == ARG_REG
                    ? wpm->cpustat.regs[op->reg1]
                    : op->args[0]);
    int32_t  dest = (argt2 == ARG_REG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARG_REG
                        ? op->args[0]
                        : op->args[1]));

#if (WPMDEBUG)
    fprintf(stderr, "CMP: %x, %x\n", src, dest);
#endif
    msw &= ~(MSW_ZF | MSW_SF);
    if (src < dest) {
        msw |= MSW_SF;
    } else if (src == dest) {
        msw |= MSW_ZF;
    }
    wpm->cpustat.msw = msw;
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opmul(struct wpmopcode *op)
{
    uint32_t argt1 = op->arg1t;
    uint32_t argt2 = op->arg2t;
    int32_t  src = (argt1 == ARG_REG
                    ? wpm->cpustat.regs[op->reg1]
                    : op->args[0]);
    int32_t  dest = (argt2 == ARG_REG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARG_REG
                        ? op->args[0]
                        : op->args[1]));

#if (USEASM)
    asmmul(src, dest);
#else
    dest *= src;
#endif
    if (argt2 == ARG_REG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARG_REG) {
        memstore(dest, op->args[0]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opdiv(struct wpmopcode *op)
{
    uint32_t argt1 = op->arg1t;
    uint32_t argt2 = op->arg2t;
    int32_t  src = (argt1 == ARG_REG
                    ? wpm->cpustat.regs[op->reg1]
                    : op->args[0]);
    int32_t  dest = (argt2 == ARG_REG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARG_REG
                        ? op->args[0]
                        : op->args[1]));
    uint32_t sp = wpm->cpustat.sp;
    uint32_t pc;

    if (!src) {
        sp -= 4;
        sp = wpm->cpustat.sp;
        wpm->cpustat.sp = sp;
        pc = memfetch(TRAPDIV << 2);
        if (pc) {
            wpm->cpustat.pc = pc;
        } else {
            fprintf(stderr, "division by zero\n");

            exit(1);
        }
    } else {
#if (USEASM)
        asmdiv(src, dest);
#else
        dest /= src;
#endif
    }
    if (argt2 == ARG_REG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARG_REG) {
        memstore(dest, op->args[0]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}


static void
opmod(struct wpmopcode *op)
{
    uint32_t argt1 = op->arg1t;
    uint32_t argt2 = op->arg2t;
    int32_t  src = (argt1 == ARG_REG
                    ? wpm->cpustat.regs[op->reg1]
                    : op->args[0]);
    int32_t  dest = (argt2 == ARG_REG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARG_REG
                        ? op->args[0]
                        : op->args[1]));

#if (USEASM)
    asmmod(src, dest);
#else
    dest %= src;
#endif
    if (argt2 == ARG_REG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARG_REG) {
        memstore(dest, op->args[0]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opbz(struct wpmopcode *op)
{
    if (wpm->cpustat.msw & MSW_ZF) {
        uint32_t argt = op->arg1t;
        int32_t  dest = (argt == ARG_REG
                         ? wpm->cpustat.regs[op->reg1]
                         : op->args[0]);
        wpm->cpustat.pc = dest;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

static void
opbnz(struct wpmopcode *op)
{
    if (!(wpm->cpustat.msw & MSW_ZF)) {
        uint32_t argt = op->arg1t;
        int32_t  dest = (argt == ARG_REG
                         ? wpm->cpustat.regs[op->reg1]
                         : op->args[0]);
        wpm->cpustat.pc = dest;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

static void
opblt(struct wpmopcode *op)
{
    if (wpm->cpustat.msw & MSW_SF) {
        uint32_t argt = op->arg1t;
        int32_t  dest = (argt == ARG_REG
                         ? wpm->cpustat.regs[op->reg1]
                         : op->args[0]);
        wpm->cpustat.pc = dest;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

static void
opble(struct wpmopcode *op)
{
    if (wpm->cpustat.msw & MSW_SF || wpm->cpustat.msw & MSW_ZF) {
        uint32_t argt = op->arg1t;
        int32_t  dest = (argt == ARG_REG
                         ? wpm->cpustat.regs[op->reg1]
                         : op->args[0]);
        wpm->cpustat.pc = dest;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

static void
opbgt(struct wpmopcode *op)
{
    if (!(wpm->cpustat.msw & MSW_SF) && !(wpm->cpustat.msw & MSW_ZF)) {
        uint32_t argt = op->arg1t;
        int32_t  dest = (argt == ARG_REG
                         ? wpm->cpustat.regs[op->reg1]
                         : op->args[0]);
        wpm->cpustat.pc = dest;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

static void
opbge(struct wpmopcode *op)
{
    if (!(wpm->cpustat.msw & MSW_SF)) {
        uint32_t argt = op->arg1t;
        int32_t  dest = (argt == ARG_REG
                         ? wpm->cpustat.regs[op->reg1]
                         : op->args[0]);
        wpm->cpustat.pc = dest;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

static void
opbo(struct wpmopcode *op)
{
    if (wpm->cpustat.msw & MSW_OF) {
        uint32_t argt = op->arg1t;
        int32_t  dest = (argt == ARG_REG
                         ? wpm->cpustat.regs[op->reg1]
                         : op->args[0]);
        wpm->cpustat.pc = dest;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

static void
opbno(struct wpmopcode *op)
{
    if (!(wpm->cpustat.msw & MSW_OF)) {
        uint32_t argt = op->arg1t;
        int32_t  dest = (argt == ARG_REG
                         ? wpm->cpustat.regs[op->reg1]
                         : op->args[0]);
        wpm->cpustat.pc = dest;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

static void
opbc(struct wpmopcode *op)
{
    if (wpm->cpustat.msw & MSW_CF) {
        uint32_t argt = op->arg1t;
        int32_t  dest = (argt == ARG_REG
                         ? wpm->cpustat.regs[op->reg1]
                         : op->args[0]);
        wpm->cpustat.pc = dest;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

static void
opbnc(struct wpmopcode *op)
{
    if (!(wpm->cpustat.msw & MSW_CF)) {
        uint32_t argt = op->arg1t;
        int32_t  dest = (argt == ARG_REG
                         ? wpm->cpustat.regs[op->reg1]
                         : op->args[0]);
        wpm->cpustat.pc = dest;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

static void
oppop(struct wpmopcode *op)
{
    uint32_t argt = op->arg1t;
    int32_t  val;

    if (argt == ARG_REG) {
        val = memfetch(wpm->cpustat.sp);
        wpm->cpustat.sp += 4;
        wpm->cpustat.regs[op->reg1] = val;
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

static void
oppush(struct wpmopcode *op)
{
    uint32_t argt = op->arg1t;
    int32_t  src = (argt == ARG_REG
                    ? wpm->cpustat.regs[op->reg1]
                    : op->args[0]);
    
    wpm->cpustat.sp -= 4;
    memstore(src, wpm->cpustat.sp);
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opmov(struct wpmopcode *op)
{
    uint32_t argt1 = op->arg1t;
    uint32_t argt2 = op->arg2t;
    uint32_t reg1  = op->reg1;
    uint32_t reg2  = op->reg2;
    int32_t  src = (argt1 == ARG_REG
                    ? ((reg1 & REG_INDEX)
                       ? memfetch(wpm->cpustat.regs[reg1 & 0x0f] + op->args[0])
                       : (reg1 & REG_INDIR
                          ? memfetch(wpm->cpustat.regs[reg1 & 0x0f])
                          : wpm->cpustat.regs[reg1 & 0x0f]))
                    : (argt1 == ARG_IMMED
                       ? op->args[0]
                       : memfetch(op->args[0])));
    uint32_t dest;

    if (argt2 == ARG_REG) {
        if (reg2 & REG_INDIR) {
            dest = wpm->cpustat.regs[reg2 & 0x0f];
            memstore(src, dest);
        } else if (reg2 & REG_INDEX) {
            if (argt1 == ARG_REG) {
                dest = wpm->cpustat.regs[reg2 &0x0f] + op->args[0];
            } else {
                dest = wpm->cpustat.regs[reg2 &0x0f] + op->args[1];
            }
            memstore(src, dest);
        } else {
            wpm->cpustat.regs[reg2 & 0x0f] = src;
        }
    } else if (argt1 == ARG_REG) {
        memstore(src, op->args[0]);
    } else {
        memstore(src, op->args[1]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opmovb(struct wpmopcode *op)
{
    uint32_t argt1 = op->arg1t;
    uint32_t argt2 = op->arg2t;
    uint32_t reg1  = op->reg1;
    uint32_t reg2  = op->reg2;
    int8_t   src = (argt1 == ARG_REG
                    ? ((reg1 & REG_INDEX)
                       ? memfetchb(wpm->cpustat.regs[reg1 & 0x0f] + op->args[0])
                       : (reg1 & REG_INDIR
                          ? memfetchb(wpm->cpustat.regs[reg1 & 0x0f])
                          : wpm->cpustat.regs[reg1 & 0x0f]))
                    : (argt1 == ARG_IMMED
                       ? op->args[0]
                       : memfetchb(op->args[0])));
    uint32_t dest;

#if (WPMDEBUG)
    fprintf(stderr, "MOVB: %x - \n", src);
#endif    
    if (argt2 == ARG_REG) {
        if (reg2 & REG_INDIR) {
            dest = wpm->cpustat.regs[reg2 & 0x0f];
            memstoreb(src, dest);
        } else if (reg2 & REG_INDEX) {
            if (argt1 == ARG_REG) {
                dest = wpm->cpustat.regs[reg2 &0x0f] + op->args[0];
            } else {
                dest = wpm->cpustat.regs[reg2 &0x0f] + op->args[1];
            }
            memstoreb(src, dest);
        } else {
            wpm->cpustat.regs[reg2 & 0x0f] = src;
        }
    } else if (argt1 == ARG_REG) {
        memstoreb(src, op->args[0]);
    } else {
        memstoreb(src, op->args[1]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opmovw(struct wpmopcode *op)
{
    uint32_t argt1 = op->arg1t;
    uint32_t argt2 = op->arg2t;
    uint32_t reg1  = op->reg1;
    uint32_t reg2  = op->reg2;
    int16_t  src = (argt1 == ARG_REG
                    ? ((reg1 & REG_INDEX)
                       ? memfetchw(wpm->cpustat.regs[reg1 & 0x0f] + op->args[0])
                       : (reg1 & REG_INDIR
                          ? memfetchw(wpm->cpustat.regs[reg1 & 0x0f])
                          : wpm->cpustat.regs[reg1 & 0x0f]))
                    : (argt1 == ARG_IMMED
                       ? op->args[0]
                       : memfetchw(op->args[0])));
    uint32_t dest;
    
    if (argt2 == ARG_REG) {
        if (reg2 & REG_INDIR) {
            dest = wpm->cpustat.regs[reg2 & 0x0f];
            memstorew(src, dest);
        } else if (reg2 & REG_INDEX) {
            if (argt1 == ARG_REG) {
                dest = wpm->cpustat.regs[reg2 & 0x0f] + op->args[0];
            } else {
                dest = wpm->cpustat.regs[reg2 & 0x0f] + op->args[1];
            }
            memstorew(src, dest);
        } else {
            wpm->cpustat.regs[reg2 & 0x0f] = src;
        }
    } else if (argt1 == ARG_REG) {
        memstorew(src, op->args[0]);
    } else {
        memstorew(src, op->args[1]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opjmp(struct wpmopcode *op)
{
    uint32_t argt = op->arg1t;
    int32_t  dest = (argt == ARG_REG
                     ? wpm->cpustat.regs[op->reg1]
                     : op->args[0]);

    wpm->cpustat.pc = dest;

    return;
}

/*
 * call stack
 * ----------
 * r7
 * r6
 * r5
 * r4
 * r3
 * r2
 * r1
 * retadr
 * oldfp
 */
static void
opcall(struct wpmopcode *op)
{
    uint32_t argt = op->arg1t;
    int32_t  dest = (argt == ARG_REG
                     ? wpm->cpustat.regs[op->reg1]
                     : op->args[0]);
    int32_t *stk = (int32_t *)(&physmem[wpm->cpustat.sp]);
    int32_t  fp = wpm->cpustat.sp - 36;

    stk[-1] = wpm->cpustat.regs[7];
    stk[-2] = wpm->cpustat.regs[6];
    stk[-3] = wpm->cpustat.regs[5];
    stk[-4] = wpm->cpustat.regs[4];
    stk[-5] = wpm->cpustat.regs[3];
    stk[-6] = wpm->cpustat.regs[2];
    stk[-7] = wpm->cpustat.regs[1];
    stk[-8] = wpm->cpustat.pc + (op->size << 2);
    stk[-9] = wpm->cpustat.fp;
    wpm->cpustat.fp = fp;
    wpm->cpustat.sp = fp;
    wpm->cpustat.pc = dest;

    return;
}

static void
openter(struct wpmopcode *op)
{
    uint32_t  argt = op->arg1t;
    int32_t   ofs = (argt == ARG_REG
                     ? wpm->cpustat.regs[op->reg1]
                     : op->args[0]);
    int32_t *stk = (int32_t *)wpm->cpustat.sp;

    *--stk = wpm->cpustat.fp;
    wpm->cpustat.sp -= ofs + 4;

    return;
}

static void
opleave(struct wpmopcode *op)
{
    int32_t fp = memfetch(wpm->cpustat.fp);
    
    wpm->cpustat.fp = memfetch(fp);
    wpm->cpustat.sp = fp - 4;

    return;
}

/*
 * ret stack
 * ---------
 */
static void
opret(struct wpmopcode *op)
{
    int32_t   fp = memfetch(wpm->cpustat.fp);
    int32_t   dest = memfetch(wpm->cpustat.fp + 4);
    int32_t  *stk = (int32_t *)(&physmem[wpm->cpustat.fp + 8]);

    wpm->cpustat.regs[1] = stk[0];
    wpm->cpustat.regs[2] = stk[1];
    wpm->cpustat.regs[3] = stk[2];
    wpm->cpustat.regs[4] = stk[3];
    wpm->cpustat.regs[5] = stk[4];
    wpm->cpustat.regs[6] = stk[5];
    wpm->cpustat.regs[7] = stk[6];
    wpm->cpustat.fp = fp;
    wpm->cpustat.sp = fp;
    wpm->cpustat.pc = dest;

    return;
}

static void
oplmsw(struct wpmopcode *op)
{
    uint32_t argt = op->arg1t;
    int32_t  msw = (argt == ARG_REG
                    ? wpm->cpustat.regs[op->reg1]
                    : op->args[0]);

    wpm->cpustat.msw = msw;
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opsmsw(struct wpmopcode *op)
{
    int32_t msw = wpm->cpustat.msw;
    int32_t dest = op->args[0];
    
    memstore(msw, dest);
    
    return;
}

static void
opreset(struct wpmopcode *op)
{
    exit(1);

    return;
}

static void
ophlt(struct wpmopcode *op)
{
    wpm->shutdown = 1;

    return;
}

static void
opnop(struct wpmopcode *op)
{
    wpm->cpustat.pc += op->size << 2;
}

static void
opbrk(struct wpmopcode *op)
{
    uint32_t pc = memfetch(TRAPBRK << 2);

    if (pc) {
        wpm->cpustat.pc = pc;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

static void
optrap(struct wpmopcode *op)
{
    uint32_t argt1 = op->arg1t;
    int32_t  trap = (argt1 == ARG_REG
                     ? wpm->cpustat.regs[op->reg1]
                     : memfetch(op->args[0]));
    int32_t  pc = memfetch(trap << 2);

    if (pc) {
        int32_t *stk = (int32_t *)(&physmem[wpm->cpustat.sp]);
        int32_t  fp = wpm->cpustat.sp - 8;
        stk[-1] = wpm->cpustat.pc + (op->size << 2);
        stk[-2] = wpm->cpustat.fp;
        wpm->cpustat.fp = fp;
        wpm->cpustat.sp = fp;
        wpm->cpustat.pc = pc;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

static void
opcli(struct wpmopcode *op)
{
    mtxtrylk(&intlk, 1);
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opsti(struct wpmopcode *op)
{
    mtxunlk(&intlk, 1);
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opiret(struct wpmopcode *op)
{
    int32_t fp = memfetch(wpm->cpustat.fp);
    int32_t dest = memfetch(wpm->cpustat.fp + 4);

    wpm->cpustat.fp = fp;
    wpm->cpustat.sp = fp;
    wpm->cpustat.pc = dest;

    return;
}

static void
opthr(struct wpmopcode *op)
{
    uint32_t argt1 = op->arg1t;
    uint32_t pc  = (argt1 == ARG_REG
                    ? wpm->cpustat.regs[op->reg1]
                    : op->args[0]);

#if (WPMTRACE)
    fprintf(stderr, "thread == 0x%08x\n", pc);
#endif
    wpminitthr(pc);
    wpm->cpustat.pc += op->size << 2;

    return;
}

#if (PTHREAD)
/*
 * - opcmpswap can be used to create mutexes
 */
static void
opcmpswap(struct wpmopcode *op)
{
    uint32_t argt1 = op->arg1t;
    uint32_t argt2 = op->arg2t;
    int32_t adr = (argt1 == ARG_REG
                   ? wpm->cpustat.regs[op->reg1]
                   : memfetch(op->args[0]));
    int32_t val = (argt2 == ARG_REG
                   ? wpm->cpustat.regs[op->reg2]
                   : (argt1 == ARG_REG
                      ? memfetch(op->args[0])
                      : memfetch(op->args[1])));

    mtxlk(&atomlk, 1);
    while (memfetch(adr)) {
        pthread_yield();
    }
    memstore(val, adr);
    mtxunlk(&atomlk, 1);

    return;
}
#endif /* PTHREAD */

static void
opinb(struct wpmopcode *op)
{
}

static void
opoutb(struct wpmopcode *op)
{
    uint8_t data = (uint8_t)wpm->cpustat.regs[op->reg1];
    uint8_t port = (uint8_t)wpm->cpustat.regs[op->reg2];

    switch (port) {
        case CONOUTPORT:
            fprintf(stdout, "%c", data);
            fflush(stdout);

            break;
        case ERROUTPORT:
            fprintf(stderr, "%c", data);
            fflush(stderr);

            break;
    }

    wpm->cpustat.pc += op->size << 2;
}

static void
opinw(struct wpmopcode *op)
{
}

static void
opoutw(struct wpmopcode *op)
{
}

static void
opinl(struct wpmopcode *op)
{
}

static void
ophook(struct wpmopcode *op)
{
    uint32_t argt = op->arg1t;
    int32_t id = (argt == ARG_REG
                  ? op->reg1
                  : op->args[0]);
    hookfunc_t *hook = hookfunctab[id];

    if (hook) {
        hook(op);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

static void
opoutl(struct wpmopcode *op)
{
}

void
wpmpzero(uint32_t adr, uint32_t size)
{
    uint32_t  npg = roundup2(size, 1U << MINBKT) >> MINBKT;
    void     *ptr = NULL;

    while (npg--) {
        if (adr < MEMSIZE) {
            ptr = &physmem[adr];
        } else {
            ptr = (void *)mempagetab[pagenum(adr)];
        }
        if (ptr) {
            bzero(ptr, 4096);
        } else {
            fprintf(stderr, "PZERO: page not mapped\n");

            exit(1);
        }
        adr += 4096;
    }
}

/* TODO: this and other hooks need to use page tables */
void
hookpzero(struct wpmopcode *op)
{
    uint32_t adr = wpm->cpustat.regs[0];
    uint32_t sz = wpm->cpustat.regs[1];

    wpmpzero(adr, sz);

    return;
}

void
hookpalloc(struct wpmopcode *op)
{
    uint32_t size = wpm->cpustat.regs[0];

    wpm->cpustat.regs[0] = mempalloc(size);

    return;
}

void
hookpfree(struct wpmopcode *op)
{
    uint32_t adr = wpm->cpustat.regs[0];

    mempfree(adr);

    return;
}
