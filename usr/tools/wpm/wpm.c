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
#include <zas/zas.h>
#if (ZPC)
#include <zpc/asm.h>
#endif
#include <wpm/wpm.h>
#include <wpm/mem.h>
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
#else
typedef struct wpmopcode opcode_t;
#endif
typedef void ophandler_t(opcode_t *);
typedef void hookfunc_t(opcode_t *);

#if (WPMDB)
extern struct zasline  *linehash[];
#endif
extern struct zastoken *zastokenqueue;
extern struct zastoken *zastokentail;
extern unsigned long    zasinputread;
extern zasmemadr_t      _startadr;
extern wpmpage_t       *mempagetab;

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
void opmovd(opcode_t *op);
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

static void hookpzero(opcode_t *op);
static void hookpalloc(opcode_t *op);
static void hookpfree(opcode_t *op);

static void    memstoreq(int64_t src, wpmmemadr_t virt);
static int64_t memfetchq(wpmmemadr_t virt);
static void    memstorel(int32_t src, wpmmemadr_t virt);
static int32_t memfetchl(wpmmemadr_t virt);

uint8_t wpmopnargtab[WPMNASMOP]
= {
    0,
    1,  // NOT
    2,  // AND
    2,  // OR
    2,  // XOR
    2,  // SHL
    2,  // SHR
    2,  // SRHL
    2,  // ROR
    2,  // ROL
    1,  // INC
    1,  // DEC
    2,  // ADD
    2,  // SUB
    2,  // CMP
    2,  // MUL
    2,  // DIV
    2,  // MOD
    1,  // BZ
    1,  // BNZ
    1,  // BLT
    1,  // BLE
    1,  // BGT
    1,  // BGE
    1,  // BO
    1,  // BNO
    1,  // BC
    1,  // BNC
    1,  // POP
    1,  // PUSH
    2,  // MOV
    2,  // MOVB
    2,  // MOVW
    1,  // JMP
    1,  // CALL
    0,  // ENTER
    0,  // LEAVE
    0,  // RET
    1,  // LMSW
    1,  // SMSW
    0,  // RESET
    0,  // NOP
    0,  // HLT
    0,  // BRK
    1,  // TRAP
    0,  // CLI
    0,  // STI
    0,  // IRET
    1,  // THR
    2,  // CMPSWAP
    1,  // INB
    2,  // OUTB
    1,  // INW
    2,  // OUTW
    1,  // INL
    2,  // OUTL
    1   // HOOK
};

ophandler_t *wpmopfunctab[256] ALIGNED(PAGESIZE)
    = {
    NULL,
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

char *wpmopnametab[WPMNASMOP + 1]
= {
    NULL,
    "not",
    "and",
    "or",
    "xor",
    "shr",
    "shra",
    "shl",
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
    "hook",
    NULL
};

hookfunc_t *hookfunctab[256]
= {
    hookpzero,
    hookpalloc,
    hookpfree
};

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
wpminitthr(wpmmemadr_t pc)
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
wpmprintop(opcode_t *op)
{
    fprintf(stderr, "INST: %s, size %d, arg1t == %s, arg2t == %s, reg1 == %x, reg2 == %x, args[0] == %x", wpmopnametab[op->inst], op->size << 2, argnametab[op->arg1t], argnametab[op->arg2t], op->reg1, op->reg2, op->args[0]);
    if (op->arg2t) {
        fprintf(stderr, ", args[1] == %x", op->args[1]);
    }
    fprintf(stderr, "\n");
}

void *
wpmloop(void *cpustat)
{
    ophandler_t    *func;
    opcode_t       *op;
#if (WPMTRACE)
    int             i;
#endif
#if (WPMDB)
    struct zasline *line;
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
        if (op->inst == OPNOP) {
            wpm->cpustat.pc++;
        } else {
            wpm->cpustat.pc = roundup2(wpm->cpustat.pc, sizeof(wpmword_t));
            op = (opcode_t *)&physmem[wpm->cpustat.pc];
            func = wpmopfunctab[op->inst];
            if (func) {
#if (WPMDB)
                line = zasfindline(wpm->cpustat.pc);
                if (line) {
                    fprintf(stderr, "%s:%d:\t%s\n", line->file, line->num, line->data);
                }
#endif
                func(op);
            } else {
                fprintf(stderr, "illegal instruction, PC == %lx\n",
                        (long)wpm->cpustat.pc);
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

void
opnot(opcode_t *op)
{
    uint_fast8_t argt = op->arg1t;
    wpmword_t    dest = (argt == ARGREG
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
    wpmword_t    src = (argt1 == ARGREG
                        ? wpm->cpustat.regs[op->reg1]
                        : op->args[0]);
    wpmword_t    dest = (argt2 == ARGREG
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
    wpmword_t    src = (argt1 == ARGREG
                        ? wpm->cpustat.regs[op->reg1]
                        : op->args[0]);
    wpmword_t    dest = (argt2 == ARGREG
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
    wpmword_t    src = (argt1 == ARGREG
                        ? wpm->cpustat.regs[op->reg1]
                        : op->args[0]);
    wpmword_t    dest = (argt2 == ARGREG
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
    wpmword_t  cnt = (argt1 == ARGREG
                    ? wpm->cpustat.regs[op->reg1]
                    : (argt1 == ARGIMMED
                       ? op->args[0]
                       : memfetchl(op->args[0])));
    wpmword_t  dest = (argt2 == ARGREG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARGREG
                        ? op->args[0]
                        : op->args[1]));
#if (!USEASM)
    wpmuword_t sign = 0xffffffff >> cnt;
#endif
    
#if (WPMDEBUG)
    fprintf(stderr, "SHR: %x by %d\n", dest, cnt);
#endif
#if (USEASM)
    asmshr(cnt, dest);
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
    wpmword_t  cnt = (argt1 == ARGREG
                    ? wpm->cpustat.regs[op->reg1]
                    : (argt1 == ARGIMMED
                       ? op->args[0]
                       : memfetchl(op->args[0])));
    wpmword_t  dest = (argt2 == ARGREG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARGREG
                        ? op->args[0]
                        : op->args[1]));

#if (WPMDEBUG)
    fprintf(stderr, "SHRA: %x by %d\n", dest, cnt);
#endif
#if (!USEASM)
    wpmuword_t sign = dest & 0x80000000;
#endif

#if (USEASM)
    asmshra(cnt, dest);
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
    wpmword_t  cnt = (argt1 == ARGREG
                    ? wpm->cpustat.regs[op->reg1]
                    : (argt1 == ARGIMMED
                       ? op->args[0]
                       : memfetchl(op->args[0])));
    wpmword_t  dest = (argt2 == ARGREG
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
    wpmword_t  cnt = (argt1 == ARGREG
                    ? wpm->cpustat.regs[op->reg1]
                    : (argt1 == ARGIMMED
                       ? op->args[0]
                       : memfetchl(op->args[0])));
    wpmword_t  dest = (argt2 == ARGREG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARGREG
                        ? op->args[0]
                        : op->args[1]));
#if (!USEASM)
    wpmuword_t mask = 0xffffffff >> (32 - cnt);
    wpmuword_t bits = dest & mask;
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
    wpmword_t  cnt = (argt1 == ARGREG
                    ? wpm->cpustat.regs[op->reg1]
                    : (argt1 == ARGIMMED
                       ? op->args[0]
                       : memfetchl(op->args[0])));
    wpmword_t  dest = (argt2 == ARGREG
                     ? wpm->cpustat.regs[op->reg2]
                     : (argt1 == ARGREG
                        ? op->args[0]
                        : op->args[1]));
#if (!USEASM)
    wpmuword_t mask = 0xffffffff << (32 - cnt);
    wpmuword_t bits = dest & mask;
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
    wpmword_t  dest = (argt == ARGREG
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
    wpmword_t  dest = (argt == ARGREG
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
    wpmword_t    src = (argt1 == ARGREG
                        ? wpm->cpustat.regs[op->reg1]
                        : op->args[0]);
    wpmword_t    dest = (argt2 == ARGREG
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
    wpmword_t    src = (argt1 == ARGREG
                        ? wpm->cpustat.regs[op->reg1]
                        : op->args[0]);
    wpmword_t    dest = (argt2 == ARGREG
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
    wpmword_t    msw = wpm->cpustat.msw;
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    wpmword_t    src = (argt1 == ARGREG
                        ? wpm->cpustat.regs[op->reg1]
                        : op->args[0]);
    wpmword_t    dest = (argt2 == ARGREG
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
    wpmword_t    src = (argt1 == ARGREG
                        ? wpm->cpustat.regs[op->reg1]
                        : op->args[0]);
    wpmword_t    dest = (argt2 == ARGREG
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
    wpmword_t    src = (argt1 == ARGREG
                        ? wpm->cpustat.regs[op->reg1]
                        : op->args[0]);
    wpmword_t    dest = (argt2 == ARGREG
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
    wpmword_t    src = (argt1 == ARGREG
                        ? wpm->cpustat.regs[op->reg1]
                        : op->args[0]);
    wpmword_t    dest = (argt2 == ARGREG
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
        wpmword_t    dest = (argt == ARGREG
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
        wpmword_t    dest = (argt == ARGREG
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
        wpmword_t    dest = (argt == ARGREG
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
        wpmword_t    dest = (argt == ARGREG
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
        wpmword_t    dest = (argt == ARGREG
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
        wpmword_t    dest = (argt == ARGREG
                             ? wpm->cpustat.regs[op->reg1]
                             : op->args[0]);
        wpm->cpustat.pc = dest;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

void
opbo(opcode_t *op)
{
    if (wpm->cpustat.msw & MSW_OF) {
        uint_fast8_t argt = op->arg1t;
        wpmword_t    dest = (argt == ARGREG
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
        wpmword_t    dest = (argt == ARGREG
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
        wpmword_t    dest = (argt == ARGREG
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
        wpmword_t    dest = (argt == ARGREG
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
    wpmword_t    val;

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
    wpmword_t    src = (argt == ARGREG
                        ? wpm->cpustat.regs[op->reg1]
                        : op->args[0]);
    
    wpm->cpustat.sp -= 4;
    memstorel(src, wpm->cpustat.sp);
    wpm->cpustat.pc += op->size << 2;

    return;
}

void
opmov(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    uint64_t   reg1  = op->reg1;
    uint64_t   reg2  = op->reg2;
    wpmword_t    src = (argt1 == ARGREG
                        ? ((reg1 & REGINDEX)
                           ? memfetchl(wpm->cpustat.regs[reg1 & 0x0f]
                                       + op->args[0])
                           : (reg1 & REGINDIR
                              ? memfetchl(wpm->cpustat.regs[reg1 & 0x0f])
                              : wpm->cpustat.regs[reg1 & 0x0f]))
                        : (argt1 == ARGIMMED
                           ? op->args[0]
                           : memfetchl(op->args[0])));
    uint64_t   dest;

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

void
opmovd(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    uint32_t     reg1  = op->reg1;
    uint32_t     reg2  = op->reg2;
    int32_t      src = (argt1 == ARGREG
                        ? ((reg1 & REGINDEX)
                           ? memfetchl(wpm->cpustat.regs[reg1 & 0x0f]
                                       + op->args[0])
                           : (reg1 & REGINDIR
                              ? memfetchl(wpm->cpustat.regs[reg1 & 0x0f])
                              : wpm->cpustat.regs[reg1 & 0x0f]))
                        : (argt1 == ARGIMMED
                           ? op->args[0]
                           : memfetchl(op->args[0])));
    uint32_t   dest;
    
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

void
opmovb(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    wpmuword_t   reg1  = op->reg1;
    wpmuword_t   reg2  = op->reg2;
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
    wpmuword_t   dest;

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
    wpmuword_t   reg1  = op->reg1;
    wpmuword_t   reg2  = op->reg2;
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
    wpmuword_t   dest;
    
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

void
opjmp(opcode_t *op)
{
    uint_fast8_t argt = op->arg1t;
    wpmword_t  dest = (argt == ARGREG
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
    wpmword_t  dest = (argt == ARGREG
                     ? wpm->cpustat.regs[op->reg1]
                     : op->args[0]);
    wpmword_t *stk = (wpmword_t *)(&physmem[wpm->cpustat.sp]);
    wpmword_t  fp = wpm->cpustat.sp - 36;

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

void
openter(opcode_t *op)
{
    long  argt = op->arg1t;
    wpmword_t   ofs = (argt == ARGREG
                     ? wpm->cpustat.regs[op->reg1]
                     : op->args[0]);
    wpmword_t *stk = (wpmword_t *)wpm->cpustat.sp;

    *--stk = wpm->cpustat.fp;
    wpm->cpustat.sp -= ofs + 4;

    return;
}

void
opleave(opcode_t *op)
{
    wpmword_t fp = memfetchl(wpm->cpustat.fp);
    
    wpm->cpustat.fp = memfetchl(fp);
    wpm->cpustat.sp = fp - 4;

    return;
}

/*
 * ret stack
 * ---------
 */
void
opret(opcode_t *op)
{
    wpmword_t   fp = memfetchl(wpm->cpustat.fp);
    wpmword_t   dest = memfetchl(wpm->cpustat.fp + 4);
    wpmword_t  *stk = (wpmword_t *)(&physmem[wpm->cpustat.fp + 8]);

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

void
oplmsw(opcode_t *op)
{
    uint_fast8_t argt = op->arg1t;
    wpmword_t  msw = (argt == ARGREG
                    ? wpm->cpustat.regs[op->reg1]
                    : op->args[0]);

    wpm->cpustat.msw = msw;
    wpm->cpustat.pc += op->size << 2;

    return;
}

void
opsmsw(opcode_t *op)
{
    wpmword_t msw = wpm->cpustat.msw;
    wpmword_t dest = op->args[0];
    
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
    wpmuword_t pc = memfetchl(TRAPBRK << 2);

    if (pc) {
        wpm->cpustat.pc = pc;
    } else {
        wpm->cpustat.pc += op->size << 2;
    }

    return;
}

void
optrap(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    wpmword_t  trap = (argt1 == ARGREG
                     ? wpm->cpustat.regs[op->reg1]
                     : memfetchl(op->args[0]));
    wpmword_t  pc = memfetchl(trap << 2);

    if (pc) {
        wpmword_t *stk = (wpmword_t *)(&physmem[wpm->cpustat.sp]);
        wpmword_t  fp = wpm->cpustat.sp - 8;
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

void
opiret(opcode_t *op)
{
    wpmword_t fp = memfetchl(wpm->cpustat.fp);
    wpmword_t dest = memfetchl(wpm->cpustat.fp + 4);

    wpm->cpustat.fp = fp;
    wpm->cpustat.sp = fp;
    wpm->cpustat.pc = dest;

    return;
}

void
opthr(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    wpmuword_t pc  = (argt1 == ARGREG
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
void
opcmpswap(opcode_t *op)
{
    uint_fast8_t argt1 = op->arg1t;
    uint_fast8_t argt2 = op->arg2t;
    wpmword_t adr = (argt1 == ARGREG
                   ? wpm->cpustat.regs[op->reg1]
                   : memfetchl(op->args[0]));
    wpmword_t val = (argt2 == ARGREG
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
    wpmword_t id = (argt == ARGREG
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

void
wpmpzero(wpmmemadr_t adr, wpmuword_t size)
{
    wpmuword_t  npg = roundup2(size, 1U << MINBKT) >> MINBKT;
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
static void
hookpzero(opcode_t *op)
{
    wpmmemadr_t adr = wpm->cpustat.regs[0];
    wpmuword_t sz = wpm->cpustat.regs[1];

    wpmpzero(adr, sz);

    return;
}

static void
hookpalloc(opcode_t *op)
{
    wpmuword_t size = wpm->cpustat.regs[0];

    wpm->cpustat.regs[0] = mempalloc(size);

    return;
}

static void
hookpfree(opcode_t *op)
{
    wpmmemadr_t adr = wpm->cpustat.regs[0];

    mempfree(adr);

    return;
}

int
wpmmain(int argc, char *argv[])
{
    long        l;
#if (ZPC)
    zasmemadr_t adr = ZPCTEXTBASE;
#else
    zasmemadr_t adr = WPMTEXTBASE;
#endif
#if (ZASPROF)
    PROFTICK(tick);
#endif

    if (argc < 2) {
        fprintf(stderr, "usage: zas <file1> ...\n");

        exit(1);
    }
    zasinit();
    wpminitmem(MEMSIZE);
    wpminit();
    memset(physmem, 0, WPMTEXTBASE);
#if (ZASPROF)
    profstarttick(tick);
#endif
    for (l = 1 ; l < argc ; l++) {
#if (ZASBUF)
        zasreadfile(argv[l], adr, readbufcur);
#else
        zasreadfile(argv[l], adr);
#endif
        if (!zastokenqueue) {
            fprintf(stderr, "WARNING: no input in %s\n", argv[l]);
        } else {
            adr = zastranslate(adr);
#if (ZPC)
            zasresolve(ZPCTEXTBASE);
#else
            zasresolve(WPMTEXTBASE);
#endif
            zasremovesyms();
#if (ZASPROF)
            profstoptick(tick);
            fprintf(stderr, "%lld cycles to process %s\n",
                    proftickdiff(tick), argv[l]);
#endif        
        }
    }
    if (!zasinputread) {
        fprintf(stderr, "empty input\n");

        exit(1);
    }
    wpminitthr(_startadr);
    pause();

    /* NOTREACHED */
    exit(0);
}

int
main(int argc, char *argv[])
{
    exit(wpmmain(argc, argv));
}

