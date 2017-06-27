#ifndef __ZPM_OP_H__
#define __ZPM_OP_H__

#include <zpm/conf.h>
#include <stddef.h>
#include <limits.h>
#include <stdint.h>
#include <zero/param.h>
#include <zpm/zpm.h>
#include <zpm/op.h>

#define ZPM_PC_OK    0
#define ZPM_PC_INVAL (~(zpmureg)ZPM_PC_OK)

#if !defined(__GNUC__)
#define ZPMOP_T FASTCALL __inline__ zpmureg
#else
#define ZPMOP_T zpmureg
#endif

#define zpmget1arggenreg(vm, op, dptr)                                  \
    ((dptr) = &vm->genregs[(op)->reg1], *(dptr))
#define zpmget2argsgenreg(vm, op, dptr, src, dest)                      \
    (!((op)->argt & ZPM_IMM_ARG)                                        \
     ? ((dptr) = &vm->genregs[(op)->reg2],                              \
        (src) = vm->genregs[(op)->reg1],                                \
        (dest) = *(dptr))                                               \
     : ((dptr) = &vm->genregs[(op)->reg2],                              \
        (src) = op->imm[0],                                             \
        (dest) = *(dptr)))

ZPMOP_T
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

        return ZPM_PC_INVAL;
    }
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmand(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src;
    zpmreg        dest;
    
    zpmget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & ZPM_IMM_ARG) {
        pc += sizeof(struct zpmop) + sizeof(zpmreg);
    } else {
        pc += sizeof(struct zpmop);
    }
    dest &= src;
    if (pc > vm->seglims[ZPM_TEXT]) {
        
        return ZPM_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[ZPM_PC] = pc;
    
    return pc;
}

ZPMOP_T
zpmor(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src;
    zpmreg        dest;
    
    zpmget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & ZPM_IMM_ARG) {
        pc += sizeof(struct zpmop) + sizeof(zpmreg);
    } else {
        pc += sizeof(struct zpmop);
    }
    dest |= src;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ZPM_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmxor(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src;
    zpmreg        dest;

    zpmget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & ZPM_IMM_ARG) {
        pc += sizeof(struct zpmop) + sizeof(zpmreg);
    } else {
        pc += sizeof(struct zpmop);
    }
    dest ^= src;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ZPM_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmshl(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr = &vm->genregs[op->reg2];
    zpmreg        src = ((op->argt & ZPM_IMM_ARG)
                         ? op->imm8
                         : vm->genregs[op->reg1]);
    zpmreg  dest = *dptr;

    dest <<= src;
    pc += sizeof(struct zpmop);
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ZPM_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmshr(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr = &vm->genregs[op->reg2];
    zpmreg        src = ((op->argt & ZPM_IMM_ARG)
                         ? op->imm8
                         : vm->genregs[op->reg1]);
    zpmreg        dest = *dptr;
    zpmreg        fill = ~((zpmreg)0) >> src;

    dest >>= src;
    pc += sizeof(struct zpmop);
    dest &= fill;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ZPM_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmsar(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr = &vm->genregs[op->reg2];
    zpmreg        src = ((op->argt & ZPM_IMM_ARG)
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

        return ZPM_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmrol(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr = &vm->genregs[op->reg2];
    zpmreg        src = ((op->argt & ZPM_IMM_ARG)
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

        return ZPM_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmror(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr = &vm->genregs[op->reg2];
    zpmreg        src = ((op->argt & ZPM_IMM_ARG)
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

        return ZPM_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
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

        return ZPM_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
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

        return ZPM_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmadd(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src;
    zpmreg        dest;
    zpmreg        res;

    zpmget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & ZPM_IMM_ARG) {
        pc += sizeof(struct zpmop) + sizeof(zpmreg);
    } else {
        pc += sizeof(struct zpmop);
    }
    res = src + dest;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ZPM_PC_INVAL;
    }
    if (res < dest) {
        zpmsetof(vm);
    }
    *dptr = dest;
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmadc(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
#if 0
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src;
    zpmreg        dest;
    zpmreg        res;

    zpmget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & ZPM_IMM_ARG) {
        pc += sizeof(struct zpmop) + sizeof(zpmreg);
    } else {
        pc += sizeof(struct zpmop);
    }
    res = src + dest;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ZPM_PC_INVAL;
    }
    if (res < dest) {
        zpmsetof(vm);
    }
    *dptr = dest;
    vm->sysregs[ZPM_PC] = pc;
#endif

    return pc;
}

ZPMOP_T
zpmadi(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
#if 0
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src;
    zpmreg        dest;
    zpmreg        res;

    zpmget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & ZPM_IMM_ARG) {
        pc += sizeof(struct zpmop) + sizeof(zpmreg);
    } else {
        pc += sizeof(struct zpmop);
    }
    res = src + dest;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ZPM_PC_INVAL;
    }
    if (res < dest) {
        zpmsetof(vm);
    }
    *dptr = dest;
    vm->sysregs[ZPM_PC] = pc;
#endif

    return pc;
}

ZPMOP_T
zpmsub(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src;
    zpmreg        dest;

    zpmget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & ZPM_IMM_ARG) {
        pc += sizeof(struct zpmop) + sizeof(zpmreg);
    } else {
        pc += sizeof(struct zpmop);
    }
    dest -= src;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ZPM_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmsbc(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src;
    zpmreg        dest;

    zpmget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & ZPM_IMM_ARG) {
        pc += sizeof(struct zpmop) + sizeof(zpmreg);
    } else {
        pc += sizeof(struct zpmop);
    }
    dest -= src;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ZPM_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmcmp(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src;
    zpmreg        dest;

    zpmget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & ZPM_IMM_ARG) {
        pc += sizeof(struct zpmop) + sizeof(zpmreg);
    } else {
        pc += sizeof(struct zpmop);
    }
    dest -= src;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return ZPM_PC_INVAL;
    }
    zpmclrmsw(vm);
    if (!dest) {
        zpmsetzf(vm);
    } else if (dest < 0) {
        zpmsetcf(vm);
    }
    *dptr = dest;
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmmul(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

ZPMOP_T
zpmdiv(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

ZPMOP_T
zpmrem(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

ZPMOP_T
zpmlda(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

ZPMOP_T
zpmsta(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

ZPMOP_T
zpmjmp(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

ZPMOP_T
zpmbz(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    if (zpmzfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct zpmop);
    }
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmbnz(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    if (!zpmzfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct zpmop);
    }
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmblt(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    if (!zpmofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct zpmop);
    }
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
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
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmbgt(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    if (zpmofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct zpmop);
    }
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
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
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmbo(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    if (zpmofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct zpmop);
    }
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmbno(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    if (!zpmofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct zpmop);
    }
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmbc(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    if (zpmcfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct zpmop);
    }
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmbnc(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    if (!zpmcfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct zpmop);
    }
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmcall(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

ZPMOP_T
zpmthr(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

ZPMOP_T
zpmenter(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

ZPMOP_T
zpmleave(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

ZPMOP_T
zpmret(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    /* return from subroutine or thread if return address is 0 */
    ;
}

ZPMOP_T
zpmpsh(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmureg       sp = vm->sysregs[ZPM_SP];
    zpmreg       *src = (zpmreg *)&vm->genregs[op->reg1];
    zpmreg       *dest = (zpmreg *)&vm->mem[sp];

    pc+= sizeof(struct zpmop);
    vm->sysregs[sp] = --sp;
    *dest = *src;
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmpsha(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmureg       sp = vm->sysregs[ZPM_SP];
    zpmureg       newsp = sp - ZPM_NGENREG;
    zpmreg       *dest = (zpmreg *)&vm->mem[sp];
    zpmreg       *src = (zpmreg *)&vm->genregs[ZPM_REG0];
    zpmreg        ndx;

    vm->sysregs[sp] = newsp;
    pc += sizeof(struct zpmop);
    for (ndx = 0 ; ndx < (ZPM_NGENREG >> 3); ndx++) {
        dest[0] = src[0];
        dest[-1] = src[1];
        dest[-2] = src[2];
        dest[-3] = src[3];
        dest[-4] = src[4];
        dest[-5] = src[5];
        dest[-6] = src[6];
        dest[-7] = src[7];
        dest -= 8;
    }
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmpop(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmureg       sp = vm->sysregs[ZPM_SP];
    zpmreg       *src = (zpmreg *)&vm->mem[sp];
    zpmreg       *dest = (zpmreg *)&vm->genregs[op->reg1];

    pc+= sizeof(struct zpmop);
    vm->sysregs[sp] = ++sp;
    *dest = *src;
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmpopa(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmureg       sp = vm->sysregs[ZPM_SP];
    zpmureg       newsp = sp + ZPM_NGENREG;
    zpmreg       *dest = (zpmreg *)&vm->genregs[ZPM_REG0];
    zpmreg       *src = (zpmreg *)&vm->mem[sp];
    zpmreg        ndx;

    vm->sysregs[sp] = newsp;
    pc += sizeof(struct zpmop);
    for (ndx = 0 ; ndx < (ZPM_NGENREG >> 3); ndx++) {
        dest[0] = src[0];
        dest[1] = src[1];
        dest[2] = src[2];
        dest[3] = src[3];
        dest[4] = src[4];
        dest[5] = src[5];
        dest[6] = src[6];
        dest[7] = src[7];
        dest += 8;
    }
    vm->sysregs[ZPM_PC] = pc;

    return pc;
}

ZPMOP_T
zpmin(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    return ZPM_PC_INVAL;
}

ZPMOP_T
zpmout(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    return ZPM_PC_INVAL;
}

ZPMOP_T
zpmldr(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    return ZPM_PC_INVAL;
}

ZPMOP_T
zpmstr(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    return ZPM_PC_INVAL;
}

ZPMOP_T
zpmrst(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    return ZPM_PC_INVAL;
}

ZPMOP_T
zpmhlt(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    return ZPM_PC_INVAL;
}

ZPMOP_T
zpmnop(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    pc += sizeof(struct zpmop);

    return pc;
}

#if defined(__GNUC__)

#define zpmdeclop(name) zpm##name

#else /* !defined(__GNUC__) */

#define zpmdeclop(name)                                                 \
    ZPMOP_T                                                             \
    zpmop##name(struct zpm *vm, uint8_t *ptr, zpmureg pc)               \
    {                                                                   \
        pc = zpm##name(vm, ptr, pc);                                    \
                                                                        \
        return pc;                                                      \
    }

zpmdeclop(not)
zpmdeclop(and)
zpmdeclop(or)
zpmdeclop(xor)
zpmdeclop(shl)
zpmdeclop(shr)
zpmdeclop(sar)
zpmdeclop(rol)
zpmdeclop(ror)
zpmdeclop(inc)
zpmdeclop(dec)
zpmdeclop(add)
zpmdeclop(adc)
zpmdeclop(adi)
zpmdeclop(sub)
zpmdeclop(sbc)
zpmdeclop(cmp)
zpmdeclop(mul)
zpmdeclop(div)
zpmdeclop(rem)
zpmdeclop(lda)
zpmdeclop(sta)
zpmdeclop(ror)
zpmdeclop(jmp)
zpmdeclop(bz)
zpmdeclop(bnz)
zpmdeclop(blt)
zpmdeclop(ble)
zpmdeclop(bgt)
zpmdeclop(bge)
zpmdeclop(bo)
zpmdeclop(bno)
zpmdeclop(bc)
zpmdeclop(bnc)
zpmdeclop(call)
zpmdeclop(thr)
zpmdeclop(enter)
zpmdeclop(leave)
zpmdeclop(ret)
zpmdeclop(pop)
zpmdeclop(popa)
zpmdeclop(push)
zpmdeclop(pusha)
zpmdeclop(ldr)
zpmdeclop(str)
zpmdeclop(rst)
zpmdeclop(hlt)
zpmdeclop(in)
zpmdeclop(out)
zpmdeclop(nop)

#endif /* defined(__GNUC__) */

#endif /* __ZPM_OP_H__ */

