#ifndef __ZPM_OP_H__
#define __ZPM_OP_H__

#include <zpm/conf.h>
#include <stddef.h>
#include <limits.h>
#include <stdint.h>
#include <zpm/zpm.h>
#include <zpm/op.h>

#define zpmget1arggenreg(vm, op, dptr)                                  \
    ((dptr) = &vm->genregs[(op)->reg1], *(dptr))
#define zpmget2argsgenreg(vm, op, dptr, src, dest)                      \
    (!((op)->argt & ZPM_IMM_VAL)                                        \
     ? ((dptr) = &vm->genregs[(op)->reg2],                              \
        (src) = vm->genregs[(op)->reg1],                                \
        (dest) = *(dptr))                                               \
     : ((dptr) = &vm->genregs[(op)->reg2],                              \
        (src) = op->imm[0],                                             \
        (dest) = *(dptr)))

__inline__ zpmureg
zpmnot(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src = zpmget1arggenreg(vm, op, dptr);
    zpmreg        dest;

    dest = ~src;
    pc += sizeof(struct zpmop);
    *dptr = dest;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ~(zpmureg)0;
    }
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmand(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src;
    zpmreg        dest;

    zpmget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & ZPM_IMM_VAL) {
        pc += sizeof(struct zpmop) + sizeof(zpmreg);
    } else {
        pc += sizeof(struct zpmop);
    }
    dest &= src;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ~(zpmureg)0;
    }
    *dptr = dest;
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmor(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src;
    zpmreg        dest;

    zpmget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & ZPM_IMM_VAL) {
        pc += sizeof(struct zpmop) + sizeof(zpmreg);
    } else {
        pc += sizeof(struct zpmop);
    }
    dest |= src;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ~(zpmureg)0;
    }
    *dptr = dest;
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmxor(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src;
    zpmreg        dest;

    zpmget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & ZPM_IMM_VAL) {
        pc += sizeof(struct zpmop) + sizeof(zpmreg);
    } else {
        pc += sizeof(struct zpmop);
    }
    dest ^= src;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ~(zpmureg)0;
    }
    *dptr = dest;
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmshl(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr = &vm->genregs[op->reg2];
    zpmreg        src = ((op->argt & ZPM_IMM_VAL)
                         ? op->imm8
                         : vm->genregs[op->reg1]);
    zpmreg  dest = *dptr;

    dest <<= src;
    pc += sizeof(struct zpmop);
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ~(zpmureg)0;
    }
    *dptr = dest;
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmshr(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr = &vm->genregs[op->reg2];
    zpmreg        src = ((op->argt & ZPM_IMM_VAL)
                         ? op->imm8
                         : vm->genregs[op->reg1]);
    zpmreg        dest = *dptr;
    zpmreg        fill = ~((zpmreg)0) >> src;

    dest >>= src;
    pc += sizeof(struct zpmop);
    dest &= fill;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ~(zpmureg)0;
    }
    *dptr = dest;
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmsar(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr = &vm->genregs[op->reg2];
    zpmreg        src = ((op->argt & ZPM_IMM_VAL)
                         ? op->imm8
                         : vm->genregs[op->reg1]);
    zpmreg        dest = *dptr;
    zpmreg        mask = ~(zpmreg)0;
#if defined(ZPM64BIT)
    zpmreg        fill = (((dest) & (INT64_C(1) << 63))
                          ? (mask >> (64 - src))
                          : 0);
#else
    zpmreg        fill = (((dest) & (INT32_C(1) << 31))
                          ? (mask >> (32 - src))
                          : 0);
#endif

    dest >>= src;
    pc += sizeof(struct zpmop);
    fill = -fill << (CHAR_BIT * sizeof(zpmreg) - src);
    dest &= fill;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ~(zpmureg)0;
    }
    *dptr = dest;
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmrol(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr = &vm->genregs[op->reg2];
    zpmreg        src = ((op->argt & ZPM_IMM_VAL)
                         ? op->imm8
                         : vm->genregs[op->reg1]);
    zpmreg        dest = *dptr;
    zpmreg        mask = (~(zpmreg)0) << (CHAR_BIT * sizeof(zpmreg) - src);
    zpmreg        bits = dest & mask;
    zpmreg        cf = dest & ((zpmreg)1 << (src - 1));

    bits >>= CHAR_BIT * sizeof(zpmreg) - 1;
    dest <<= src;
    pc += sizeof(struct zpmop);
    dest |= bits;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ~(zpmureg)0;
    }
    *dptr = dest;
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmror(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr = &vm->genregs[op->reg2];
    zpmreg        src = ((op->argt & ZPM_IMM_VAL)
                         ? op->imm8
                         : vm->genregs[op->reg1]);
    zpmreg        dest = *dptr;
    zpmreg        mask = (~(zpmreg)0) >> (CHAR_BIT * sizeof(zpmreg) - src);
    zpmreg        bits = dest & mask;
    zpmreg        cf = dest & ((zpmreg)1 << (src - 1));

    bits <<= CHAR_BIT * sizeof(zpmreg) - 1;
    dest >>= src;
    pc += sizeof(struct zpmop);
    dest |= bits;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ~(zpmureg)0;
    }
    *dptr = dest;
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpminc(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr = &vm->genregs[op->reg1];
    zpmreg        src = *dptr;
    zpmreg        dest;

    src++;
    pc += sizeof(struct zpmop);
    dest = src;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ~(zpmureg)0;
    }
    *dptr = dest;
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmdec(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr = &vm->genregs[op->reg1];
    zpmreg        src = *dptr;
    zpmreg        dest;

    src--;
    pc += sizeof(struct zpmop);
    dest = src;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ~(zpmureg)0;
    }
    *dptr = dest;
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmadd(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src;
    zpmreg        dest;
    zpmreg        res;

    zpmget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & ZPM_IMM_VAL) {
        pc += sizeof(struct zpmop) + sizeof(zpmreg);
    } else {
        pc += sizeof(struct zpmop);
    }
    res = src + dest;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ~(zpmureg)0;
    }
    if (res < dest) {
        zpmsetof(vm);
    }
    *dptr = dest;
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmsub(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src;
    zpmreg        dest;

    zpmget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & ZPM_IMM_VAL) {
        pc += sizeof(struct zpmop) + sizeof(zpmreg);
    } else {
        pc += sizeof(struct zpmop);
    }
    dest -= src;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ~(zpmureg)0;
    }
    *dptr = dest;
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmcmp(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src;
    zpmreg        dest;

    zpmget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & ZPM_IMM_VAL) {
        pc += sizeof(struct zpmop) + sizeof(zpmreg);
    } else {
        pc += sizeof(struct zpmop);
    }
    dest -= src;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ~(zpmureg)0;
    }
    zpmclrmsw(vm);
    if (!dest) {
        zpmsetzf(vm);
    } else if (dest < 0) {
        zpmsetcf(vm);
    }
    *dptr = dest;
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmmul(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ zpmureg
zpmdiv(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ zpmureg
zpmjmp(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ zpmureg
zpmbz(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    if (zpmzfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct zpmop);
    }
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmbnz(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    if (!zpmzfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct zpmop);
    }
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmblt(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    if (!zpmofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct zpmop);
    }
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmble(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    pc += sizeof(struct zpmop);
    if (!zpmofset(vm) || zpmzfset(vm)) {
        ptr = &vm->mem[dest];
    } else {
        ptr = &vm->mem[pc];
    }
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmbgt(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    if (zpmofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct zpmop);
    }
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmbge(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    pc += sizeof(struct zpmop);
    if (zpmofset(vm) || zpmzfset(vm)) {
        ptr = &vm->mem[dest];
    } else {
        ptr = &vm->mem[pc];
    }
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmbo(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    if (zpmofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct zpmop);
    }
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmbno(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    if (!zpmofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct zpmop);
    }
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmbc(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    if (zpmcfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct zpmop);
    }
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmbnc(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    if (!zpmcfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct zpmop);
    }
    vm->pc = pc;

    return pc;
}

__inline__ zpmureg
zpmpop(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ zpmureg
zpmpush(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ zpmureg
zpmpusha(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ zpmureg
zpmlda(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ zpmureg
zpmsta(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ zpmureg
zpmcall(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ zpmureg
zpmenter(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ zpmureg
zpmleave(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ zpmureg
zpmret(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ zpmureg
zpmthr(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ zpmureg
zpmltb(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ zpmureg
zpmldr(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ zpmureg
zpmstr(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ zpmureg
zpmrst(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ zpmureg
zpmhlt(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ zpmureg
zpmin(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ zpmureg
zpmout(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

#endif /* __ZPM_OP_H__ */

