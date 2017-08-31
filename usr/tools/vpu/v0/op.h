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

#define V0_PC_OK        0
#define V0_PC_INVAL     (~(v0ureg)V0_PC_OK)
#if !defined(__GNUC__)
#define V0OP_T FASTCALL INLINE v0ureg
#else
#define V0OP_T INLINE          v0ureg
#endif

#define v0doxcpt(xcpt)                                                  \
    do {                                                                \
        fprintf(stderr, "%s\n", #xcpt);                                 \
                                                                        \
        exit(xcpt);                                                     \
    } while (0)

static V0OP_T
v0nop(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    pc += sizeof(struct v0op);

    return pc;
}

static V0OP_T
v0not(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg       *dptr = &vm->genregs[op->reg1];
    v0reg        src = *dptr;
    v0reg        dest;

    dest = ~src;
    pc += sizeof(struct v0op);
    *dptr = dest;
    vm->sysregs[V0_PC_REG] = pc;
    if (pc > vm->seglims[V0_TEXT_SEG]) {

        return V0_PC_INVAL;
    }

    return pc;
}

static V0OP_T
v0and(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        src = vm->genregs[op->reg1];
    v0reg        dest = *dptr;

    dest &= src;
    pc += sizeof(struct v0op);
    *dptr = dest;
    vm->sysregs[V0_PC_REG] = pc;
    if (pc > vm->seglims[V0_TEXT_SEG]) {

        return V0_PC_INVAL;
    }

    return pc;
}

static V0OP_T
v0or(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        src = vm->genregs[op->reg1];
    v0reg        dest = *dptr;

    dest |= src;
    pc += sizeof(struct v0op);
    *dptr = dest;
    vm->sysregs[V0_PC_REG] = pc;
    if (pc > vm->seglims[V0_TEXT_SEG]) {

        return V0_PC_INVAL;
    }

    return pc;
}

static V0OP_T
v0xor(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        src = vm->genregs[op->reg1];
    v0reg        dest = *dptr;

    dest ^= src;
    pc += sizeof(struct v0op);
    *dptr = dest;
    vm->sysregs[V0_PC_REG] = pc;
    if (pc > vm->seglims[V0_TEXT_SEG]) {

        return V0_PC_INVAL;
    }

    return pc;
}

static V0OP_T
v0shl(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        src = ((op->adr == V0_DIR_ADR)
                       ? op->val
                       : vm->genregs[op->reg1]);
    v0reg        dest = *dptr;

    dest <<= src;
    pc += sizeof(struct v0op);
    *dptr = dest;
    vm->sysregs[V0_PC_REG] = pc;
    if (pc > vm->seglims[V0_TEXT_SEG]) {

        return V0_PC_INVAL;
    }

    return pc;
}

static V0OP_T
v0shr(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        dest = *dptr;
    v0reg        src = ((op->adr == V0_DIR_ADR)
                       ? op->val
                       : vm->genregs[op->reg1]);
    v0reg        fill = ~((v0reg)0) >> src;

    dest >>= src;
    pc += sizeof(struct v0op);
    dest &= fill;
    vm->sysregs[V0_PC_REG] = pc;
    *dptr = dest;
    if (pc > vm->seglims[V0_TEXT_SEG]) {

        return V0_PC_INVAL;
    }

    return pc;
}

static V0OP_T
v0sar(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        src = ((op->adr == V0_DIR_ADR)
                       ? op->val
                       : vm->genregs[op->reg1]);
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
    dest &= fill;
    vm->sysregs[V0_PC_REG] = pc;
    *dptr = dest;
    if (pc > vm->seglims[V0_TEXT_SEG]) {

        return V0_PC_INVAL;
    }

    return pc;
}

static V0OP_T
v0inc(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg       *dptr = &vm->genregs[op->reg1];
    v0reg        src = *dptr;

    src++;
    pc += sizeof(struct v0op);
    *dptr = src;
    vm->sysregs[V0_PC_REG] = pc;
    if (pc > vm->seglims[V0_TEXT_SEG]) {

        return V0_PC_INVAL;
    }

    return pc;
}

static V0OP_T
v0dec(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg      *dptr = &vm->genregs[op->reg1];
    v0reg       src = *dptr;

    src--;
    pc += sizeof(struct v0op);
    *dptr = src;
    vm->sysregs[V0_PC_REG] = pc;
    if (pc > vm->seglims[V0_TEXT_SEG]) {

        return V0_PC_INVAL;
    }

    return pc;
}

static V0OP_T
v0add(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg      *dptr = &vm->genregs[op->reg2];
    v0reg       src = vm->genregs[op->reg1];
    v0reg       dest = *dptr;
    v0reg       res;

    pc += sizeof(struct v0op);
    res = src + dest;
    vm->sysregs[V0_PC_REG] = pc;
    if (res < dest) {
        v0setof(vm);
    }
    *dptr = dest;
    if (pc > vm->seglims[V0_TEXT_SEG]) {

        return V0_PC_INVAL;
    }

    return pc;
}

static V0OP_T
v0adc(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
#if 0
    struct v0op *op = (struct v0op *)ptr;
    v0reg      *dptr = &vm->genregs[op->reg2];
    v0reg       src = vm->genregs[op->reg1];
    v0reg       dest = *dptr;
    v0reg       res;

    pc += sizeof(struct v0op);
    res = src + dest;
    vm->sysregs[V0_PC_REG] = pc;
    if (res < dest) {
        v0setof(vm);
    }
    *dptr = dest;
    if (pc > vm->seglims[V0_TEXT_SEG]) {

        return V0_PC_INVAL;
    }
#endif

    return pc;
}

static V0OP_T
v0sub(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        src = vm->genregs[op->reg1];
    v0reg        dest = *dptr;

    pc += sizeof(struct v0op);
    dest -= src;
    vm->sysregs[V0_PC_REG] = pc;
    *dptr = dest;
    if (pc > vm->seglims[V0_TEXT_SEG]) {

        return V0_PC_INVAL;
    }

    return pc;
}

static V0OP_T
v0sbb(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        src = vm->genregs[op->reg1];
    v0reg        dest = *dptr;

    pc += sizeof(struct v0op);
    dest -= src;
    vm->sysregs[V0_PC_REG] = pc;
    if (pc > vm->seglims[V0_TEXT_SEG]) {

        return V0_PC_INVAL;
    }
    *dptr = dest;

    return pc;
}

static V0OP_T
v0cmp(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        src = vm->genregs[op->reg1];
    v0reg        dest = *dptr;

    pc += sizeof(struct v0op);
    dest -= src;
    vm->sysregs[V0_PC_REG] = pc;
    if (pc > vm->seglims[V0_TEXT_SEG]) {

        return V0_PC_INVAL;
    }
    v0clrmsw(vm);
    if (!dest) {
        v0setzf(vm);
    } else if (dest < 0) {
        v0setcf(vm);
    }
    *dptr = dest;

    return pc;
}

static V0OP_T
v0mul(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        src = vm->genregs[op->reg1];
    v0reg        dest = *dptr;

    pc += sizeof(struct v0op);
    dest *= src;
    vm->sysregs[V0_PC_REG] = pc;
    *dptr = dest;
    if (pc > vm->seglims[V0_TEXT_SEG]) {

        return V0_PC_INVAL;
    }

    return pc;
}

static V0OP_T
v0div(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        src = vm->genregs[op->reg1];
    v0reg        dest = *dptr;

    pc += sizeof(struct v0op);
    if (!src) {
        v0doxcpt(V0_DIV_BY_ZERO);
    } else if (src <= 0xffff) {
        dest = fastuf16divuf16(dest, src, vm->divu16tab);
    } else {
        dest /= src;
    }
    vm->sysregs[V0_PC_REG] = pc;
    *dptr = dest;
    if (pc > vm->seglims[V0_TEXT_SEG]) {

        return V0_PC_INVAL;
    }

    return pc;
}

static V0OP_T
v0rem(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg       *dptr = &vm->genregs[op->reg2];
    v0reg        src = vm->genregs[op->reg1];
    v0reg        dest = *dptr;

    pc += sizeof(struct v0op);
    dest %= src;
    vm->sysregs[V0_PC_REG] = pc;
    *dptr = dest;
    if (pc > vm->seglims[V0_TEXT_SEG]) {

        return V0_PC_INVAL;
    }

    return pc;
}

#if 0
static V0OP_T
v0crm(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    ; // TODO: hack this a'la ridiculous_fish?
}
#endif

static V0OP_T
v0jmp(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg        dest = vm->genregs[op->reg1];

    pc = dest;
    vm->sysregs[V0_PC_REG] = pc;

    return pc;
}

/* FIXME: implement v0call, v0ret */

static V0OP_T
v0call(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    ;
}

static V0OP_T
v0ret(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    ;
}

static V0OP_T
v0bz(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg        dest = vm->genregs[op->reg1];

    if (v0zfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    vm->sysregs[V0_PC_REG] = pc;

    return pc;
}

static V0OP_T
v0bnz(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg        dest = vm->genregs[op->reg1];

    if (!v0zfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    vm->sysregs[V0_PC_REG] = pc;

    return pc;
}

static V0OP_T
v0bc(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg        dest = vm->genregs[op->reg1];

    if (v0cfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    vm->sysregs[V0_PC_REG] = pc;

    return pc;
}

static V0OP_T
v0bnc(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg        dest = vm->genregs[op->reg1];

    if (!v0cfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    vm->sysregs[V0_PC_REG] = pc;

    return pc;
}

static V0OP_T
v0bo(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg        dest = vm->genregs[op->reg1];

    if (v0ofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    vm->sysregs[V0_PC_REG] = pc;

    return pc;
}

static V0OP_T
v0bno(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg        dest = vm->genregs[op->reg1];

    if (!v0ofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    vm->sysregs[V0_PC_REG] = pc;

    return pc;
}

static V0OP_T
v0blt(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg        dest = vm->genregs[op->reg1];

    if (!v0ofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    vm->sysregs[V0_PC_REG] = pc;

    return pc;
}

static V0OP_T
v0ble(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg        dest = vm->genregs[op->reg1];

    pc += sizeof(struct v0op);
    if (!v0ofset(vm) || v0zfset(vm)) {
        ptr = &vm->mem[dest];
    } else {
        ptr = &vm->mem[pc];
    }
    vm->sysregs[V0_PC_REG] = pc;

    return pc;
}

static V0OP_T
v0bgt(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg        dest = vm->genregs[op->reg1];

    if (v0ofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    vm->sysregs[V0_PC_REG] = pc;

    return pc;
}

static V0OP_T
v0bge(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg        dest = vm->genregs[op->reg1];

    pc += sizeof(struct v0op);
    if (v0ofset(vm) || v0zfset(vm)) {
        ptr = &vm->mem[dest];
    } else {
        ptr = &vm->mem[pc];
    }
    vm->sysregs[V0_PC_REG] = pc;

    return pc;
}

/*
 * V0_REG_ADR -> *reg
 * V0_NDX_ADR -> reg[op->imm8 << op->parm]
 * V0_DIR_ADR -> op->arg.adr
 * V0_PC_ADR  ->
 */
#define v0getadr(vm, op, reg)                                           \
    (((op)->adr == V0_REG_ADR)                                          \
     ? ((vm)->genregs[(op)->reg])                                       \
     : (((op)->adr == V0_NDX_ADR)                                       \
        ? ((vm)->genregs[(op)->reg] + ((op)->val << (op)->parm))        \
        : (((op)->adr == V0_DIR_ADR)                                    \
           ? ((op)->arg[0].adr)                                         \
           : ((vm)->genregs[V0_PC_REG] + ((op)->val << (op)->parm)))))

/* TODO: address translations for v0ldr, v0st */

static V0OP_T
v0ldr(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg        adr = v0getadr(vm, op, reg1);
    v0reg        src;
    v0ureg       usrc;
    v0reg       *dptr = &vm->genregs[op->reg2];

    pc += sizeof(struct v0op);
    pc += ((op)->adr & V0_DIR_ADR) * sizeof(union v0oparg);
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
    vm->sysregs[V0_PC_REG] = pc;

    return pc;
}

static V0OP_T
v0str(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0reg        adr = v0getadr(vm, op, reg2);
    v0reg        src;
    v0ureg       usrc;

    pc += sizeof(struct v0op);
    pc += ((op)->adr & V0_DIR_ADR) * sizeof(union v0oparg);
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
    vm->sysregs[V0_PC_REG] = pc;

    return pc;
}

static V0OP_T
v0psh(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0ureg       sp = vm->sysregs[V0_SP_REG];
    v0reg       *src = (v0reg *)&vm->genregs[op->reg1];
    v0reg       *dest = (v0reg *)&vm->mem[sp];

    pc+= sizeof(struct v0op);
    vm->sysregs[sp] = sp--;
    *dest = *src;
    vm->sysregs[V0_PC_REG] = pc;

    return pc;
}

static V0OP_T
v0pop(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0ureg       sp = vm->sysregs[V0_SP];
    v0reg       *src = (v0reg *)&vm->mem[sp];
    v0reg       *dest = (v0reg *)&vm->genregs[op->reg1];

    pc+= sizeof(struct v0op);
    vm->sysregs[sp] = sp++;
    *dest = *src;
    vm->sysregs[V0_PC_REG] = pc;

    return pc;
}

static V0OP_T
v0psha(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0ureg       sp = vm->sysregs[V0_SP_REG];
    v0ureg       newsp = sp - V0_NGENREG;
    v0reg       *dest = (v0reg *)&vm->mem[sp];
    v0reg       *src = (v0reg *)&vm->genregs[V0_R0_REG];
    v0reg        ndx;

    vm->sysregs[sp] = newsp;
    pc += sizeof(struct v0op);
    for (ndx = 0 ; ndx < (V0_NGENREG >> 3); ndx++) {
        dest[0] = src[0];
        dest[-1] = src[1];
        dest[-2] = src[2];
        dest[-3] = src[3];
        dest[-4] = src[4];
        dest[-5] = src[5];
        dest[-6] = src[6];
        dest[-7] = src[7];
        sp -= 8;
        dest -= 8;
    }
    vm->sysregs[V0_SP_REG] = sp;
    vm->sysregs[V0_PC_REG] = pc;

    return pc;
}

static V0OP_T
v0popa(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0ureg       sp = vm->sysregs[V0_SP];
    v0ureg       newsp = sp + V0_NGENREG;
    v0reg       *dest = (v0reg *)&vm->genregs[V0_R0_REG];
    v0reg       *src = (v0reg *)&vm->mem[sp];
    v0reg        ndx;

    vm->sysregs[sp] = newsp;
    pc += sizeof(struct v0op);
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
    }
    vm->sysregs[V0_SP_REG] = sp;
    vm->sysregs[V0_PC_REG] = pc;

    return pc;
}

static V0OP_T
v0pshm(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0ureg       sp = vm->sysregs[V0_SP_REG];
    v0ureg       newsp = sp - V0_NGENREG;
    v0reg       *dest = (v0reg *)&vm->mem[sp];
    v0reg       *src = (v0reg *)&vm->genregs[V0_R0_REG];
    v0reg        map = op->arg[0].rmap;
    v0reg        mask;
    v0reg        ndx;

    vm->sysregs[sp] = newsp;
    mask = 0x01;
    pc += sizeof(struct v0op);
    for (ndx = 0 ; ndx < (V0_NGENREG >> 3); ndx++) {
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
        sp -= 8;
        dest -= 8;
        mask >>= 8;
    }
    vm->sysregs[V0_SP_REG] = sp;
    vm->sysregs[V0_PC_REG] = pc;

    return pc;
}

static V0OP_T
v0popm(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    v0ureg       sp = vm->sysregs[V0_SP_REG];
    v0ureg       newsp = sp - V0_NGENREG;
    v0reg       *dest = (v0reg *)&vm->genregs[V0_R0_REG];
    v0reg       *src = (v0reg *)&vm->mem[sp];
    v0reg        map = op->arg[0].rmap;
    v0reg        mask;
    v0reg        ndx;

    vm->sysregs[sp] = newsp;
    mask = 0x01;
    pc += sizeof(struct v0op);
    for (ndx = 0 ; ndx < (V0_NGENREG >> 3); ndx++) {
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
        sp += 8;
        dest += 8;
        mask <<= 8;
    }
    vm->sysregs[V0_SP_REG] = sp;
    vm->sysregs[V0_PC_REG] = pc;

    return pc;
}

static V0OP_T
v0ior(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    uint16_t     port = op->val;
    uint16_t     reg = op->reg1;
    v0iofunc_t  *func = vm->iovec[port].rdfunc;
    v0reg        val = 0;

    if (func) {
        val = func(vm, port, reg);
    } else {
        v0doxcpt(V0_INV_IO_READ);
    }

    return val;
}

static V0OP_T
v0iow(struct v0 *vm, uint8_t *ptr, v0ureg pc)
{
    struct v0op *op = (struct v0op *)ptr;
    uint16_t     port = op->val;
    uint16_t     reg = op->reg1;
    v0iofunc_t  *func = vm->iovec[port].wrfunc;
    v0reg        val = 0;

    if (func) {
        val = func(vm, port, reg);
    } else {
        v0doxcpt(V0_INV_IO_WRITE);
    }

    return val;
}

#if defined(__GNUC__) && 0

#define v0declop(name) v0op##name

v0declop(not)
v0declop(and)
v0declop(or)
v0declop(xor)
v0declop(shl)
v0declop(shr)
v0declop(sar)
v0declop(rol)
v0declop(ror)
v0declop(inc)
v0declop(dec)
v0declop(add)
v0declop(adc)
v0declop(sub)
v0declop(sbb)
v0declop(cmp)
v0declop(mul)
v0declop(div)
v0declop(rem)
//v0declop(crm)
v0declop(jmp)
v0declop(call)
v0declop(ret)
v0declop(lda)
v0declop(sta)
v0declop(ror)
v0declop(jmp)
v0declop(bz)
v0declop(bnz)
v0declop(bc)
v0declop(bnc)
v0declop(bo)
v0declop(bno)
v0declop(blt)
v0declop(ble)
v0declop(bgt)
v0declop(bge)
v0declop(ldr)
v0declop(str)
v0declop(psh)
v0declop(pop)
v0declop(psha)
v0declop(popa)
v0declop(pshm)
v0declop(popm)
v0declop(ldr)
v0declop(str)
v0declop(rst)
v0declop(hlt)
v0declop(in)
v0declop(out)

#endif /* defined(__GNUC__) */

#endif /* __V0_OP_H__ */

