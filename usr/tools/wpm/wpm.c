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
#include <wpm/asm.h>
#if (ZPC)
#include <zpc/asm.h>
#endif

#define WPMDEBUG 0

#if defined(__i386__) || defined(__i486__) || defined(__i586__)         \
    || defined(__i686__) || defined(__x86_64__) || defined(__amd64__)
#define USEASM 0
#include <wpm/ia32/asm.h>
#else
#define USEASM 0
#endif

#if (ZPC)
typedef struct zpcopcode opcode_t;
#elif (WPM)
typedef struct wpmopcode opcode_t;
#endif
typedef void ophandler_t(opcode_t *);
typedef void hookfunc_t(opcode_t *);

#if (WPMDB)
extern struct asmline *linehash[];
#endif
extern asmadr_t       *mempagetab;

/* logical operations */
void opnot(opcode_t *op);
void opand(opcode_t *op);
void opor(opcode_t *op);
void opxor(opcode_t *op);
/* shift and rotate */
void opshr(opcode_t *op);
void opshra(opcode_t *op);
void opshl(opcode_t *op);
void opror(opcode_t *op);
void oprol(opcode_t *op);
/* arithmetic operations */
void opinc(opcode_t *op);
void opdec(opcode_t *op);
void opadd(opcode_t *op);
void opsub(opcode_t *op);
void opcmp(opcode_t *op);
void opmul(opcode_t *op);
void opdiv(opcode_t *op);
void opmod(opcode_t *op);
/* branch operations */
void opbz(opcode_t *op);
void opbnz(opcode_t *op);
void opblt(opcode_t *op);
void opble(opcode_t *op);
void opbgt(opcode_t *op);
void opbge(opcode_t *op);
void opbo(opcode_t *op);
void opbno(opcode_t *op);
void opbc(opcode_t *op);
void opbnc(opcode_t *op);
/* stack operations */
void oppop(opcode_t *op);
void oppush(opcode_t *op);
/* load/store */
void opmov(opcode_t *op);
void opmovb(opcode_t *op);
void opmovw(opcode_t *op);
/* jump */
void opjmp(opcode_t *op);
/* function call interface */
void opcall(opcode_t *op);
void openter(opcode_t *op);
void opleave(opcode_t *op);
void opret(opcode_t *op);
/* machine status word */
void oplmsw(opcode_t *op);
void opsmsw(opcode_t *op);
/* reset and shutdown */
void opreset(opcode_t *op);
void opnop(opcode_t *op);
void ophlt(opcode_t *op);
void opbrk(opcode_t *op);
void optrap(opcode_t *op);
void opcli(opcode_t *op);
void opsti(opcode_t *op);
void opiret(opcode_t *op);
void opthr(opcode_t *op);
void opcmpswap(opcode_t *op);
void opinb(opcode_t *op);
void opoutb(opcode_t *op);
void opinw(opcode_t *op);
void opoutw(opcode_t *op);
void opinl(opcode_t *op);
void opoutl(opcode_t *op);
void ophook(opcode_t *op);

#if (WPM)
static void hookpzero(opcode_t *op);
static void hookpalloc(opcode_t *op);
static void hookpfree(opcode_t *op);
#endif

static void    memstorel(int32_t src, asmadr_t virt);
static int32_t memfetchl(asmadr_t virt);

#if (ZPC)
ophandler_t *opfunctab[ZPCNASMOP] ALIGNED(PAGESIZE)
    = {
    NULL,       // illegal instruction
    opnot,
    opand,
    opor,
    opxor,
    opshr,
    opshra,
    opshl,
    opror,
    oprol,
    opinc,
    opdec,
    opadd,
    opsub,
    opmul,
    opdiv,
    opmod,
    opbz,
    opbnz,
    opblt,
    opble,
    opbgt,
    opbge,
    opmov,
    opcall,
    opret,
    optrap,
    opiret
};
#elif (WPM)
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
#endif
#if (ZPC)
extern char *opnametab[ZPCNASMOP];
#elif (WPM)
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
#endif /* WPM */

char *argnametab[]
= {
    "ARGNONE",
    "ARGIMMED",
    "ARGADR",
    "ARGREG",
    "ARGSYM"
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
wpminitthr(asmadr_t pc)
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

#if (WPM)
void
wpmprintop(opcode_t *op)
{
    fprintf(stderr, "INST: %s, size %d, arg1t == %s, arg2t == %s, reg1 == %x, reg2 == %x, args[0] == %x", opnametab[op->inst], op->size << 2, argnametab[op->arg1t], argnametab[op->arg2t], op->reg1, op->reg2, op->args[0]);
    if (op->arg2t) {
        fprintf(stderr, ", args[1] == %x", op->args[1]);
    }
    fprintf(stderr, "\n");
}
#endif /* WPM */

void *
wpmloop(void *cpustat)
{
    ophandler_t    *func;
    opcode_t       *op;
#if (WPMTRACE)
    int             i;
#endif
#if (WPMDB)
    struct asmline *line;
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
        op = (opcode_t *)&physmem[wpm->cpustat.pc];
#if (WPM)
        if (op->inst == OPNOP) {
            wpm->cpustat.pc++;
        } else
#endif
        {
            wpm->cpustat.pc = roundup2(wpm->cpustat.pc, 4);
            op = (opcode_t *)&physmem[wpm->cpustat.pc];
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
                fprintf(stderr, "illegal instruction, PC == %lx\n",
                        (long)wpm->cpustat.pc);
#if (WPM)
                wpmprintop(op);
#endif
                
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

void
opnot(opcode_t *op)
{
    uint_fast8_t argt = op->arg1t;
    asmword_t    dest = (argt == ARGREG
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
    if (argt == ARGREG) {
        wpm->cpustat.regs[op->reg1] = dest;
    } else {
        memstorel(dest, op->args[0]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

void
opand(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    asmword_t    src = (argt1 == ARGREG
                        ? wpm->cpustat.regs[op->reg1]
                        : op->args[0]);
    asmword_t    dest = (argt2 == ARGREG
                         ? wpm->cpustat.regs[op->reg2]
                         : (argt1 == ARGREG
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
    if (argt2 == ARGREG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARGREG) {
        memstorel(dest, op->args[0]);
    } else {
        memstorel(dest, op->args[1]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

void
opor(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    asmword_t    src = (argt1 == ARGREG
                        ? wpm->cpustat.regs[op->reg1]
                        : op->args[0]);
    asmword_t    dest = (argt2 == ARGREG
                         ? wpm->cpustat.regs[op->reg2]
                         : (argt1 == ARGREG
                            ? op->args[0]
                            : op->args[1]));

#if (USEASM)
    asmor(src, dest);
#else
    dest |= src;
#endif
    if (argt2 == ARGREG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARGREG) {
        memstorel(dest, op->args[0]);
    } else {
        memstorel(dest, op->args[1]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

void
opxor(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    asmword_t    src = (argt1 == ARGREG
                        ? wpm->cpustat.regs[op->reg1]
                        : op->args[0]);
    asmword_t    dest = (argt2 == ARGREG
                         ? wpm->cpustat.regs[op->reg2]
                         : (argt1 == ARGREG
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
    if (argt2 == ARGREG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARGREG) {
        memstorel(dest, op->args[0]);
    } else {
        memstorel(dest, op->args[1]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

void
opshr(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    asmword_t  cnt = (argt1 == ARGREG
                    ? wpm->cpustat.regs[op->reg1]
                    : (argt1 == ARGIMMED
                       ? op->args[0]
                       : memfetchl(op->args[0])));
    asmword_t  dest = (argt2 == ARGREG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARGREG
                        ? op->args[0]
                        : op->args[1]));
#if (!USEASM)
    asmuword_t sign = 0xffffffff >> cnt;
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
    if (argt2 == ARGREG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARGREG) {
        memstorel(dest, op->args[0]);
    } else {
        memstorel(dest, op->args[1]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

void
opshra(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    asmword_t  cnt = (argt1 == ARGREG
                    ? wpm->cpustat.regs[op->reg1]
                    : (argt1 == ARGIMMED
                       ? op->args[0]
                       : memfetchl(op->args[0])));
    asmword_t  dest = (argt2 == ARGREG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARGREG
                        ? op->args[0]
                        : op->args[1]));

#if (WPMDEBUG)
    fprintf(stderr, "SHR: %x by %d\n", dest, cnt);
#endif
#if (!USEASM)
    asmuword_t sign = dest & 0x80000000;
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
    if (argt2 == ARGREG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARGREG) {
        memstorel(dest, op->args[0]);
    } else {
        memstorel(dest, op->args[1]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

void
opshl(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    asmword_t  cnt = (argt1 == ARGREG
                    ? wpm->cpustat.regs[op->reg1]
                    : (argt1 == ARGIMMED
                       ? op->args[0]
                       : memfetchl(op->args[0])));
    asmword_t  dest = (argt2 == ARGREG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARGREG
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
    if (argt2 == ARGREG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARGREG) {
        memstorel(dest, op->args[0]);
    } else {
        memstorel(dest, op->args[1]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

void
opror(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    asmword_t  cnt = (argt1 == ARGREG
                    ? wpm->cpustat.regs[op->reg1]
                    : (argt1 == ARGIMMED
                       ? op->args[0]
                       : memfetchl(op->args[0])));
    asmword_t  dest = (argt2 == ARGREG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARGREG
                        ? op->args[0]
                        : op->args[1]));
#if (!USEASM)
    asmuword_t mask = 0xffffffff >> (32 - cnt);
    asmuword_t bits = dest & mask;
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
    if (argt2 == ARGREG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARGREG) {
        memstorel(dest, op->args[0]);
    } else {
        memstorel(dest, op->args[1]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

void
oprol(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    asmword_t  cnt = (argt1 == ARGREG
                    ? wpm->cpustat.regs[op->reg1]
                    : (argt1 == ARGIMMED
                       ? op->args[0]
                       : memfetchl(op->args[0])));
    asmword_t  dest = (argt2 == ARGREG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARGREG
                        ? op->args[0]
                        : op->args[1]));
#if (!USEASM)
    asmuword_t mask = 0xffffffff << (32 - cnt);
    asmuword_t bits = dest & mask;
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
    if (argt2 == ARGREG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARGREG) {
        memstorel(dest, op->args[0]);
    } else {
        memstorel(dest, op->args[1]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

void
opinc(opcode_t *op)
{
    uint_fast8_t argt = op->arg1t;
    asmword_t  dest = (argt == ARGREG
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
    if (argt == ARGREG) {
        wpm->cpustat.regs[op->reg1] = dest;
    } else {
        memstorel(dest, op->args[0]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

void
opdec(opcode_t *op)
{
    uint_fast8_t argt = op->arg1t;
    asmword_t  dest = (argt == ARGREG
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
    if (argt == ARGREG) {
        wpm->cpustat.regs[op->reg1] = dest;
    } else {
        memstorel(dest, op->args[0]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

void
opadd(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    asmword_t    src = (argt1 == ARGREG
                        ? wpm->cpustat.regs[op->reg1]
                        : op->args[0]);
    asmword_t    dest = (argt2 == ARGREG
                         ? wpm->cpustat.regs[op->reg2]
                         : (argt1 == ARGREG
                            ? op->args[0]
                            : op->args[1]));

#if (USEASM)
    asmadd(src, dest);
#else
    dest += src;
#endif
    if (argt2 == ARGREG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARGREG) {
        memstorel(dest, op->args[0]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

void
opsub(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    asmword_t    src = (argt1 == ARGREG
                        ? wpm->cpustat.regs[op->reg1]
                        : op->args[0]);
    asmword_t    dest = (argt2 == ARGREG
                         ? wpm->cpustat.regs[op->reg2]
                         : (argt1 == ARGREG
                            ? op->args[0]
                            : op->args[1]));

#if (USEASM)
    asmsub(src, dest);
#else
    dest -= src;
#endif
    if (argt2 == ARGREG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARGREG) {
        memstorel(dest, op->args[0]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

void
opcmp(opcode_t *op)
{
    asmword_t    msw = wpm->cpustat.msw;
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    asmword_t    src = (argt1 == ARGREG
                        ? wpm->cpustat.regs[op->reg1]
                        : op->args[0]);
    asmword_t    dest = (argt2 == ARGREG
                         ? wpm->cpustat.regs[op->reg2]
                         : (argt1 == ARGREG
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

void
opmul(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    asmword_t    src = (argt1 == ARGREG
                        ? wpm->cpustat.regs[op->reg1]
                        : op->args[0]);
    asmword_t    dest = (argt2 == ARGREG
                         ? wpm->cpustat.regs[op->reg2]
                         : (argt1 == ARGREG
                            ? op->args[0]
                            : op->args[1]));

#if (USEASM)
    asmmul(src, dest);
#else
    dest *= src;
#endif
    if (argt2 == ARGREG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARGREG) {
        memstorel(dest, op->args[0]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

void
opdiv(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    asmword_t    src = (argt1 == ARGREG
                        ? wpm->cpustat.regs[op->reg1]
                        : op->args[0]);
    asmword_t    dest = (argt2 == ARGREG
                         ? wpm->cpustat.regs[op->reg2]
                         : (argt1 == ARGREG
                            ? op->args[0]
                            : op->args[1]));
    long         sp = wpm->cpustat.sp;
    long         pc;

    if (!src) {
        sp -= 4;
        sp = wpm->cpustat.sp;
        wpm->cpustat.sp = sp;
        pc = memfetchl(TRAPDIV << 2);
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
    if (argt2 == ARGREG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARGREG) {
        memstorel(dest, op->args[0]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}


void
opmod(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    asmword_t    src = (argt1 == ARGREG
                        ? wpm->cpustat.regs[op->reg1]
                        : op->args[0]);
    asmword_t    dest = (argt2 == ARGREG
                         ? wpm->cpustat.regs[op->reg2]
                         : (argt1 == ARGREG
                            ? op->args[0]
                            : op->args[1]));
    
#if (USEASM)
    asmmod(src, dest);
#else
    dest %= src;
#endif
    if (argt2 == ARGREG) {
        wpm->cpustat.regs[op->reg2] = dest;
    } else if (argt1 == ARGREG) {
        memstorel(dest, op->args[0]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

void
opbz(opcode_t *op)
{
    if (wpm->cpustat.msw & MSW_ZF) {
        uint_fast8_t argt = op->arg1t;
        asmword_t    dest = (argt == ARGREG
                             ? wpm->cpustat.regs[op->reg1]
                             : op->args[0]);
        wpm->cpustat.pc = dest;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

void
opbnz(opcode_t *op)
{
    if (!(wpm->cpustat.msw & MSW_ZF)) {
        uint_fast8_t argt = op->arg1t;
        asmword_t    dest = (argt == ARGREG
                             ? wpm->cpustat.regs[op->reg1]
                             : op->args[0]);
        wpm->cpustat.pc = dest;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

void
opblt(opcode_t *op)
{
    if (wpm->cpustat.msw & MSW_SF) {
        uint_fast8_t argt = op->arg1t;
        asmword_t    dest = (argt == ARGREG
                             ? wpm->cpustat.regs[op->reg1]
                             : op->args[0]);
        wpm->cpustat.pc = dest;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

void
opble(opcode_t *op)
{
    if (wpm->cpustat.msw & MSW_SF || wpm->cpustat.msw & MSW_ZF) {
        uint_fast8_t argt = op->arg1t;
        asmword_t    dest = (argt == ARGREG
                             ? wpm->cpustat.regs[op->reg1]
                             : op->args[0]);
        wpm->cpustat.pc = dest;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

void
opbgt(opcode_t *op)
{
    if (!(wpm->cpustat.msw & MSW_SF) && !(wpm->cpustat.msw & MSW_ZF)) {
        uint_fast8_t argt = op->arg1t;
        asmword_t    dest = (argt == ARGREG
                             ? wpm->cpustat.regs[op->reg1]
                             : op->args[0]);
        wpm->cpustat.pc = dest;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

void
opbge(opcode_t *op)
{
    if (!(wpm->cpustat.msw & MSW_SF)) {
        uint_fast8_t argt = op->arg1t;
        asmword_t    dest = (argt == ARGREG
                             ? wpm->cpustat.regs[op->reg1]
                             : op->args[0]);
        wpm->cpustat.pc = dest;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

#if (WPM)

void
opbo(opcode_t *op)
{
    if (wpm->cpustat.msw & MSW_OF) {
        uint_fast8_t argt = op->arg1t;
        asmword_t    dest = (argt == ARGREG
                             ? wpm->cpustat.regs[op->reg1]
                             : op->args[0]);
        wpm->cpustat.pc = dest;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

void
opbno(opcode_t *op)
{
    if (!(wpm->cpustat.msw & MSW_OF)) {
        uint_fast8_t argt = op->arg1t;
        asmword_t    dest = (argt == ARGREG
                             ? wpm->cpustat.regs[op->reg1]
                             : op->args[0]);
        wpm->cpustat.pc = dest;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

void
opbc(opcode_t *op)
{
    if (wpm->cpustat.msw & MSW_CF) {
        uint_fast8_t argt = op->arg1t;
        asmword_t    dest = (argt == ARGREG
                             ? wpm->cpustat.regs[op->reg1]
                             : op->args[0]);
        wpm->cpustat.pc = dest;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

void
opbnc(opcode_t *op)
{
    if (!(wpm->cpustat.msw & MSW_CF)) {
        uint_fast8_t argt = op->arg1t;
        asmword_t    dest = (argt == ARGREG
                             ? wpm->cpustat.regs[op->reg1]
                             : op->args[0]);
        wpm->cpustat.pc = dest;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

void
oppop(opcode_t *op)
{
    uint_fast8_t argt = op->arg1t;
    asmword_t    val;

    if (argt == ARGREG) {
        val = memfetchl(wpm->cpustat.sp);
        wpm->cpustat.sp += 4;
        wpm->cpustat.regs[op->reg1] = val;
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

void
oppush(opcode_t *op)
{
    uint_fast8_t argt = op->arg1t;
    asmword_t    src = (argt == ARGREG
                        ? wpm->cpustat.regs[op->reg1]
                        : op->args[0]);
    
    wpm->cpustat.sp -= 4;
    memstorel(src, wpm->cpustat.sp);
    wpm->cpustat.pc += op->size << 2;

    return;
}

#endif /* WPM */

void
opmov(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    asmuword_t   reg1  = op->reg1;
    asmuword_t   reg2  = op->reg2;
    asmword_t    src = (argt1 == ARGREG
                        ? ((reg1 & REGINDEX)
                           ? memfetchl(wpm->cpustat.regs[reg1 & 0x0f]
                                       + op->args[0])
                           : (reg1 & REGINDIR
                              ? memfetchl(wpm->cpustat.regs[reg1 & 0x0f])
                              : wpm->cpustat.regs[reg1 & 0x0f]))
                        : (argt1 == ARGIMMED
                           ? op->args[0]
                           : memfetchl(op->args[0])));
    asmuword_t   dest;

    if (argt2 == ARGREG) {
        if (reg2 & REGINDIR) {
            dest = wpm->cpustat.regs[reg2 & 0x0f];
            memstorel(src, dest);
        } else if (reg2 & REGINDEX) {
            if (argt1 == ARGREG) {
                dest = wpm->cpustat.regs[reg2 &0x0f] + op->args[0];
            } else {
                dest = wpm->cpustat.regs[reg2 &0x0f] + op->args[1];
            }
            memstorel(src, dest);
        } else {
            wpm->cpustat.regs[reg2 & 0x0f] = src;
        }
    } else if (argt1 == ARGREG) {
        memstorel(src, op->args[0]);
    } else {
        memstorel(src, op->args[1]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

#if (WPM)

void
opmovb(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    asmuword_t   reg1  = op->reg1;
    asmuword_t   reg2  = op->reg2;
    int8_t       src = (argt1 == ARGREG
                        ? ((reg1 & REGINDEX)
                           ? memfetchb(wpm->cpustat.regs[reg1 & 0x0f]
                                       + op->args[0])
                           : (reg1 & REGINDIR
                              ? memfetchb(wpm->cpustat.regs[reg1 & 0x0f])
                              : wpm->cpustat.regs[reg1 & 0x0f]))
                        : (argt1 == ARGIMMED
                           ? op->args[0]
                           : memfetchb(op->args[0])));
    asmuword_t   dest;

#if (WPMDEBUG)
    fprintf(stderr, "MOVB: %x - \n", src);
#endif    
    if (argt2 == ARGREG) {
        if (reg2 & REGINDIR) {
            dest = wpm->cpustat.regs[reg2 & 0x0f];
            memstoreb(src, dest);
        } else if (reg2 & REGINDEX) {
            if (argt1 == ARGREG) {
                dest = wpm->cpustat.regs[reg2 &0x0f] + op->args[0];
            } else {
                dest = wpm->cpustat.regs[reg2 &0x0f] + op->args[1];
            }
            memstoreb(src, dest);
        } else {
            wpm->cpustat.regs[reg2 & 0x0f] = src;
        }
    } else if (argt1 == ARGREG) {
        memstoreb(src, op->args[0]);
    } else {
        memstoreb(src, op->args[1]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

void
opmovw(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    asmuword_t   reg1  = op->reg1;
    asmuword_t   reg2  = op->reg2;
    int16_t      src = (argt1 == ARGREG
                        ? ((reg1 & REGINDEX)
                           ? memfetchw(wpm->cpustat.regs[reg1 & 0x0f]
                                       + op->args[0])
                           : (reg1 & REGINDIR
                              ? memfetchw(wpm->cpustat.regs[reg1 & 0x0f])
                          : wpm->cpustat.regs[reg1 & 0x0f]))
                        : (argt1 == ARGIMMED
                           ? op->args[0]
                       : memfetchw(op->args[0])));
    asmuword_t   dest;
    
    if (argt2 == ARGREG) {
        if (reg2 & REGINDIR) {
            dest = wpm->cpustat.regs[reg2 & 0x0f];
            memstorew(src, dest);
        } else if (reg2 & REGINDEX) {
            if (argt1 == ARGREG) {
                dest = wpm->cpustat.regs[reg2 & 0x0f] + op->args[0];
            } else {
                dest = wpm->cpustat.regs[reg2 & 0x0f] + op->args[1];
            }
            memstorew(src, dest);
        } else {
            wpm->cpustat.regs[reg2 & 0x0f] = src;
        }
    } else if (argt1 == ARGREG) {
        memstorew(src, op->args[0]);
    } else {
        memstorew(src, op->args[1]);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

#endif /* WPM */

void
opjmp(opcode_t *op)
{
    uint_fast8_t argt = op->arg1t;
    asmword_t  dest = (argt == ARGREG
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
void
opcall(opcode_t *op)
{
    uint_fast8_t argt = op->arg1t;
    asmword_t  dest = (argt == ARGREG
                     ? wpm->cpustat.regs[op->reg1]
                     : op->args[0]);
    asmword_t *stk = (asmword_t *)(&physmem[wpm->cpustat.sp]);
    asmword_t  fp = wpm->cpustat.sp - 36;

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

#if (WPM)

void
openter(opcode_t *op)
{
    long  argt = op->arg1t;
    asmword_t   ofs = (argt == ARGREG
                     ? wpm->cpustat.regs[op->reg1]
                     : op->args[0]);
    asmword_t *stk = (asmword_t *)wpm->cpustat.sp;

    *--stk = wpm->cpustat.fp;
    wpm->cpustat.sp -= ofs + 4;

    return;
}

void
opleave(opcode_t *op)
{
    asmword_t fp = memfetchl(wpm->cpustat.fp);
    
    wpm->cpustat.fp = memfetchl(fp);
    wpm->cpustat.sp = fp - 4;

    return;
}

#endif

/*
 * ret stack
 * ---------
 */
void
opret(opcode_t *op)
{
    asmword_t   fp = memfetchl(wpm->cpustat.fp);
    asmword_t   dest = memfetchl(wpm->cpustat.fp + 4);
    asmword_t  *stk = (asmword_t *)(&physmem[wpm->cpustat.fp + 8]);

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

#if (WPM)

void
oplmsw(opcode_t *op)
{
    uint_fast8_t argt = op->arg1t;
    asmword_t  msw = (argt == ARGREG
                    ? wpm->cpustat.regs[op->reg1]
                    : op->args[0]);

    wpm->cpustat.msw = msw;
    wpm->cpustat.pc += op->size << 2;

    return;
}

void
opsmsw(opcode_t *op)
{
    asmword_t msw = wpm->cpustat.msw;
    asmword_t dest = op->args[0];
    
    memstorel(msw, dest);
    
    return;
}

void
opreset(opcode_t *op)
{
    exit(1);

    return;
}

void
ophlt(opcode_t *op)
{
    wpm->shutdown = 1;

    return;
}

void
opnop(opcode_t *op)
{
    wpm->cpustat.pc += op->size << 2;
}

void
opbrk(opcode_t *op)
{
    asmuword_t pc = memfetchl(TRAPBRK << 2);

    if (pc) {
        wpm->cpustat.pc = pc;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

#endif /* WPM */

void
optrap(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    asmword_t  trap = (argt1 == ARGREG
                     ? wpm->cpustat.regs[op->reg1]
                     : memfetchl(op->args[0]));
    asmword_t  pc = memfetchl(trap << 2);

    if (pc) {
        asmword_t *stk = (asmword_t *)(&physmem[wpm->cpustat.sp]);
        asmword_t  fp = wpm->cpustat.sp - 8;
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

#if (WPM)

void
opcli(opcode_t *op)
{
    mtxtrylk(&intlk, 1);
    wpm->cpustat.pc += op->size << 2;

    return;
}

void
opsti(opcode_t *op)
{
    mtxunlk(&intlk, 1);
    wpm->cpustat.pc += op->size << 2;

    return;
}

#endif /* WPM */

void
opiret(opcode_t *op)
{
    asmword_t fp = memfetchl(wpm->cpustat.fp);
    asmword_t dest = memfetchl(wpm->cpustat.fp + 4);

    wpm->cpustat.fp = fp;
    wpm->cpustat.sp = fp;
    wpm->cpustat.pc = dest;

    return;
}

void
opthr(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    asmuword_t pc  = (argt1 == ARGREG
                    ? wpm->cpustat.regs[op->reg1]
                    : op->args[0]);

#if (WPMTRACE)
    fprintf(stderr, "thread == 0x%08x\n", pc);
#endif
    wpminitthr(pc);
    wpm->cpustat.pc += op->size << 2;

    return;
}

#if (WPM)

#if (PTHREAD)
/*
 * - opcmpswap can be used to create mutexes
 */
void
opcmpswap(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    asmword_t adr = (argt1 == ARGREG
                   ? wpm->cpustat.regs[op->reg1]
                   : memfetchl(op->args[0]));
    asmword_t val = (argt2 == ARGREG
                   ? wpm->cpustat.regs[op->reg2]
                   : (argt1 == ARGREG
                      ? memfetchl(op->args[0])
                      : memfetchl(op->args[1])));

    mtxlk(&atomlk, 1);
    while (memfetchl(adr)) {
        pthread_yield();
    }
    memstorel(val, adr);
    mtxunlk(&atomlk, 1);

    return;
}
#endif /* PTHREAD */

void
opinb(opcode_t *op)
{
}

void
opoutb(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    uint8_t  data = (argt1 == ARGREG
                    ? (uint8_t)wpm->cpustat.regs[op->reg1]
                    : (uint8_t)op->args[0]);
    uint8_t  port = (argt2 == ARGREG
                     ? (uint8_t)wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARGREG
                        ? (uint8_t)op->args[0]
                        : (uint8_t)op->args[1]));

#if (WPMDEBUG)
    fprintf(stderr, "OUTB: %x -> %x\n", data, port);
#endif
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

void
opinw(opcode_t *op)
{
}

void
opoutw(opcode_t *op)
{
}

void
opinl(opcode_t *op)
{
}

void
ophook(opcode_t *op)
{
    uint_fast8_t argt = op->arg1t;
    asmword_t id = (argt == ARGREG
                  ? op->reg1
                  : op->args[0]);
    hookfunc_t *hook = hookfunctab[id];

    if (hook) {
        hook(op);
    }
    wpm->cpustat.pc += op->size << 2;

    return;
}

void
opoutl(opcode_t *op)
{
}

#endif

void
wpmpzero(asmadr_t adr, asmuword_t size)
{
    asmuword_t  npg = roundup2(size, 1U << MINBKT) >> MINBKT;
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

#if (WPM)
/* TODO: this and other hooks need to use page tables */
static void
hookpzero(opcode_t *op)
{
    asmadr_t adr = wpm->cpustat.regs[0];
    asmuword_t sz = wpm->cpustat.regs[1];

    wpmpzero(adr, sz);

    return;
}

static void
hookpalloc(opcode_t *op)
{
    asmuword_t size = wpm->cpustat.regs[0];

    wpm->cpustat.regs[0] = mempalloc(size);

    return;
}

static void
hookpfree(opcode_t *op)
{
    asmadr_t adr = wpm->cpustat.regs[0];

    mempfree(adr);

    return;
}
#endif /* WPM */

