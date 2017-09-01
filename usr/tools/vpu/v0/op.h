#ifndef __V0_OP_H__
#define __V0_OP_H__

/* FIXME: make this file work */

#include <v0/conf.h>
#include <stddef.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <zero/param.h>
#include <zero/cdefs.h>
#include <zero/fastudiv.h>
#include <v0/mach.h>

#define V0_OP_INVAL            NULL
#if !defined(__GNUC__)
#define V0OP_T FASTCALL INLINE void *
#else
#define V0OP_T INLINE          void *
#endif

#if defined(V0_GAME)
static long long               v0speedcnt;

#define v0addspeedcnt(n)       (v0speedcnt += (n))

#else

#define v0addspeedcnt(n)

#endif

#define v0doxcpt(xcpt)                                                  \
    do {                                                                \
        fprintf(stderr, "%s\n", #xcpt);                                 \
                                                                        \
        exit(xcpt);                                                     \
    } while (0)

/* immediate source operand */
#define v0getsrc(vm, op)                                                \
    (((op)->adr == V0_DIR_ADR)                                          \
     ? ((op)->arg[0].i32 & (1 << ((op)->parm * CHAR_BIT)))              \
     : ((vm)->genregs[(op)->reg1]))

/* count for shl, ahr, sar */
#define v0getcnt(vm, op)                                                \
    (((op)->adr == V0_DIR_ADR)                                          \
     ? ((op)->val)                                                      \
     : ((vm)->genregs[(op)->reg1]))

/* operand address for psh */
#define v0getsrcadr(vm, op)                                             \
    (((op)->adr == V0_DIR_ADR)                                          \
     ? ((v0reg *)&(vm)->mem[(op)->arg[0].adr])                          \
     : (&vm->genregs[op->reg1]))

/*
 * V0_REG_ADR -> *reg
 * V0_NDX_ADR -> reg[op->val << op->parm]
 * V0_DIR_ADR -> op->arg.adr
 */
#define v0getadr(vm, op, reg)                                           \
    (((op)->adr == V0_REG_ADR)                                          \
     ? ((vm)->genregs[(op)->reg])                                       \
     : (((op)->adr == V0_NDX_ADR)                                       \
        ? ((vm)->genregs[(op)->reg] + ((op)->val << (op)->parm))        \
        : (((op)->adr == V0_DIR_ADR)                                    \
           ? ((op)->arg[0].adr)                                         \
           : 0x00000000)))

/*
 * V0_REG_ADR -> *reg
 * V0_NDX_ADR -> reg[op->val << op->parm]
 * V0_DIR_ADR -> op->arg.adr
 * V0_PC_ADR  -> pc[op->val << op->parm]
 */
#define v0getbranch(vm, op, reg)                                        \
    (((op)->adr == V0_REG_ADR)                                          \
     ? ((vm)->genregs[(op)->reg])                                       \
     : (((op)->adr == V0_NDX_ADR)                                       \
        ? ((vm)->genregs[(op)->reg] + ((op)->val << (op)->parm))        \
        : (((op)->adr == V0_DIR_ADR)                                    \
           ? ((op)->arg[0].adr)                                         \
           : ((vm)->genregs[V0_PC_REG] + ((op)->val << (op)->parm)))))

static V0OP_T
v0nop(struct v0 *vm, void *ptr)
{
    v0reg  pc = vm->sysregs[V0_PC_REG];
    void  *opadr;

    pc += sizeof(struct v0op);
    v0addspeedcnt(1);
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0not(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg       *dptr = &vm->genregs[op->reg1];
    v0reg        src = *dptr;
    v0reg        dest;

    dest = ~src;
    pc += sizeof(struct v0op);
    v0addspeedcnt(1);
    *dptr = dest;
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0and(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        src = v0getsrc(vm, op);
    v0reg        dest = *dptr;

    dest &= src;
    pc += sizeof(struct v0op);
    v0addspeedcnt(2);
    *dptr = dest;
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0or(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        src = v0getsrc(vm, op);
    v0reg        dest = *dptr;

    dest |= src;
    pc += sizeof(struct v0op);
    v0addspeedcnt(2);
    *dptr = dest;
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0xor(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        src = v0getsrc(vm, op);
    v0reg        dest = *dptr;

    dest ^= src;
    pc += sizeof(struct v0op);
    v0addspeedcnt(2);
    *dptr = dest;
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0shl(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        src = v0getcnt(vm, op);
    v0reg        dest = *dptr;

    dest <<= src;
    pc += sizeof(struct v0op);
    v0addspeedcnt(3);
    *dptr = dest;
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0shr(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        dest = *dptr;
    v0reg        src = v0getcnt(vm, op);
    v0reg        fill = ~((v0reg)0) >> src;

    dest >>= src;
    pc += sizeof(struct v0op);
    v0addspeedcnt(3);
    dest &= fill;
    *dptr = dest;
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0sar(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        src = v0getcnt(vm, op);
    v0reg        dest = *dptr;
    v0reg        mask = ~(v0reg)0;
#if (WORDSIZE == 8)
    v0reg        fill = (((dest) & (INT64_C(1) << 63))
                        ? (mask >> (64 - src))
                        : 0);
#else
    v0reg        fill = (((dest) & (INT32_C(1) << 31))
                        ? (mask >> (32 - src))
                        : 0);
#endif

    dest >>= src;
    fill = -fill << (CHAR_BIT * sizeof(v0reg) - src);
    pc += sizeof(struct v0op);
    v0addspeedcnt(3);
    dest &= fill;
    *dptr = dest;
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0inc(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg       *dptr = &vm->genregs[op->reg1];
    v0reg        src = *dptr;

    src++;
    pc += sizeof(struct v0op);
    v0addspeedcnt(2);
    *dptr = src;
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0dec(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg      *dptr = &vm->genregs[op->reg1];
    v0reg       src = *dptr;

    src--;
    pc += sizeof(struct v0op);
    v0addspeedcnt(2);
    *dptr = src;
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0add(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg      *dptr = &vm->genregs[op->reg2];
    v0reg       src = v0getsrc(vm, op);
    v0reg       dest = *dptr;
    v0reg       res;

    pc += sizeof(struct v0op);
    res = src + dest;
    v0addspeedcnt(2);
    if (res < dest) {
        v0setof(vm);
    }
    *dptr = dest;
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

/* FIXME: set carry-bit */
static V0OP_T
v0adc(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg      *dptr = &vm->genregs[op->reg2];
    v0reg       src = v0getsrc(vm, op);
    v0reg       dest = *dptr;
    v0reg       res;

    pc += sizeof(struct v0op);
    res = src + dest;
    v0addspeedcnt(2);
    if (res < dest) {
        v0setof(vm);
    }
    *dptr = dest;
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0sub(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        src = v0getsrc(vm, op);
    v0reg        dest = *dptr;

    pc += sizeof(struct v0op);
    dest -= src;
    v0addspeedcnt(2);
    *dptr = dest;
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0sbb(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        src = v0getsrc(vm, op);
    v0reg        dest = *dptr;

    pc += sizeof(struct v0op);
    dest -= src;
    v0addspeedcnt(2);
    opadr = &vm->mem[pc];
    *dptr = dest;
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0cmp(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        src = v0getsrc(vm, op);
    v0reg        dest = *dptr;

    pc += sizeof(struct v0op);
    dest -= src;
    v0addspeedcnt(2);
    v0clrmsw(vm);
    if (!dest) {
        v0setzf(vm);
    } else if (dest < 0) {
        v0setcf(vm);
    }
    opadr = &vm->mem[pc];
    *dptr = dest;
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0mul(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        src = v0getsrc(vm, op);
    v0reg        dest = *dptr;

    pc += sizeof(struct v0op);
    dest *= src;
    v0addspeedcnt(8);
    *dptr = dest;
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0div(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        src = v0getsrc(vm, op);
    v0reg        dest = *dptr;

    pc += sizeof(struct v0op);
    if (!src) {
        v0doxcpt(V0_DIV_BY_ZERO);
    } else if (src <= 0xffff) {
        dest = fastuf16divuf16(dest, src, vm->divu16tab);
    } else {
        dest /= src;
    }
    v0addspeedcnt(32);
    *dptr = dest;
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0rem(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        src = v0getsrc(vm, op);
    v0reg        dest = *dptr;

    pc += sizeof(struct v0op);
    dest %= src;
    v0addspeedcnt(32);
    *dptr = dest;
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

#if 0
static V0OP_T
v0crm(struct v0 *vm, void *ptr)
{
    ; // TODO: hack this a'la ridiculous_fish?
}
#endif

static V0OP_T
v0jmp(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg        dest = v0getbranch(vm, op, reg1);

    pc = dest;
    v0addspeedcnt(8);
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

/* call subroutine */
/*
 * stack frame after call
 * ----------------------
 * r7
 * r6
 * r5
 * r4
 * r3
 * r2
 * r1
 * r0
 * ln <- sp
 */
static V0OP_T
v0call(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0ureg       sp = vm->sysregs[V0_SP_REG];
    v0ureg       ln = vm->sysregs[V0_LN_REG];
    v0ureg       dest = v0getadr(vm, op, reg1);
    v0reg       *sptr = &vm->genregs[V0_R0_REG];
    v0reg       *dptr = (v0reg *)&vm->mem[sp];
    v0ureg       newsp = sp - (V0_NSAVEREG + 1) * sizeof(v0reg);

    v0addspeedcnt(16);
    dptr = (v0reg *)&vm->mem[newsp];
    pc += sizeof(struct v0op);
    vm->sysregs[V0_SP_REG] = newsp;
    if (op->adr == V0_DIR_ADR && !op->val) {
        pc += sizeof(union v0oparg);
    }
    dptr[0] = ln;
    dptr[1] = sptr[0];
    dptr[2] = sptr[1];
    dptr[3] = sptr[2];
    dptr[4] = sptr[3];
    dptr[5] = sptr[4];
    dptr[6] = sptr[5];
    dptr[7] = sptr[6];
    dptr[8] = sptr[7];
    opadr = &vm->mem[dest];
    vm->sysregs[V0_LN_REG] = pc;
    vm->sysregs[V0_PC_REG] = dest;

    return opadr;
}

/* create subroutine stack-frame;
 * - push caller-save registers
 * - push frame pointer
 * - allocate room for local variables on stack
 */
/*
 * stack after enter
 * -----------------
 * ln
 * oldfp <- fp
 * var0
 * ...
 * varN <- sp
 */
static V0OP_T
v0enter(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0ureg       sp = vm->sysregs[V0_SP_REG];
    v0ureg       fp = vm->sysregs[V0_FP_REG];
    v0reg        nvar = op->val;
    v0ureg      *dptr = (v0ureg *)&vm->mem[sp];

    v0addspeedcnt(8);
    dptr--;
    sp -= sizeof(v0reg);
    *dptr = fp;
    vm->sysregs[V0_FP_REG] = sp;
    pc += sizeof(struct v0op);
    opadr = &vm->mem[pc];
    sp -=  nvar * sizeof(v0reg);
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

/* destroy subroutine stack-frame */
/*
 * stack after leave
 * -----------------
 * ln <- sp
 */
static V0OP_T
v0leave(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    v0ureg       fp = vm->sysregs[V0_FP_REG];
    v0ureg       sp;
    v0reg       *sptr;

    v0addspeedcnt(4);
    sp = fp;
    sptr = (v0reg *)&vm->mem[fp];
    sp += sizeof(v0reg);
    fp = *sptr;
    vm->sysregs[V0_SP_REG] = sp;
    opadr = &vm->mem[pc];
    vm->sysregs[V0_FP_REG] = fp;
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

/* return from subroutine;
 * pop old link-register value from stack, then adjust stack pointer
 */
static V0OP_T
v0ret(struct v0 *vm, void *ptr)
{
    void        *opadr;
    struct v0op *op = ptr;
    v0ureg       sp = vm->sysregs[V0_SP_REG];
    v0ureg       ln = vm->sysregs[V0_LN_REG];
    v0ureg       newln;
    v0ureg       nstk = op->val;
    v0ureg      *sptr = (v0ureg *)&vm->mem[sp];

    v0addspeedcnt(4);
    newln = *sptr;
    sp += nstk * sizeof(v0reg);
    opadr = &vm->mem[ln];
    vm->sysregs[V0_SP_REG] = sp;
    vm->sysregs[V0_LN_REG] = newln;
    vm->sysregs[V0_PC_REG] = ln;

    return opadr;
}

static V0OP_T
v0bz(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg        dest = v0getbranch(vm, op, reg1);

    if (v0zfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(8);
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0bnz(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg        dest = v0getbranch(vm, op, reg1);

    if (!v0zfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(8);
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0bc(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg        dest = v0getbranch(vm, op, reg1);

    if (v0cfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(8);
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0bnc(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg        dest = v0getbranch(vm, op, reg1);

    if (!v0cfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(8);
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0bo(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg        dest = v0getbranch(vm, op, reg1);

    if (v0ofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(8);
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0bno(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg        dest = v0getbranch(vm, op, reg1);

    if (!v0ofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(8);
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0blt(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg        dest = v0getbranch(vm, op, reg1);

    if (!v0ofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(8);
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0ble(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg        dest = v0getbranch(vm, op, reg1);

    pc += sizeof(struct v0op);
    if (!v0ofset(vm) || v0zfset(vm)) {
        ptr = &vm->mem[dest];
    } else {
        ptr = &vm->mem[pc];
    }
    v0addspeedcnt(8);
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0bgt(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg        dest = v0getbranch(vm, op, reg1);

    if (v0ofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(8);
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0bge(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg        dest = v0getbranch(vm, op, reg1);

    pc += sizeof(struct v0op);
    if (v0ofset(vm) || v0zfset(vm)) {
        ptr = &vm->mem[dest];
    } else {
        ptr = &vm->mem[pc];
    }
    v0addspeedcnt(8);
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

/* TODO: address translations for v0ldr, v0st */

static V0OP_T
v0ldr(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg        adr = v0getadr(vm, op, reg1);
    v0reg        src;
    v0ureg       usrc;
    v0reg       *dptr = &vm->genregs[op->reg2];

    if (!adr) {
        v0doxcpt(V0_INV_MEM_ADR);
    }
    pc += sizeof(struct v0op);
    v0addspeedcnt(2);
    if (op->adr == V0_DIR_ADR) {
        pc += sizeof(union v0oparg);
        v0addspeedcnt(2);
    }
    if (op->val & V0_SIGNED_BIT) {
        switch (op->parm) {
            case 0:
                src = *(int8_t *)&vm->mem[adr];

                break;
            case 1:
                src = *(int16_t *)&vm->mem[adr];

                break;
            case 2:
                src = *(int32_t *)&vm->mem[adr];

                break;
            case 3:
                v0doxcpt(V0_INV_MEM_READ);

                break;
        }
        *dptr = src;
    } else {
        switch (op->parm) {
            case 0:
                usrc = *(uint8_t *)&vm->mem[adr];

                break;
            case 1:
                usrc = *(uint16_t *)&vm->mem[adr];

                break;
            case 2:
                usrc = *(uint32_t *)&vm->mem[adr];

                break;
            case 3:
                v0doxcpt(V0_INV_MEM_READ);

                break;
        }
        *(v0ureg *)dptr = usrc;
    }
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0str(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0reg        adr = v0getadr(vm, op, reg2);
    v0reg        src;
    v0ureg       usrc;

    if (!adr) {
        v0doxcpt(V0_INV_MEM_ADR);
    }
    pc += sizeof(struct v0op);
    v0addspeedcnt(2);
    if (op->adr == V0_DIR_ADR) {
        pc += sizeof(union v0oparg);
        v0addspeedcnt(2);
    }
    if (op->val & V0_SIGNED_BIT) {
        switch (op->parm) {
            case 0:
                src = *(int8_t *)&vm->mem[adr];
                *(int8_t *)&vm->mem[adr] = (int8_t)src;

                break;
            case 1:
                src = *(int16_t *)&vm->mem[adr];
                *(int16_t *)&vm->mem[adr] = (int16_t)src;

                break;
            case 2:
                src = *(int32_t *)&vm->mem[adr];
                *(int32_t *)&vm->mem[adr] = (int32_t)src;

                break;
            case 3:
                v0doxcpt(V0_INV_MEM_WRITE);

                break;
        }
    } else {
        switch (op->parm) {
            case 0:
                usrc = *(uint8_t *)&vm->mem[adr];
                *(uint8_t *)&vm->mem[adr] = (uint8_t)usrc;

                break;
            case 1:
                usrc = *(uint16_t *)&vm->mem[adr];
                *(uint16_t *)&vm->mem[adr] = (uint16_t)usrc;

                break;
            case 2:
                usrc = *(int32_t *)&vm->mem[adr];
                *(uint32_t *)&vm->mem[adr] = (uint32_t)usrc;

                break;
            case 3:
                v0doxcpt(V0_INV_MEM_WRITE);

                break;
        }
    }
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0psh(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0ureg       sp = vm->sysregs[V0_SP_REG];
    v0reg       *src = v0getsrcadr(vm, op);
    v0reg       *dest = (v0reg *)&vm->mem[sp];

    sp -= sizeof(v0reg);
    pc += sizeof(struct v0op);
    v0addspeedcnt(2);
    if (op->adr == V0_DIR_ADR) {
        pc += sizeof(union v0oparg);
        v0addspeedcnt(2);
    }
    vm->sysregs[sp] = sp;
    *dest = *src;
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0pop(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0ureg       sp = vm->sysregs[V0_SP];
    v0reg       *src = (v0reg *)&vm->mem[sp];
    v0reg       *dest = &vm->genregs[op->reg1];

    pc += sizeof(struct v0op);
    sp += sizeof(v0reg);
    *dest = *src;
    v0addspeedcnt(2);
    vm->sysregs[sp] = sp;
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0psha(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    v0ureg       sp = vm->sysregs[V0_SP_REG];
    v0reg       *src = &vm->genregs[V0_R0_REG];
    v0reg       *dest = (v0reg *)&vm->mem[sp];
    v0ureg       newsp = sp - V0_NGENREG * sizeof(v0reg);
    v0reg        ndx;

    v0addspeedcnt(16);
    pc += sizeof(struct v0op);
    dest -= V0_NGENREG;
    vm->sysregs[sp] = newsp;
    for (ndx = 0 ; ndx < (V0_NGENREG >> 3); ndx++) {
        dest[0] = src[0];
        dest[1] = src[1];
        dest[2] = src[2];
        dest[3] = src[3];
        dest[4] = src[4];
        dest[5] = src[5];
        dest[6] = src[6];
        dest[7] = src[7];
        sp += 8;
        dest += 8;
        src += 8;
    }
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0popa(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    v0ureg       sp = vm->sysregs[V0_SP];
    v0reg       *dest = &vm->genregs[V0_R0_REG];
    v0reg       *src = (v0reg *)&vm->mem[sp];
    v0ureg       newsp = sp + V0_NGENREG * sizeof(v0reg);
    v0reg        ndx;

    v0addspeedcnt(16);
    pc += sizeof(struct v0op);
    vm->sysregs[sp] = newsp;
    for (ndx = 0 ; ndx < (V0_NGENREG >> 3); ndx++) {
        dest[0] = src[0];
        dest[1] = src[1];
        dest[2] = src[2];
        dest[3] = src[3];
        dest[4] = src[4];
        dest[5] = src[5];
        dest[6] = src[6];
        dest[7] = src[7];
        sp += 8;
        dest += 8;
        src += 8;
    }
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

/* push callee save registers r0..r7 */
static V0OP_T
v0pshc(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    v0ureg       sp = vm->sysregs[V0_SP_REG];
    v0reg       *src = &vm->genregs[V0_R0_REG];
    v0reg       *dest = (v0reg *)&vm->mem[sp];
    v0ureg       newsp = sp - V0_NSAVEREG * sizeof(v0reg);

    v0addspeedcnt(8);
    pc += sizeof(struct v0op);
    dest -= V0_NSAVEREG;
    vm->sysregs[sp] = newsp;
    dest[0] = src[0];
    dest[1] = src[1];
    dest[2] = src[2];
    dest[3] = src[3];
    dest[4] = src[4];
    dest[5] = src[5];
    dest[6] = src[6];
    dest[7] = src[7];
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

/* pop callee save registers r0..r7 */
static V0OP_T
v0popc(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    v0ureg       sp = vm->sysregs[V0_SP_REG];
    v0reg       *src = &vm->genregs[V0_R0_REG];
    v0reg       *dest = (v0reg *)&vm->mem[sp];
    v0ureg       newsp = sp - V0_NSAVEREG * sizeof(v0reg);

    v0addspeedcnt(8);
    pc += sizeof(struct v0op);
    vm->sysregs[sp] = newsp;
    dest[0] = src[0];
    dest[1] = src[1];
    dest[2] = src[2];
    dest[3] = src[3];
    dest[4] = src[4];
    dest[5] = src[5];
    dest[6] = src[6];
    dest[7] = src[7];
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0pshm(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0ureg       sp = vm->sysregs[V0_SP_REG];
    v0reg       *src = &vm->genregs[V0_R0_REG];
    v0reg       *dest = (v0reg *)&vm->mem[sp];
    v0ureg       newsp = sp - V0_NGENREG * sizeof(v0reg);
    v0reg        map = op->arg[0].rmap;
    v0reg        mask;
    v0reg        ndx;

    mask = 0x01;
    v0addspeedcnt(32);
    pc += sizeof(struct v0op);
    vm->sysregs[sp] = newsp;
    for (ndx = 0 ; ndx < (V0_NGENREG >> 3) && (mask) ; ndx++) {
        if (map & mask) {
            dest[0] = src[0];
        }
        if (map & (mask >> 1)) {
            dest[-1] = src[1];
        }
        if (map & (mask >> 2)) {
            dest[-2] = src[2];
        }
        if (map & (mask >> 3)) {
            dest[-3] = src[3];
        }
        if (map & (mask >> 4)) {
            dest[-4] = src[4];
        }
        if (map & (mask >> 5)) {
            dest[-5] = src[5];
        }
        if (map & (mask >> 6)) {
            dest[-6] = src[6];
        }
        if (map & (mask >> 7)) {
            dest[-7] = src[7];
        }
        dest -= 8;
        mask >>= 8;
    }
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0popm(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    v0ureg       sp = vm->sysregs[V0_SP_REG];
    v0reg       *dest = &vm->genregs[V0_R0_REG];
    v0reg        map = op->arg[0].rmap;
    v0reg       *src = (v0reg *)&vm->mem[sp];
    v0ureg       newsp = sp - V0_NGENREG * sizeof(v0reg);
    v0reg        mask;
    v0reg        ndx;

    mask = 0x01;
    v0addspeedcnt(32);
    pc += sizeof(struct v0op);
    vm->sysregs[sp] = newsp;
    for (ndx = 0 ; ndx < (V0_NGENREG >> 3) && (mask) ; ndx++) {
        if (map & mask) {
            dest[0] = src[0];
        }
        if (map & (mask << 1)) {
            dest[-1] = src[1];
        }
        if (map & (mask << 2)) {
            dest[-2] = src[2];
        }
        if (map & (mask << 3)) {
            dest[-3] = src[3];
        }
        if (map & (mask << 4)) {
            dest[-4] = src[4];
        }
        if (map & (mask << 5)) {
            dest[-5] = src[5];
        }
        if (map & (mask << 6)) {
            dest[-6] = src[6];
        }
        if (map & (mask << 7)) {
            dest[-7] = src[7];
        }
        dest += 8;
        mask <<= 8;
    }
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0ior(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    uint16_t     port = op->val;
    uint16_t     reg = op->reg1;
    v0iofunc_t  *func = vm->iovec[port].rdfunc;

    v0addspeedcnt(8);
    pc += sizeof(struct v0op);
    if (func) {
        func(vm, port, reg);
    } else {
        v0doxcpt(V0_INV_IO_READ);
    }
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

static V0OP_T
v0iow(struct v0 *vm, void *ptr)
{
    v0reg        pc = vm->sysregs[V0_PC_REG];
    void        *opadr;
    struct v0op *op = ptr;
    uint16_t     port = op->val;
    v0reg        val = v0getsrc(vm, op);
    v0iofunc_t  *func = vm->iovec[port].wrfunc;

    v0addspeedcnt(8);
    pc += sizeof(struct v0op);
    if (func) {
        func(vm, port, val);
    } else {
        v0doxcpt(V0_INV_IO_WRITE);
    }
    if (op->adr == V0_DIR_ADR) {
        pc += sizeof(union v0oparg);
        v0addspeedcnt(2);
    }
    opadr = &vm->mem[pc];
    vm->sysregs[V0_PC_REG] = pc;

    return opadr;
}

#endif /* __V0_OP_H__ */

