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
    int64_t sreg = op->src & ((1 << ZPUNREG) - 1);
    int64_t dreg = sreg;
    int64_t mask32 = INT64_C(0xffffffff);
    int64_t src = zpu->ctx.regs[sreg] & mask32;
    int64_t dest = ~src;

    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.pc += 4;

    return;
}

ZPUOPRET
zpuopand(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src & ((1 << ZPUNREG) - 1);
    int64_t dreg = op->dest & ((1 << ZPUNREG) - 1);
    int64_t mask32 = INT64_C(0xffffffff);
    int64_t src = zpu->ctx.regs[sreg] & mask32;
    int64_t dest = zpu->ctx.regs[dreg] & mask32;

    dest &= src;
    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.pc += 4;

    return;
}

ZPUOPRET
zpuopor(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src & ((1 << ZPUNREG) - 1);
    int64_t dreg = op->dest & ((1 << ZPUNREG) - 1);
    int64_t mask32 = INT64_C(0xffffffff);
    int64_t src = zpu->ctx.regs[sreg] & mask32;
    int64_t dest = zpu->ctx.regs[dreg] & mask32;

    dest |= src;
    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.pc += 4;

    return;
}

ZPUOPRET
zpuopxor(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src & ((1 << ZPUNREG) - 1);
    int64_t dreg = op->dest & ((1 << ZPUNREG) - 1);
    int64_t mask32 = INT64_C(0xffffffff);
    int64_t src = zpu->ctx.regs[sreg] & mask32;
    int64_t dest = zpu->ctx.regs[dreg] & mask32;

    dest ^= src;
    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.pc += 4;

    return;
}

ZPUOPRET
zpuopshr(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src & ((1 << ZPUNREG) - 1);
    int64_t dreg = op->dest & ((1 << ZPUNREG) - 1);
    int64_t mask32 = INT64_C(0xffffffff);
    int64_t src = zpu->ctx.regs[sreg] & mask32;
    int64_t dest = zpu->ctx.regs[dreg] & mask32;
    int64_t mask = INT64_C(1) << ((32 - src) - 1);

    dest >>= src;
    dest &= mask;
    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.pc += 4;

    return;
}

ZPUOPRET
zpuopshra(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src & ((1 << ZPUNREG) - 1);
    int64_t dreg = op->dest & ((1 << ZPUNREG) - 1);
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
    zpu->ctx.pc += 4;

    return;
}

ZPUOPRET
zpuopshl(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src & ((1 << ZPUNREG) - 1);
    int64_t dreg = op->dest & ((1 << ZPUNREG) - 1);
    int64_t mask32 = INT64_C(0xffffffff);
    int64_t src = zpu->ctx.regs[sreg] & mask32;
    int64_t dest = zpu->ctx.regs[dreg] & mask32;

    dest <<= src;
    dest &= mask32;
    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.pc += 4;

    return;
}

ZPUOPRET
zpulmsw(struct zpu *zpu, struct zpuop *op)
{
    long flg = op->src & ((1 << MSWNBIT) - 1);

    zpu->ctx.msw = flg;
}

ZPUOPRET
zpusmsw(struct zpu *zpu, struct zpuop *op)
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
    zpusetinst(OP_SHRA, OP_SHIFT, zpuopshra);
    zpusetinst(OP_SHL, OP_SHIFT, zpuopshl);
    zpusetinst(OP_ROR, OP_SHIFT, zpuopror);
    zpusetinst(OP_ROL, OP_SHIFT, zpuoprol);
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
        opadr = zpu->ctx.pc;
        op = (struct zpuop *)&zpu->core[opadr];
        inst = op->inst;
        /* dispatch instruction */
        func = zpu->functab[inst];
        if (func) {
            func(zpu, op);
        }
        msw = zpu->ctx.msw;
        if (msw & MSW_IF) {
            /* TODO: dispatch pending interrupts */
        }
    }

    return;
}

int
main(int argc, char *argv[])
{
    int   i;
    int   j;
    int   a;
    int   b;
    int **tab;
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

    zpuinit(&zpu);
    zpurun(&zpu);

    exit(zpu.exitflg);
}

