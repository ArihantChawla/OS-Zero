#ifndef __VPU_OP_H__
#define __VPU_OP_H__

/* FIXME: make this file work */

#include <vpu/conf.h>
#include <stddef.h>
#include <limits.h>
#include <stdint.h>
#include <zero/param.h>
#include <vpu/vpu.h>
#include <vpu/op.h>

#define VPU_PC_OK        0
#define VPU_PC_INVAL     (~(vpuureg)VPU_PC_OK)
#if !defined(__GNUC__)
#define VPUOP_T FASTCALL vpuureg
#else
#define VPUOP_T INLINE   vpuureg
#endif

#define vpuget1arggenreg(vm, op, dptr)                                  \
    ((dptr) = &vm->genregs[(op)->reg1], *(dptr))
#define vpuget2argsgenreg(vm, op, dptr, src, dest)                      \
    (!((op)->argt & VPU_IMM_ARG)                                        \
     ? ((dptr) = &vm->genregs[(op)->reg2],                              \
        (src) = vm->genregs[(op)->reg1],                                \
        (dest) = *(dptr))                                               \
     : ((dptr) = &vm->genregs[(op)->reg2],                              \
        (src) = op->imm[0],                                             \
        (dest) = *(dptr)))

VPUOP_T
vpunop(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    pc += sizeof(struct vpuop);

    return pc;
}

VPUOP_T
vpunot(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg       *dptr;
    vpureg        src = vpuget1arggenreg(vm, op, dptr);
    vpureg        dest;

    dest = ~src;
    pc += sizeof(struct vpuop);
    *dptr = dest;
    if (pc > vm->seglims[VPU_TEXT]) {

        return VPU_PC_INVAL;
    }
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpuand(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg       *dptr;
    vpureg        src;
    vpureg        dest;
    
    vpuget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & VPU_IMM_ARG) {
        pc += sizeof(struct vpuop) + sizeof(vpureg);
    } else {
        pc += sizeof(struct vpuop);
    }
    dest &= src;
    if (pc > vm->seglims[VPU_TEXT]) {
        
        return VPU_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[VPU_PC] = pc;
    
    return pc;
}

VPUOP_T
vpuor(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg       *dptr;
    vpureg        src;
    vpureg        dest;
    
    vpuget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & VPU_IMM_ARG) {
        pc += sizeof(struct vpuop) + sizeof(vpureg);
    } else {
        pc += sizeof(struct vpuop);
    }
    dest |= src;
    if (pc > vm->seglims[VPU_TEXT]) {

        return VPU_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpuxor(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg       *dptr;
    vpureg        src;
    vpureg        dest;

    vpuget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & VPU_IMM_ARG) {
        pc += sizeof(struct vpuop) + sizeof(vpureg);
    } else {
        pc += sizeof(struct vpuop);
    }
    dest ^= src;
    if (pc > vm->seglims[VPU_TEXT]) {

        return VPU_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpushl(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg       *dptr = &vm->genregs[op->reg2];
    vpureg        src = ((op->argt & VPU_IMM_ARG)
                         ? op->imm8
                         : vm->genregs[op->reg1]);
    vpureg  dest = *dptr;

    dest <<= src;
    pc += sizeof(struct vpuop);
    if (pc > vm->seglims[VPU_TEXT]) {

        return VPU_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpushr(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg       *dptr = &vm->genregs[op->reg2];
    vpureg        src = ((op->argt & VPU_IMM_ARG)
                         ? op->imm8
                         : vm->genregs[op->reg1]);
    vpureg        dest = *dptr;
    vpureg        fill = ~((vpureg)0) >> src;

    dest >>= src;
    pc += sizeof(struct vpuop);
    dest &= fill;
    if (pc > vm->seglims[VPU_TEXT]) {

        return VPU_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpusar(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg       *dptr = &vm->genregs[op->reg2];
    vpureg        src = ((op->argt & VPU_IMM_ARG)
                         ? op->imm8
                         : vm->genregs[op->reg1]);
    vpureg        dest = *dptr;
    vpureg        mask = ~(vpureg)0;
#if defined(VPU64BIT)
    vpureg        fill = (((dest) & (INT64_C(1) << 63))
                          ? (mask >> (64 - src))
                          : 0);
#else
    vpureg        fill = (((dest) & (INT32_C(1) << 31))
                          ? (mask >> (32 - src))
                          : 0);
#endif

    dest >>= src;
    pc += sizeof(struct vpuop);
    fill = -fill << (CHAR_BIT * sizeof(vpureg) - src);
    dest &= fill;
    if (pc > vm->seglims[VPU_TEXT]) {

        return VPU_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpurol(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg       *dptr = &vm->genregs[op->reg2];
    vpureg        src = ((op->argt & VPU_IMM_ARG)
                         ? op->imm8
                         : vm->genregs[op->reg1]);
    vpureg        dest = *dptr;
    vpureg        mask = (~(vpureg)0) << (CHAR_BIT * sizeof(vpureg) - src);
    vpureg        bits = dest & mask;
    vpureg        cf = dest & ((vpureg)1 << (src - 1));

    bits >>= CHAR_BIT * sizeof(vpureg) - 1;
    dest <<= src;
    pc += sizeof(struct vpuop);
    dest |= bits;
    if (pc > vm->seglims[VPU_TEXT]) {

        return VPU_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpuror(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg       *dptr = &vm->genregs[op->reg2];
    vpureg        src = ((op->argt & VPU_IMM_ARG)
                         ? op->imm8
                         : vm->genregs[op->reg1]);
    vpureg        dest = *dptr;
    vpureg        mask = (~(vpureg)0) >> (CHAR_BIT * sizeof(vpureg) - src);
    vpureg        bits = dest & mask;
    vpureg        cf = dest & ((vpureg)1 << (src - 1));

    bits <<= CHAR_BIT * sizeof(vpureg) - 1;
    dest >>= src;
    pc += sizeof(struct vpuop);
    dest |= bits;
    if (pc > vm->seglims[VPU_TEXT]) {

        return VPU_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpuinc(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg       *dptr = &vm->genregs[op->reg1];
    vpureg        src = *dptr;
    vpureg        dest;

    src++;
    pc += sizeof(struct vpuop);
    dest = src;
    if (pc > vm->seglims[VPU_TEXT]) {

        return VPU_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpudec(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg       *dptr = &vm->genregs[op->reg1];
    vpureg        src = *dptr;
    vpureg        dest;

    src--;
    pc += sizeof(struct vpuop);
    dest = src;
    if (pc > vm->seglims[VPU_TEXT]) {

        return VPU_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpuadd(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg       *dptr;
    vpureg        src;
    vpureg        dest;
    vpureg        res;

    vpuget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & VPU_IMM_ARG) {
        pc += sizeof(struct vpuop) + sizeof(vpureg);
    } else {
        pc += sizeof(struct vpuop);
    }
    res = src + dest;
    if (pc > vm->seglims[VPU_TEXT]) {

        return VPU_PC_INVAL;
    }
    if (res < dest) {
        vpusetof(vm);
    }
    *dptr = dest;
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpuadc(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
#if 0
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg       *dptr;
    vpureg        src;
    vpureg        dest;
    vpureg        res;

    vpuget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & VPU_IMM_ARG) {
        pc += sizeof(struct vpuop) + sizeof(vpureg);
    } else {
        pc += sizeof(struct vpuop);
    }
    res = src + dest;
    if (pc > vm->seglims[VPU_TEXT]) {

        return VPU_PC_INVAL;
    }
    if (res < dest) {
        vpusetof(vm);
    }
    *dptr = dest;
    vm->sysregs[VPU_PC] = pc;
#endif

    return pc;
}

VPUOP_T
vpuadi(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
#if 0
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg       *dptr;
    vpureg        src;
    vpureg        dest;
    vpureg        res;

    vpuget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & VPU_IMM_ARG) {
        pc += sizeof(struct vpuop) + sizeof(vpureg);
    } else {
        pc += sizeof(struct vpuop);
    }
    res = src + dest;
    if (pc > vm->seglims[VPU_TEXT]) {

        return VPU_PC_INVAL;
    }
    if (res < dest) {
        vpusetof(vm);
    }
    *dptr = dest;
    vm->sysregs[VPU_PC] = pc;
#endif

    return pc;
}

VPUOP_T
vpusub(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg       *dptr;
    vpureg        src;
    vpureg        dest;

    vpuget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & VPU_IMM_ARG) {
        pc += sizeof(struct vpuop) + sizeof(vpureg);
    } else {
        pc += sizeof(struct vpuop);
    }
    dest -= src;
    if (pc > vm->seglims[VPU_TEXT]) {

        return VPU_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpusbc(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg       *dptr;
    vpureg        src;
    vpureg        dest;

    vpuget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & VPU_IMM_ARG) {
        pc += sizeof(struct vpuop) + sizeof(vpureg);
    } else {
        pc += sizeof(struct vpuop);
    }
    dest -= src;
    if (pc > vm->seglims[VPU_TEXT]) {

        return VPU_PC_INVAL;
    }
    *dptr = dest;
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpucmp(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg       *dptr;
    vpureg        src;
    vpureg        dest;

    vpuget2argsgenreg(vm, op, dptr, src, dest);
    if (op->argt & VPU_IMM_ARG) {
        pc += sizeof(struct vpuop) + sizeof(vpureg);
    } else {
        pc += sizeof(struct vpuop);
    }
    dest -= src;
    if (pc > vm->seglims[VPU_TEXT]) {

        return VPU_PC_INVAL;
    }
    vpuclrmsw(vm);
    if (!dest) {
        vpusetzf(vm);
    } else if (dest < 0) {
        vpusetcf(vm);
    }
    *dptr = dest;
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpumul(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    ;
}

VPUOP_T
vpudiv(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    ;
}

VPUOP_T
vpurem(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    ;
}

VPUOP_T
vpulda(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    ;
}

VPUOP_T
vpusta(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    ;
}

VPUOP_T
vpujmp(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    ;
}

VPUOP_T
vpubz(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg        dest = vm->genregs[op->reg1];

    if (vpuzfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct vpuop);
    }
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpubnz(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg        dest = vm->genregs[op->reg1];

    if (!vpuzfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct vpuop);
    }
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpublt(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg        dest = vm->genregs[op->reg1];

    if (!vpuofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct vpuop);
    }
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpuble(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg        dest = vm->genregs[op->reg1];

    pc += sizeof(struct vpuop);
    if (!vpuofset(vm) || vpuzfset(vm)) {
        ptr = &vm->mem[dest];
    } else {
        ptr = &vm->mem[pc];
    }
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpubgt(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg        dest = vm->genregs[op->reg1];

    if (vpuofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct vpuop);
    }
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpubge(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg        dest = vm->genregs[op->reg1];

    pc += sizeof(struct vpuop);
    if (vpuofset(vm) || vpuzfset(vm)) {
        ptr = &vm->mem[dest];
    } else {
        ptr = &vm->mem[pc];
    }
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpubo(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg        dest = vm->genregs[op->reg1];

    if (vpuofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct vpuop);
    }
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpubno(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg        dest = vm->genregs[op->reg1];

    if (!vpuofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct vpuop);
    }
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpubc(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg        dest = vm->genregs[op->reg1];

    if (vpucfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct vpuop);
    }
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpubnc(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpureg        dest = vm->genregs[op->reg1];

    if (!vpucfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct vpuop);
    }
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpucall(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    ;
}

VPUOP_T
vputhr(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    ;
}

VPUOP_T
vpuenter(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    ;
}

VPUOP_T
vpuleave(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    ;
}

VPUOP_T
vpuret(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    /* return from subroutine or thread if return address is 0 */
    ;
}

VPUOP_T
vpupsh(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpuureg       sp = vm->sysregs[VPU_SP];
    vpureg       *src = (vpureg *)&vm->genregs[op->reg1];
    vpureg       *dest = (vpureg *)&vm->mem[sp];

    pc+= sizeof(struct vpuop);
    vm->sysregs[sp] = --sp;
    *dest = *src;
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpupsha(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpuureg       sp = vm->sysregs[VPU_SP];
    vpuureg       newsp = sp - VPU_NGENREG;
    vpureg       *dest = (vpureg *)&vm->mem[sp];
    vpureg       *src = (vpureg *)&vm->genregs[VPU_REG0];
    vpureg        ndx;

    vm->sysregs[sp] = newsp;
    pc += sizeof(struct vpuop);
    for (ndx = 0 ; ndx < (VPU_NGENREG >> 3); ndx++) {
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
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpupop(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpuureg       sp = vm->sysregs[VPU_SP];
    vpureg       *src = (vpureg *)&vm->mem[sp];
    vpureg       *dest = (vpureg *)&vm->genregs[op->reg1];

    pc+= sizeof(struct vpuop);
    vm->sysregs[sp] = ++sp;
    *dest = *src;
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpupopa(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    struct vpuop *op = (struct vpuop *)ptr;
    vpuureg       sp = vm->sysregs[VPU_SP];
    vpuureg       newsp = sp + VPU_NGENREG;
    vpureg       *dest = (vpureg *)&vm->genregs[VPU_REG0];
    vpureg       *src = (vpureg *)&vm->mem[sp];
    vpureg        ndx;

    vm->sysregs[sp] = newsp;
    pc += sizeof(struct vpuop);
    for (ndx = 0 ; ndx < (VPU_NGENREG >> 3); ndx++) {
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
    vm->sysregs[VPU_PC] = pc;

    return pc;
}

VPUOP_T
vpuin(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    return VPU_PC_INVAL;
}

VPUOP_T
vpuout(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    return VPU_PC_INVAL;
}

VPUOP_T
vpuldr(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    return VPU_PC_INVAL;
}

VPUOP_T
vpustr(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    return VPU_PC_INVAL;
}

VPUOP_T
vpurst(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    return VPU_PC_INVAL;
}

VPUOP_T
vpuhlt(struct vpu *vm, uint8_t *ptr, vpuureg pc)
{
    return VPU_PC_INVAL;
}

#if defined(__GNUC__)

#define vpudeclop(name) vpuop##name

#else /* !defined(__GNUC__) */

#define vpudeclop(name)                                                 \
    FASTCALL                                                            \
    VPUOP_T                                                             \
    vpuop##name(struct vpu *vm, uint8_t *ptr, vpuureg pc)               \
    {                                                                   \
        pc = vpu##name(vm, ptr, pc);                                    \
                                                                        \
        return pc;                                                      \
    }

vpudeclop(nop)
vpudeclop(not)
vpudeclop(and)
vpudeclop(or)
vpudeclop(xor)
vpudeclop(shl)
vpudeclop(shr)
vpudeclop(sar)
vpudeclop(rol)
vpudeclop(ror)
vpudeclop(inc)
vpudeclop(dec)
vpudeclop(add)
vpudeclop(adc)
vpudeclop(adi)
vpudeclop(sub)
vpudeclop(sbc)
vpudeclop(cmp)
vpudeclop(mul)
vpudeclop(div)
vpudeclop(rem)
vpudeclop(lda)
vpudeclop(sta)
vpudeclop(ror)
vpudeclop(jmp)
vpudeclop(bz)
vpudeclop(bnz)
vpudeclop(blt)
vpudeclop(ble)
vpudeclop(bgt)
vpudeclop(bge)
vpudeclop(bo)
vpudeclop(bno)
vpudeclop(bc)
vpudeclop(bnc)
vpudeclop(call)
vpudeclop(thr)
vpudeclop(enter)
vpudeclop(leave)
vpudeclop(ret)
vpudeclop(pop)
vpudeclop(popa)
vpudeclop(push)
vpudeclop(pusha)
vpudeclop(ldr)
vpudeclop(str)
vpudeclop(rst)
vpudeclop(hlt)
vpudeclop(in)
vpudeclop(out)

#endif /* defined(__GNUC__) */

#endif /* __VPU_OP_H__ */

