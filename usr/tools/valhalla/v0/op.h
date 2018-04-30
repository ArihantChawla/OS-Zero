#ifndef __V0_OP_H__
#define __V0_OP_H__

/* FIXME: make this file work */

#include <v0/conf.h>
#include <stddef.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <endian.h>
#include <zero/param.h>
#include <zero/cdefs.h>
#include <zero/trix.h>
#include <zero/fastudiv.h>
#include <v0/mach.h>
#include <v0/vm.h>

#define V0_OP_INVAL                 NULL
#define V0_ADR_INVAL                0x00000000
#define V0_CNT_INVAL                (-1)
#if !defined(__GNUC__)
#define _V0OPFUNC_T FASTCALL INLINE v0ureg
#else
#define _V0OPFUNC_T INLINE          v0ureg
#endif

#if defined(__GNUC__)
#define _v0opadr(x) &&v0op##x
#define _V0OPTAB_T  void *
#else
#define _v0opadr(x) v0##x
typedef v0reg       v0opfunc(struct v0 *vm, uint8_t *ptr, v0ureg pc);
#define _V0OPTAB_T  v0opfunc *
#endif

#if defined(V0_GAME)
#define v0addspeedcnt(n)            (v0speedcnt += (n))
#else
#define v0addspeedcnt(n)
#endif

extern char *v0opnametab[V0_NINST_MAX];

#define v0doxcpt(xcpt)                                                  \
    v0procxcpt(xcpt, __FILE__, __FUNCTION__, __LINE__)
static __inline__ uintptr_t
v0procxcpt(const int xcpt, const char *file, const char *func, const int line)
{
#if defined(_VO_PRINT_XCPT)
    fprintf(stderr, "EXCEPTION: %s - %s/%s:%d\n", #xcpt, file, func, line);
#endif

    exit(xcpt);
}

#if defined(V0_DEBUG_TABS) && 0
#define opaddinfo(proc, inst, handler)                                  \
    do {                                                                \
        long _code = v0mkopid(proc, inst);                              \
                                                                        \
        v0opinfotab[_code].unit = strdup(#proc);                        \
        v0opinfotab[_code].op = strdup(#inst);                          \
        v0opinfotab[_code].func = strdup(#handler);                     \
    } while (0)
#else
#define opaddinfo(unit, op)
#endif

#define v0traceop(vm, op, pc) vasdisasm(vm, op, pc);
#define v0opisvalid(vm, pc) (vm)
#if defined(__GNUC__)
#define opjmp(vm, pc)                                                   \
    do {								\
        struct v0op *_op = v0adrtoptr(vm, pc);				\
									\
	while (v0opisnop(_op)) {					\
	    v0traceop(vm, _op, pc);					\
	    (pc) += sizeof(struct v0op);				\
	}								\
	if (v0opisvalid(_op, pc)) {					\
	    v0traceop(vm, _op, pc);					\
	    if (_op->adr == V0_DIR_ADR || _op->adr == V0_NDX_ADR) {	\
	      (pc) += sizeof(struct v0op) + sizeof(union v0oparg);	\
	    } else {							\
	      (pc) += sizeof(struct v0op);				\
	    }								\
	    goto *jmptab[(_op)->code];					\
	    vm->regs[V0_PC_REG] = (pc);					\
	} else {							\
	    v0doxcpt(V0_TEXT_FAULT);					\
	  								\
	    return V0_TEXT_FAULT;					\
	}								\
    } while (0)
#endif /* defined(__GNUC__) */
#define v0setop(op, str, narg, tab)                                     \
    (vasaddop(#str, op, narg),                                          \
     (v0opnametab[(op)] = #str),                                        \
     ((_V0OPTAB_T *)(tab))[(op)] = _v0opadr(str))

#define v0setopbits(op, bits1, bits2, tab)                              \
    ((tab)[(op)] = (bits1) | ((bits2 << 16)))

#define v0initopbits(tab)                                               \
    do {                                                                \
        v0setopbits(V0_NOP, 0, 0, tab);                                 \
        v0setopbits(V0_INC, V0_R_ARG, 0, tab);                          \
        v0setopbits(V0_DEC, V0_R_ARG, 0, tab);                          \
        v0setopbits(V0_CMP, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_ADD, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_ADC, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_SUB, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_SBB, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_SHL, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_SAL, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_SHR, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_SAR, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_NOT, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_AND, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_XOR, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_LOR, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_ARP, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_MUL, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_MUL, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_LDR, V0_RIM_ARG, V0_R_ARG, tab);                 \
        v0setopbits(V0_STR, V0_RI_ARG, V0_RM_ARG, tab);                 \
        v0setopbits(V0_PSH, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_PSM, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_POP, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_POM, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_JMP, V0_RI_ARG, V0_R_ARG, tab);                  \
        v0setopbits(V0_JMR, V0_RI_ARG, V0_R_ARG, tab);                  \
    } while (0)

#define v0initops(tab)                                                  \
    do {                                                                \
        v0setop(V0_NOP, nop, 0, tab);                                   \
        v0setop(V0_INC, inc, 1, tab);                                   \
        v0setop(V0_DEC, dec, 1, tab);                                   \
        v0setop(V0_CMP, cmp, 2, tab);                                   \
        v0setop(V0_ADD, add, 2, tab);                                   \
        v0setop(V0_ADC, adc, 2, tab);                                   \
        v0setop(V0_SUB, sub, 2, tab);                                   \
        v0setop(V0_SBB, sbb, 2, tab);                                   \
        v0setop(V0_SHL, shl, 2, tab);                                   \
        v0setop(V0_SAL, sal, 2, tab);                                   \
        v0setop(V0_SHR, shr, 2, tab);                                   \
        v0setop(V0_SAR, sar, 2, tab);                                   \
        v0setop(V0_NOT, not, 1, tab);                                   \
        v0setop(V0_AND, and, 2, tab);                                   \
        v0setop(V0_XOR, xor, 2, tab);                                   \
        v0setop(V0_LOR, lor, 2, tab);                                   \
        v0setop(V0_ARP, arp, 1, tab);                                   \
        v0setop(V0_MUL, mul, 2, tab);                                   \
        v0setop(V0_MUL, muh, 2, tab);                                   \
        v0setop(V0_LDR, ldr, 2, tab);                                   \
        v0setop(V0_STR, str, 2, tab);                                   \
        v0setop(V0_PSH, psh, 1, tab);                                   \
        v0setop(V0_PSM, psm, 1, tab);                                   \
        v0setop(V0_POP, pop, 1, tab);                                   \
        v0setop(V0_POM, pom, 1, tab);                                   \
        v0setop(V0_JMP, jmp, 1, tab);                                   \
        v0setop(V0_JMR, jmr, 1, tab);                                   \
    } while (0)

#define v0calcadr(reg, ofs, shift)                                      \
    ((reg) + ((ofs) << (shift)))
#define v0fetcharg(vm, reg, ofs, shift)                                 \
    (*(v0reg *)(&(vm)->mem[v0calcadr(reg, ofs, shift)]))
#define v0getarg1(vm, op)                                               \
    (((op)->adr == V0_REG_ADR)                                          \
     ? (*(v0reg *)(&(vm)->mem[(vm)->regs[(op)->reg1]]))                 \
     : (((op)->adr == V0_IMM_ADR)                                       \
        ? (*(v0reg *)(&(vm)->mem[(op)->val]))                           \
        : (((op)->adr == V0_DIR_ADR)                                    \
           ? (*(v0reg *)(&(vm)->mem[(op)->arg[0].adr]))                 \
           : (v0fetcharg((vm), (op)->reg1, (op)->arg[0].ofs, (op)->parm)))))
#define v0getarg2(vm, op)                                               \
    (((op)->adr == V0_REG_ADR)                                          \
     ? (&((vm)->mem[(vm)->regs[(op)->reg2]]))                           \
     : (((op)->adr == V0_IMM_ADR)                                       \
        ? ((v0reg *)(&(vm)->mem[(op)->val]))                            \
        : (((op)->adr == V0_DIR_ADR)                                    \
           ? (*(v0reg *)(&(vm)->mem[(op)->arg[0].adr]))                 \
           : (v0fetcharg((vm), (op)->reg2, (op)->arg[0].ofs, (op)->parm)))))
#define v0getadr1(vm, op)                                               \
    (((op)->adr == V0_REG_ADR)                                          \
     ? ((v0reg *)(&(vm)->mem[(vm)->regs[(op)->reg1]]))                  \
     : (((op)->adr == V0_IMM_ADR)                                       \
        ? ((v0reg *)(&(vm)->mem[(op)->val]))                            \
        : (((op)->adr == V0_DIR_ADR)                                    \
           ? ((v0reg *)(&(vm)->mem[(op)->arg[0].adr]))                  \
           : ((v0reg *)(&(vm)->mem[v0calcadr((op)->reg1, (op)->arg[0].ofs, (op)->parm)])))))
#define v0getadr2(vm, op)                                               \
    (((op)->adr == V0_REG_ADR)                                          \
     ? ((v0reg *)(&(vm)->mem[(vm)->regs[(op)->reg2]]))                  \
     : (((op)->adr == V0_IMM_ADR)                                       \
        ? ((v0reg *)(&(vm)->mem[(op)->val]))                            \
        : (((op)->adr == V0_DIR_ADR)                                    \
           ? ((v0reg *)(&(vm)->mem[(op)->arg[0].adr]))                  \
           : ((v0reg *)(&(vm)->mem[v0calcadr((op)->reg2, (op)->arg[0].ofs, (op)->parm)])))))
#define v0getjmpadr(vm, op)                                             \
    (((op)->adr == V0_REG_ADR)                                          \
     ? ((vm)->mem[(vm)->regs[(op)->reg1]])                              \
     : (((op)->adr == V0_IMM_ADR)                                       \
        ? ((vm)->mem[(op)->val])                                        \
        : (((op)->adr == V0_DIR_ADR)                                    \
           ? ((vm)->mem[(op)->arg[0].adr])                              \
           : ((vm)->mem[v0calcadr((op)->reg2, (op)->arg[0].ofs, (op)->parm)]))))
#define v0getjmpofs(vm, op)                                             \
    (((op)->adr == V0_REG_ADR)                                          \
     ? ((vm)->mem[(vm)->regs[(op)->reg2]])                              \
     : (((op)->adr == V0_IMM_ADR)                                       \
        ? ((vm)->mem[(op)->val])                                        \
        : (((op)->adr == V0_DIR_ADR)                                    \
           ? ((vm)->mem[(op)->arg[0].adr])                              \
           : ((vm)->mem[v0calcadr((op)->reg2, (op)->arg[0].ofs, (op)->parm)]))))
#define v0getioport(op) ((op)->val)

static _V0OPFUNC_T
v0nop(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];

    pc += sizeof(struct v0op);
    v0addspeedcnt(1);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0inc(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg *sptr = v0getadr1(vm, op);
    v0reg  src = *sptr;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    src++;
    v0addspeedcnt(2);
    vm->regs[V0_PC_REG] = pc;
    *sptr = src;

    return pc;
}

static _V0OPFUNC_T
v0dec(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg *sptr = v0getadr1(vm, op);
    v0reg  src = *sptr;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    src--;
    v0addspeedcnt(2);
    vm->regs[V0_PC_REG] = pc;
    *sptr = src;

    return pc;
}

static _V0OPFUNC_T
v0cmp(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg *dptr = v0getadr2(vm, op);
    v0reg  src = v0getarg1(vm, op);
    v0reg  dest = *dptr;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    dest -= src;
    v0addspeedcnt(8);
    v0clrmsw(vm);
    if (!dest) {
        v0setzf(vm);
    } else if (dest < 0) {
        v0setcf(vm);
    }
    *dptr = dest;
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0add(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg *dptr = v0getadr2(vm, op);
    v0reg  src = v0getarg1(vm, op);
    v0reg  dest = *dptr;
    v0reg  res = src;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    res += dest;
    v0addspeedcnt(4);
    if (res < dest) {
        v0setof(vm);
    }
    vm->regs[V0_PC_REG] = pc;
    *dptr = dest;

    return pc;
}

/* FIXME: set carry-bit */
static _V0OPFUNC_T
v0adc(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg *dptr = v0getadr2(vm, op);
    v0reg  src = v0getarg1(vm, op);
    v0reg  dest = *dptr;
    v0reg  res = src;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    res += dest;
    v0addspeedcnt(4);
    if (res < dest) {
        v0setof(vm);
    }
    vm->regs[V0_PC_REG] = pc;
    *dptr = dest;

    return pc;
}

static _V0OPFUNC_T
v0sub(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg *dptr = v0getadr2(vm, op);
    v0reg  src = v0getarg1(vm, op);
    v0reg  dest = *dptr;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    dest -= src;
    v0addspeedcnt(4);
    vm->regs[V0_PC_REG] = pc;
    *dptr = dest;

    return pc;
}

static _V0OPFUNC_T
v0sbb(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg *dptr = v0getadr2(vm, op);
    v0reg  src = v0getarg1(vm, op);
    v0reg  dest = *dptr;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    dest -= src;
    v0addspeedcnt(4);
    vm->regs[V0_PC_REG] = pc;
    *dptr = dest;

    return pc;
}

static _V0OPFUNC_T
v0shl(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg *dptr = v0getadr2(vm, op);
    v0reg  src = v0getarg1(vm, op);
    v0reg  dest = *dptr;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    dest <<= src;
    v0addspeedcnt(4);
    vm->regs[V0_PC_REG] = pc;
    *dptr = dest;

    return pc;
}

static _V0OPFUNC_T
v0sal(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg *dptr = v0getadr2(vm, op);
    v0reg  src = v0getarg1(vm, op);
    v0reg  dest = *dptr;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    dest <<= src;
    v0addspeedcnt(4);
    vm->regs[V0_PC_REG] = pc;
    *dptr = dest;

    return pc;
}

static _V0OPFUNC_T
v0shr(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg *dptr = v0getadr2(vm, op);
    v0reg  src = v0getarg1(vm, op);
    v0reg  dest = *dptr;
    v0reg  fill = ~((v0reg)0) >> src;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    dest >>= src;
    v0addspeedcnt(4);
    dest &= fill;
    vm->regs[V0_PC_REG] = pc;
    *dptr = dest;

    return pc;
}

static _V0OPFUNC_T
v0sar(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg *dptr = v0getadr2(vm, op);
    v0reg  src = v0getarg1(vm, op);
    v0reg  dest = *dptr;
    v0reg  mask = ~(v0reg)0;
#if (WORDSIZE == 8)
    v0reg  fill = (((dest) & (INT64_C(1) << 63))
                   ? (mask >> (64 - src))
                   : 0);
#else
    v0reg  fill = (((dest) & (INT32_C(1) << 31))
                   ? (mask >> (32 - src))
                   : 0);
#endif

    dest >>= src;
    fill = -fill << (CHAR_BIT * sizeof(v0reg) - src);
    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    v0addspeedcnt(4);
    dest &= fill;
    vm->regs[V0_PC_REG] = pc;
    *dptr = dest;

    return pc;
}

static _V0OPFUNC_T
v0not(struct v0 *vm, struct v0op *op)
{
    v0ureg  pc = vm->regs[V0_PC_REG];
    v0reg  *sptr = v0getadr1(vm, op);
    v0reg   src = *sptr;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    src = ~src;
    v0addspeedcnt(1);
    vm->regs[V0_PC_REG] = pc;
    *sptr = src;

    return pc;
}

static _V0OPFUNC_T
v0and(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg *dptr = v0getadr2(vm, op);
    v0reg  src = v0getarg1(vm, op);
    v0reg  dest = *dptr;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    dest &= src;
    v0addspeedcnt(2);
    vm->regs[V0_PC_REG] = pc;
    *dptr = dest;

    return pc;
}

static _V0OPFUNC_T
v0xor(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg *dptr = v0getadr2(vm, op);
    v0reg  src = v0getarg1(vm, op);
    v0reg  dest = *dptr;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    dest ^= src;
    v0addspeedcnt(2);
    vm->regs[V0_PC_REG] = pc;
    *dptr = dest;

    return pc;
}

static _V0OPFUNC_T
v0lor(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg *dptr = v0getadr2(vm, op);
    v0reg  src = v0getarg1(vm, op);
    v0reg  dest = *dptr;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    dest |= src;
    v0addspeedcnt(2);
    vm->regs[V0_PC_REG] = pc;
    *dptr = dest;

    return pc;
}

/* ARP */

static _V0OPFUNC_T
v0arp(struct v0 *vm, struct v0op *op)
{
    v0ureg  pc = vm->regs[V0_PC_REG];
    v0wreg  src = v0getarg1(vm, op);
    double *dptr = (double *)v0getadr2(vm, op);
    double  res = 1.0 / src;

    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    *dptr = res;

    return pc;
}

static _V0OPFUNC_T
v0mul(struct v0 *vm, struct v0op *op)
{
    v0ureg  pc = vm->regs[V0_PC_REG];
    v0reg  *dptr = v0getadr2(vm, op);
    double  src = (double)v0getarg1(vm, op);
    double  res = *(double *)dptr;
    v0wreg  dest;

    res *= src;
    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    dest = (v0wreg)res;
    vm->regs[V0_PC_REG] = pc;
    dest &= 0xffffffff;
    v0addspeedcnt(16);
    *dptr = (v0reg)dest;

    return pc;
}

static _V0OPFUNC_T
v0muh(struct v0 *vm, struct v0op *op)
{
    v0ureg  pc = vm->regs[V0_PC_REG];
    v0reg  *dptr = v0getadr2(vm, op);
    double  src = (double)v0getarg1(vm, op);
    double  res = *(double *)dptr;
    v0wreg  dest;

    res *= src;
    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    dest = (v0wreg)res;
    vm->regs[V0_PC_REG] = pc;
    dest >>= 32;
    v0addspeedcnt(16);
    *dptr = (v0reg)dest;

    return pc;
}

static _V0OPFUNC_T
v0ldr(struct v0 *vm, struct v0op *op)
{
    v0ureg  pc = vm->regs[V0_PC_REG];
    v0reg   reg;
    v0reg  *adr = v0getadr1(vm, op);
    v0reg   src = 0;
    v0ureg  usrc = 0;
    v0reg   parm = op->parm;
    v0reg  *dptr = v0regtoptr(vm, op->reg2);
    v0reg  *sptr;
    v0reg   mask;

    if (!adr) {
        v0doxcpt(V0_INV_MEM_ADR);
    }
    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    if (op->adr == V0_REG_ADR) {
        reg = op->reg1;
        mask = 1 << (parm + CHAR_BIT);
        sptr = v0regtoptr(vm, reg);
        mask--;
        src = *sptr;
        v0addspeedcnt(4);
        src &= mask;
        *dptr |= src;
    } else {
        v0addspeedcnt(16);
        if (v0opissigned(op)) {
            switch (parm) {
                case 0:
                    src = *(int8_t *)adr;

                    break;
                case 1:
                    src = *(int16_t *)adr;

                    break;
                case 2:
                    src = *(int32_t *)adr;

                    break;
                case 3:
                    v0doxcpt(V0_INV_MEM_READ);

                    break;
            }
            *dptr = src;
        } else {
            switch (op->parm) {
                case 0:
                    usrc = *(uint8_t *)adr;

                    break;
                case 1:
                    usrc = *(uint16_t *)adr;

                    break;
                case 2:
                    usrc = *(uint32_t *)adr;

                    break;
                case 3:
                    v0doxcpt(V0_INV_MEM_READ);

                    break;
            }
            *(v0ureg *)dptr = usrc;
        }
    }
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0str(struct v0 *vm, struct v0op *op)
{
    v0ureg  pc = vm->regs[V0_PC_REG];
    v0reg   reg;
    v0reg  *adr = v0getadr2(vm, op);
    v0reg   src;
    v0ureg  usrc;
    v0reg   parm = op->parm;
    v0reg  *dptr;
    v0reg  *sptr = v0adrtoptr(vm, op->reg1);
    v0reg   mask;

    if (!adr) {
        v0doxcpt(V0_INV_MEM_ADR);
    }
    if (op->adr == V0_REG_ADR || op->adr == V0_IMM_ADR) {
        pc += sizeof(struct v0op);
    } else {
        pc += sizeof(struct v0op) + sizeof(union v0oparg);
    }
    if (op->adr == V0_REG_ADR) {
        reg = op->reg2;
        mask = 1 << (parm + CHAR_BIT);
        src = *sptr;
        mask--;
        v0addspeedcnt(4);
        dptr = v0regtoptr(vm, reg);
        src &= mask;
        *dptr |= src;
    } else {
        v0addspeedcnt(16);
        if (v0opissigned(op)) {
            switch (op->parm) {
                case 0:
                    src = *(int8_t *)adr;
                    *(int8_t *)adr = (int8_t)src;

                    break;
                case 1:
                    src = *(int16_t *)adr;
                    *(int16_t *)adr = (int16_t)src;

                    break;
                case 2:
                    src = *(int32_t *)adr;
                    *(int32_t *)adr = (int32_t)src;

                    break;
                case 3:
                    v0doxcpt(V0_INV_MEM_WRITE);

                    break;
            }
        } else {
            switch (op->parm) {
                case 0:
                    usrc = *(uint8_t *)adr;
                    *(uint8_t *)adr = (uint8_t)usrc;

                    break;
                case 1:
                    usrc = *(uint16_t *)adr;
                    *(uint16_t *)adr = (uint16_t)usrc;

                    break;
                case 2:
                    usrc = *(int32_t *)adr;
                    *(uint32_t *)adr = (uint32_t)usrc;

                    break;
                case 3:
                    v0doxcpt(V0_INV_MEM_WRITE);

                    break;
            }
        }
    }
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0jmp(struct v0 *vm, struct v0op *op)
{
    v0reg pc = v0getjmpadr(vm, op);

    v0addspeedcnt(8);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0jmr(struct v0 *vm, struct v0op *op)
{
    v0reg  ofs = v0getjmpofs(vm, op);
    v0ureg pc = vm->regs[V0_PC_REG];

    pc += ofs;
    v0addspeedcnt(8);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0biz(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg  dest = v0getjmpadr(vm, op);

    if (v0zfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(16);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0bnz(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg  dest = v0getjmpadr(vm, op);

    if (!v0zfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(16);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0blt(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg  dest = v0getjmpadr(vm, op);

    if (!v0ofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(16);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0ble(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg  dest = v0getjmpadr(vm, op);

    if (!v0ofset(vm) || v0zfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(16);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0bgt(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg  dest = v0getjmpadr(vm, op);

    if (v0ofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(16);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0bge(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg  dest = v0getjmpadr(vm, op);

    if (v0ofset(vm) || v0zfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(16);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0bio(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg  dest = v0getjmpadr(vm, op);

    if (v0ofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(16);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0bno(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg  dest = v0getjmpadr(vm, op);

    if (!v0ofset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(16);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0bic(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg  dest = v0getjmpadr(vm, op);

    if (v0cfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(16);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0bnc(struct v0 *vm, struct v0op *op)
{
    v0ureg pc = vm->regs[V0_PC_REG];
    v0reg  dest = v0getjmpadr(vm, op);

    if (!v0cfset(vm)) {
        pc = dest;
    } else {
        pc += sizeof(struct v0op);
    }
    v0addspeedcnt(16);
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

/* return from subroutine;
 * - pop return address
 */
/*
 * stack after ret
 * ---------------
 * r7
 * ...
 * r0
 * argN
 * ...
 * arg0 <- sp
 * retadr
 */
static _V0OPFUNC_T
v0rt0(struct v0 *vm, struct v0op *op)
{
    v0ureg  sp = vm->regs[V0_SP_REG];
    v0reg  *sptr = v0adrtoptr(vm, sp);
    v0ureg  pc;

    v0addspeedcnt(16);
    sp -= sizeof(v0reg);
    pc = *sptr;
    vm->regs[V0_SP_REG] = sp;
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0rt1(struct v0 *vm, struct v0op *op)
{
    v0ureg  sp = vm->regs[V0_SP_REG];
    v0reg   nb = v0getarg1(vm, op);
    v0reg  *sptr;
    v0ureg  pc;

    sp -= nb;
    v0addspeedcnt(16);
    sp -= sizeof(v0reg);
    sptr = v0adrtoptr(vm, sp);
    pc = *sptr;
    vm->regs[V0_SP_REG] = sp;
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

#if 0
void
v0mkframe(struct v0 *vm, size_t narg, v0reg *tab)
{
    v0ureg  pc = vm->regs[V0_PC_REG];
    v0ureg  sp = vm->regs[V0_SP_REG];
    v0reg  *rptr = v0regtoptr(vm, V0_R0_REG);
    v0reg  *sptr;
    v0reg   n;

    sp -= V0_SAVE_REGS;
    pc += sizeof(struct v0op);
    sptr = v0adrtoptr(vm, sp);
    sptr[V0_R0_REG] = rptr[V0_R0_REG];
    sptr[V0_R1_REG] = rptr[V0_R1_REG];
    sptr[V0_R2_REG] = rptr[V0_R2_REG];
    sptr[V0_R3_REG] = rptr[V0_R3_REG];
    n = narg;
    sptr[V0_R4_REG] = rptr[V0_R4_REG];
    sptr[V0_R5_REG] = rptr[V0_R5_REG];
    sptr[V0_R6_REG] = rptr[V0_R6_REG];
    sptr[V0_R7_REG] = rptr[V0_R7_REG];
    tab += narg;
    while (n > 8) {
        n = min(narg, 4);
        sptr -= n;
        tab -= n;
        sp -= n;
        switch (narg) {
            case 4:
                sptr[3] = tab[3];
            case 3:
                sptr[2] = tab[2];
            case 2:
                sptr[1] = tab[1];
            case 1:
                sptr[0] = tab[0];
            case 0:

                break;
        }
    }
    vm->regs[V0_SP_REG] = sp;
    if (n) {
        tab -= narg;
        switch (n) {
            case 8:
                rptr[7] = tab[7];
            case 7:
                rptr[6] = tab[6];
            case 6:
                rptr[5] = tab[5];
            case 5:
                rptr[4] = tab[4];
            case 4:
                rptr[3] = tab[3];
            case 3:
                rptr[2] = tab[2];
            case 2:
                rptr[1] = tab[1];
            case 1:
                rptr[0] = tab[0];

                break;
        }
    }

    return;
}
#endif

/*
 * call
 * ----
 * - push return address
 *
 * stack frame after call - CREATED BY COMPILER
 * ----------------------
 * r7
 * r6
 * r5
 * r4
 * r3
 * r2
 * r1
 * r0
 * argN
 * ...
 * arg0 <- after compiler has generated stack frame
 * retadr <- sp
 */
static _V0OPFUNC_T
v0csr(struct v0 *vm, struct v0op *op)
{
    v0ureg  pc = vm->regs[V0_PC_REG];
    v0ureg  sp = vm->regs[V0_SP_REG];
    v0ureg  dest = v0getarg1(vm, op);
    v0reg  *sptr;

    pc += sizeof(struct v0op);
    sp -= sizeof(v0reg);
    sptr = v0adrtoptr(vm, sp);
    if (op->adr == V0_REG_ADR) {
        v0addspeedcnt(4);
    } else {
        v0addspeedcnt(8);
    }
    *sptr = pc;
    vm->regs[V0_PC_REG] = dest;
    vm->regs[V0_SP_REG] = sp;

    return dest;
}

/* create subroutine stack-frame;
 * - push frame pointer
 * - copy stack pointer to frame pointer
 * - push callee-save registers r8..r15
 * - allocate room for local variables on stack
 */
/*
 * stack after enter
 * -----------------
 * retadr
 * oldfp <- fp
 * r15
 * ...
 * r8
 * var0
 * ...
 * varN <- sp
 */
static _V0OPFUNC_T
v0beg(struct v0 *vm, struct v0op *op)
{
    v0ureg  pc = vm->regs[V0_PC_REG];
    v0ureg  fp = vm->regs[V0_SP_REG];
    v0ureg  sp = vm->regs[V0_SP_REG];
    v0reg  *rptr = v0regtoptr(vm, V0_R0_REG);
    v0reg   nb = v0getarg1(vm, op);
    v0reg  *sptr;

    /* set stack frame up */
    sp -= sizeof(v0reg);
    v0addspeedcnt(32);
    sptr = v0adrtoptr(vm, sp);
    vm->regs[V0_FP_REG] = sp;
    *sptr = fp;
    pc += sizeof(struct v0op);
    sptr -= V0_SAVE_REGS;
    sp -= V0_SAVE_REGS * sizeof(v0reg);
    sptr[V0_R8_REG] = rptr[V0_R8_REG];
    sptr[V0_R9_REG] = rptr[V0_R9_REG];
    sptr[V0_R10_REG] = rptr[V0_R10_REG];
    sptr[V0_R11_REG] = rptr[V0_R11_REG];
    sp -= nb;
    sptr[V0_R12_REG] = rptr[V0_R12_REG];
    sptr[V0_R13_REG] = rptr[V0_R13_REG];
    sptr[V0_R14_REG] = rptr[V0_R14_REG];
    sptr[V0_R15_REG] = rptr[V0_R15_REG];
    vm->regs[V0_SP_REG] = sp;
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

/* destroy subroutine stack-frame
 * - return value is in r0
 * - deallocate local variables
 * - pop callee save registers r8..r15
 * - pop caller frame pointer
 */
/*
 * stack after fin
 * -----------------
 * retadr <- sp
 * oldfp
 * callee save registers r8..r15
 * local variables
 */
static _V0OPFUNC_T
v0fin(struct v0 *vm, struct v0op *op)
{
    v0ureg  pc = vm->regs[V0_PC_REG];
    v0ureg  sp = vm->regs[V0_FP_REG];
    v0ureg  fp;
    v0reg  *rptr = v0regtoptr(vm, V0_R0_REG);
    v0reg  *sptr;

    pc += sizeof(struct v0op);
    v0addspeedcnt(8);
    rptr[V0_R8_REG] = sptr[0];
    rptr[V0_R9_REG] = sptr[1];
    sptr = v0adrtoptr(vm, sp);
    rptr[V0_R10_REG] = sptr[2];
    rptr[V0_R11_REG] = sptr[3];
    fp = *sptr;
    rptr[V0_R12_REG] = sptr[4];
    rptr[V0_R13_REG] = sptr[5];
    sp += sizeof(v0reg);
    rptr[V0_R14_REG] = sptr[6];
    rptr[V0_R15_REG] = sptr[7];
    vm->regs[V0_PC_REG] = pc;
    vm->regs[V0_FP_REG] = fp;
    vm->regs[V0_SP_REG] = sp;

    return pc;
}

/* call epilogue;
 * - number of return values (0, 1, 2) in op->parm
 * - get possible return value from r0
 * - deallocate stack arguments
 * - restore caller-save registers r0..r7
 * - set r0 to return value
 */
static _V0OPFUNC_T
v0rmframe(struct v0 *vm, size_t narg)
{
    v0reg  *rptr = v0regtoptr(vm, V0_R0_REG);
    v0ureg  pc = vm->regs[V0_PC_REG];
    v0ureg  sp = vm->regs[V0_SP_REG];
    v0reg  *sptr;

    /* adjust SP past stack arguments */
    sp += narg;
    pc += sizeof(struct v0op);
    /* store r0 and r1 in ret and rethi */
    sptr = v0adrtoptr(vm, sp);
    sp += V0_SAVE_REGS * sizeof(struct v0op);
    /* restore caller-save registers */
    rptr[V0_R0_REG] = sptr[V0_R0_REG];
    rptr[V0_R1_REG] = sptr[V0_R1_REG];
    rptr[V0_R2_REG] = sptr[V0_R2_REG];
    rptr[V0_R3_REG] = sptr[V0_R3_REG];
    rptr[V0_R4_REG] = sptr[V0_R4_REG];
    rptr[V0_R5_REG] = sptr[V0_R5_REG];
    rptr[V0_R6_REG] = sptr[V0_R6_REG];
    rptr[V0_R7_REG] = sptr[V0_R7_REG];
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0psh(struct v0 *vm, struct v0op *op)
{
    v0ureg  pc = vm->regs[V0_PC_REG];
    v0reg   src = v0getarg1(vm, op);
    v0ureg  sp = vm->regs[V0_SP_REG];
    v0reg  *sptr = v0adrtoptr(vm, src);
    v0reg  *dptr = v0adrtoptr(vm, sp);

    sp -= sizeof(v0reg);
    pc += sizeof(struct v0op);
    if (op->adr == V0_REG_ADR) {
        v0addspeedcnt(4);
    } else {
        v0addspeedcnt(8);
        pc += sizeof(union v0oparg);
    }
    *dptr = *sptr;
    vm->regs[V0_SP_REG] = sp;
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0psm(struct v0 *vm, struct v0op *op)
{
    v0ureg  pc = vm->regs[V0_PC_REG];
    v0ureg  sp = vm->regs[V0_SP_REG];
    v0reg   map = v0getarg1(vm, op);
    v0reg  *sptr = v0regtoptr(vm, V0_R15_REG);
    v0reg  *dptr = v0adrtoptr(vm, sp);

    pc += sizeof(struct v0op);
    v0addspeedcnt(8 * n);
    while (map) {
        if (map & 0x8000) {
            dptr[0] = sptr[0];
            dptr--;
            sp--;
        }
        map <<= 1;
        sptr--;
    }
    vm->regs[V0_SP_REG] = sp;
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0pop(struct v0 *vm, struct v0op *op)
{
    v0ureg  pc = vm->regs[V0_PC_REG];
    v0ureg  sp = vm->regs[V0_SP_REG];
    v0reg  *src = v0adrtoptr(vm, sp);
    v0reg  *dest = v0getadr2(vm, op);

    pc += sizeof(struct v0op);
    sp += sizeof(v0reg);
    *dest = *src;
    v0addspeedcnt(4);
    vm->regs[V0_SP_REG] = sp;
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

static _V0OPFUNC_T
v0pom(struct v0 *vm, struct v0op *op)
{
    v0ureg  pc = vm->regs[V0_PC_REG];
    v0ureg  sp = vm->regs[V0_SP_REG];
    v0reg   map = v0getarg1(vm, op);
    v0reg  *sptr = v0adrtoptr(vm, sp);
    v0reg  *dptr = v0regtoptr(vm, V0_R15_REG);

    pc += sizeof(struct v0op);
    v0addspeedcnt(4 * n);
    while (map) {
        if (map & 0x8000) {
            dptr[0] = sptr[0];
            sptr--;
            sp--;
        }
        map <<= 1;
        dptr--;
    }
    vm->regs[V0_SP_REG] = sp;
    vm->regs[V0_PC_REG] = pc;

    return pc;
}

#endif /* __V0_OP_H__ */

