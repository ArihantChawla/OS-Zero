#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/mtx.h>
#if (ZPUPROF)
#include <zero/prof.h>
#include <zero/trix.h>
#endif
#include <zpu/zpu.h>

static zpuopfunc  *zpuopfunctab[ZPUNINST] ALIGNED(PAGESIZE);
static const char *zpuopnametab[ZPUNOP]
= {
    NULL,
    "not",
    "and",
    "or",
    "xor",
    "shr",
    "sar",
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
    "pusha",
    "mov",
#if 0
    "movb",
    "movw",
    "movq",
#endif
    "jmp",
    "call",
    "enter",
    "leave",
    "ret",
    "lmsw",
    "smsw"
};
static const long  zpunargtab[ZPUNOP]
= {
    0,  // not
    1,  // and
    2,  // or
    2,  // xor
    2,  // shr
    2,  // sar
    2,  // shl
    2,  // ror
    2,  // rol
    1,  // inc
    1,  // dec,
    2,  // add
    2,  // sub
    2,  // cmp
    2,  // mul
    2,  // div
    2,  // mod
    0,  // bz
    0,  // bnz
    0,  // blt
    0,  // ble
    0,  // bgt
    0,  // bge
    0,  // bno
    0,  // bo
    0,  // bc
    0,  // bnc
    0,  // pop
    0,  // push
    0,  // pusha
    2,  // mov
    2,  // movb
    2,  // movw
    2,  // movq
    1,  // jmp
    1,  // call
    1,  // enter
    1,  // leave
    1,  // ret
    1,  // lmsw
    1   // smsw
};
static const char *zpuregnametab[ZPUNREG]
= {
    "r0",
    "r1",
    "r2",
    "r3",
    "r4",
    "r5",
    "r6",
    "r7",
    "r8",
    "r9",
    "r10",
    "r11",
    "fp",
    "sp",
    "pc",
    "msw"
};
static struct zpu  zpu ALIGNED(PAGESIZE);

void
zpuinitcore(struct zpu *zpu)
{
    void *ptr;

    ptr = malloc(ZPUCORESIZE);
    if (!ptr) {
        fprintf(stderr, "ZPU: out of memory (core)\n");

        exit(1);
    }
    zpu->core = ptr;

    return;
}

ZPUOPRET
zpuopnot(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src;
    int64_t dreg = sreg;
    int64_t mask32 = INT64_C(0xffffffff);
    int64_t src = zpu->ctx.regs[sreg] & mask32;
    int64_t dest = ~src;

    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.regs[ZPUPCREG] += 4;

    return;
}

ZPUOPRET
zpuopand(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src;
    int64_t dreg = op->dest;
    int64_t mask32 = INT64_C(0xffffffff);
    int64_t src = zpu->ctx.regs[sreg] & mask32;
    int64_t dest = zpu->ctx.regs[dreg] & mask32;

    dest &= src;
    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.regs[ZPUPCREG] += 4;

    return;
}

ZPUOPRET
zpuopor(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src;
    int64_t dreg = op->dest;
    int64_t mask32 = INT64_C(0xffffffff);
    int64_t src = zpu->ctx.regs[sreg] & mask32;
    int64_t dest = zpu->ctx.regs[dreg] & mask32;

    dest |= src;
    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.regs[ZPUPCREG] += 4;

    return;
}

ZPUOPRET
zpuopxor(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src;
    int64_t dreg = op->dest;
    int64_t mask32 = INT64_C(0xffffffff);
    int64_t src = zpu->ctx.regs[sreg] & mask32;
    int64_t dest = zpu->ctx.regs[dreg] & mask32;

    dest ^= src;
    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.regs[ZPUPCREG] += 4;

    return;
}

ZPUOPRET
zpuopshr(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src;
    int64_t dreg = op->dest;
    int64_t mask32 = INT64_C(0xffffffff);
    int64_t src = zpu->ctx.regs[sreg] & mask32;
    int64_t dest = zpu->ctx.regs[dreg] & mask32;
    int64_t mask = INT64_C(1) << ((32 - src) - 1);

    dest >>= src;
    dest &= mask;
    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.regs[ZPUPCREG] += 4;

    return;
}

ZPUOPRET
zpuopsar(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src;
    int64_t dreg = op->dest;
    int64_t mask32 = INT64_C(0xffffffff);
    int64_t src = zpu->ctx.regs[sreg] & mask32;
    int64_t dest = zpu->ctx.regs[dreg] & mask32;
    int64_t mask = ~INT64_C(0);

    dest >>= src;
    if (src & (INT64_C(1) << 31)) {
        mask -= (INT64_C(1) << src) - 1;
        dest |= mask;
    } else {
        mask -= (INT64_C(1) << (32 - src)) - 1;
        dest &= mask;
    }
    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.regs[ZPUPCREG] += 4;

    return;
}

ZPUOPRET
zpuopshl(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src;
    int64_t dreg = op->dest;
    int64_t mask32 = INT64_C(0xffffffff);
    int64_t src = zpu->ctx.regs[sreg] & mask32;
    int64_t dest = zpu->ctx.regs[dreg] & mask32;

    dest <<= src;
    dest &= mask32;
    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.regs[ZPUPCREG] += 4;

    return;
}

ZPUOPRET
zpuoplmsw(struct zpu *zpu, struct zpuop *op)
{
    long flg = op->src & ((1 << MSWNBIT) - 1);

    zpu->ctx.regs[ZPUMSWREG] |= flg;
}

ZPUOPRET
zpuopsmsw(struct zpu *zpu, struct zpuop *op)
{
    ;
}

void
zpuinitinst(struct zpu *zpu)
{
    /* logical bitwise instructions */
    zpusetinst(OP_NOT, OP_LOGIC, zpuopnot);
    zpusetinst(OP_AND, OP_LOGIC, zpuopand);
    zpusetinst(OP_OR, OP_LOGIC, zpuopor);
    zpusetinst(OP_XOR, OP_LOGIC, zpuopxor);
    /* shift instructions */
    zpusetinst(OP_SHR, OP_SHIFT, zpuopshr);
    zpusetinst(OP_SAR, OP_SHIFT, zpuopsar);
    zpusetinst(OP_SHL, OP_SHIFT, zpuopshl);
#if 0
    zpusetinst(OP_ROR, OP_SHIFT, zpuopror);
    zpusetinst(OP_ROL, OP_SHIFT, zpuoprol);
#endif
    /* special-register instructions */
    zpusetinst(OP_LMSW, OP_SREG, zpuoplmsw);
    zpusetinst(OP_SMSW, OP_SREG, zpuopsmsw);
    zpu->functab = zpuopfunctab;
}

void
zpuinit(struct zpu *zpu)
{
    zpuinitinst(zpu);
    zpuinitcore(zpu);

    return;
}

/* virtual machine main loop */
void
zpurun(struct zpu *zpu)
{
    long          opadr;
    struct zpuop *op;
    long          inst;
    zpuopfunc    *func;
    long          msw;

    while (!zpu->exitflg) {
        /* get instruction */
        opadr = zpu->ctx.regs[ZPUPCREG];
        op = (struct zpuop *)&zpu->core[opadr];
        inst = op->inst;
        /* dispatch instruction */
        func = zpu->functab[inst];
        if (func) {
            func(zpu, op);
        }
        msw = zpu->ctx.regs[ZPUMSWREG];
        if (msw & MSW_IF) {
            /* TODO: dispatch pending interrupts */
        }
    }

    return;
}

/* disassemble and print instruction op */
void
zpudisasm(struct zpu *zpu, struct zpuop *op)
{
    long src = op->src;
    long dest = op->dest;
    long argsz = op->argsz;
    long nimm = 0;
    long long arg;

    fprintf(stderr, "%lu:\t%s\t", (uint8_t *)op - zpu->core,
            zpuopnametab[op->inst]);
    if (op->sflg & ARG_INDIR) {
        fprintf(stderr, "*");
        if (op->sflg & ARG_REG) {
            fprintf(stderr, "%s", zpuregnametab[src]);
        } else {
            /* invalid argument */
        }
    } else if (op->sflg & ARG_INDEX) {
        nimm++;
        if (op->argsz < 2) {
            arg = op->args[0];
            fprintf(stderr, "0x%llx", arg);
        } else {
            arg = ((long *)&op->args)[0];
            fprintf(stderr, "0x%llx", arg);
        }
        fprintf(stderr, "(%s)", zpuregnametab[src]);
    } else if (op->sflg & ARG_ADR) {
        nimm++;
        arg = op->args[0];
        if (argsz < 2) {
            arg = op->args[0];
            fprintf(stderr, "0x%llx", arg);
        } else {
            arg = ((long *)&op->args)[0];
            fprintf(stderr, "0x%llx", arg);
        }
    } else if (op->sflg & ARG_REG) {
        fprintf(stderr, "%s", zpuregnametab[src]);
    }
    if (zpunargtab[op->inst] == 2) {
        if (op->dflg & ARG_INDIR) {
            fprintf(stderr, ", *");
            if (op->dflg & ARG_REG) {
                fprintf(stderr, "%s", zpuregnametab[dest]);
            } else {
                /* invalid argument */
            }
        } else if (op->dflg & ARG_INDEX) {
            nimm++;
            if (op->argsz < 2) {
                arg = op->args[0];
                fprintf(stderr, ", 0x%llx", arg);
            } else {
            arg = ((long *)&op->args)[0];
                fprintf(stderr, ", 0x%llx", arg);
            }
            fprintf(stderr, "(%s)", zpuregnametab[dest]);
        } else if (op->dflg & ARG_ADR) {
            nimm++;
            if (argsz < 2) {
                arg = op->args[0];
                fprintf(stderr, ", 0x%llx", arg);
            } else {
                arg = ((long *)&op->args)[0];
                fprintf(stderr, ", 0x%llx", arg);
            }
        }
    } else if (op->dflg & ARG_REG) {
        fprintf(stderr, "%s", zpuregnametab[dest]);
    }
    fprintf(stderr, "\n");

    return;
}

int
main(int argc, char *argv[])
{
    int            i;
    int            j;
    int            a;
    int            b;
    int          **tab;
    struct zpuop   op;
    PROFDECLCLK(clk);

    tab = malloc(65536 * sizeof(long *));
    profstartclk(clk);
    for (i = 1 ; i < 65536 ; i++) {
        tab[i] = malloc(65536 * sizeof(long));
        for (j = 1 ; j < 256 ; j++) {
//            fprintf(stderr, "%ld : %ld\n", i, j);
            a = abs32(i - j);
            b = abs(i - j);
            if (a != abs(i - j)) {
                fprintf(stderr, "%d: %d != %d\n", i - j,
                        a, b);
            }
            tab[i][j] = gcdu32(i, j);
        }
        free(tab[i]);
    }
    profstopclk(clk);
    fprintf(stderr, "GCD: %ld microseconds\n", profclkdiff(clk));

    fprintf(stderr, "GCD(%d, %d) == %d\n", 100, 1000, gcdu32(100, 1000));
    fflush(stderr);

    op.unit = 0;
    op.inst = OP_MOV;
    op.sflg = ARG_REG;
    op.src = 1;
    op.dflg = ARG_INDEX;
    op.dest = 2;
    op.argsz = 1;
    *((int *)&op.args[0]) = 0x888;
    zpudisasm(&zpu, &op);

    zpuinit(&zpu);
    zpurun(&zpu);

    exit(zpu.exitflg);
}

