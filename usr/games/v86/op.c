#include <stddef.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <v86/vm.h>
#include <v86/op.h>

static struct v86vmop v86optab[V86_OPERATIONS]
= {
    { 0, NULL },        // V86_NOP
    { 1, v86opnot },    // V86_NOT
    { 2, v86opand },    // V86_AND
    { 2, v86opor },     // V86_OR
    { 2, v86opxor },    // V86_XOR
    { 2, v86opshl },    // V86_SHL
    { 2, v86opshr },    // V86_SHR
    { 2, v86opsar },    // V86_SAR
    { 2, v86opadd },    // V86_ADD
    { 2, v86opsub },    // V86_SUB
    { 2, v86opcmp },    // V86_CMP
    { 2, v86opmul },    // V86_MUL
    { 2, v86opdiv },    // V86_DIV
    { 1, v86opjmp },    // V86_JMP
    { 1, v86opljmp },   // V86_LJMP
    { 1, v86opjz },     // V86_JZ
    { 1, v86opjnz },    // V86_JNZ
    { 1, v86opjc },     // V86_JC
    { 1, v86opjnc },    // V86_JNC
    { 1, v86opjo },     // V86_JO
    { 1, v86opjno },    // V86_JNO
    { 1, v86opjle },    // V86_JLE
    { 1, v86opjgt },    // V86_JGT
    { 1, v86opjge },    // V86_JGE
    { 1, v86opcall },   // V86_CALL
    { 1, v86opret },    // V86_RET
    { 2, v86opldr },    // V86_LDR
    { 2, v86opstr },    // V86_STR
    { 1, v86oppush },   // V86_PUSH
    { 0, v86oppusha },  // V86_PUSHA
    { 1, v86oppop },    // V86_POP
    { 0, v86oppopa },   // V86_POPA
    { 2, v86opin },     // V86_IN
    { 2, v86opout },    // V86_OUT
    { 2, v86ophlt }     // V86_HLT
};

#define _v86getreg1(vm, op)                                             \
    v86reg sreg = op->sreg;                                             \
    v86reg pc = vm->sysregs[V86_PC_REGISTER];                           \
    v86reg src = vm->usrregs[sreg]

#define _v86getreg2(vm, op)                                             \
    v86reg dreg = op->dreg;                                             \
    v86reg  dest = vm->usrregs[dreg]

/* NOT r/a, r */
void
v86opnot(struct v86vm *vm, struct v86op *op)
{
    v86reg res;

    _v86getreg1(vm, op);
    res = ~src;
    pc += sizeof(uint32_t);
    vm->usrregs[sreg] = res;
    vm->sysregs[V86_PC_REGISTER] = pc;

    return;
}

/* FIXME: addressing modes below */

/* NOT r/a, r */
void
v86opand(struct v86vm *vm, struct v86op *op)
{
    _v86getreg2(vm, op);
    _v86getreg1(vm, op);
    dest &= src;
    pc += sizeof(uint32_t);
    vm->usrregs[sreg] = dest;
    vm->sysregs[V86_PC_REGISTER] = pc;

    return;
}

/* NOT r/a, r */
void
v86opor(struct v86vm *vm, struct v86op *op)
{
    _v86getreg2(vm, op);
    _v86getreg1(vm, op);
    dest |= src;
    pc += sizeof(uint32_t);
    vm->usrregs[sreg] = dest;
    vm->sysregs[V86_PC_REGISTER] = pc;

    return;
}

/* NOT r/a, r */
void
v86opxor(struct v86vm *vm, struct v86op *op)
{
    _v86getreg2(vm, op);
    _v86getreg1(vm, op);
    dest ^= src;
    pc += sizeof(uint32_t);
    vm->usrregs[sreg] = dest;
    vm->sysregs[V86_PC_REGISTER] = pc;

    return;
}

/* NOT r/a, r */
void
v86opshl(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opshr(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opsar(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opadd(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opsub(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opcmp(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opmul(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opdiv(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opjmp(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opljmp(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opjz(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opjnz(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opjc(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opjnc(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opjo(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opjno(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opjle(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opjgt(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opjge(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opcall(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opret(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opldr(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opstr(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86oppush(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86oppusha(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86oppop(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86oppopa(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opin(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86opout(struct v86vm *vm, struct v86op *op)
{
}

/* NOT r/a, r */
void
v86ophlt(struct v86vm *vm, struct v86op *op)
{
}

