#ifndef __V86_OP_H__
#define __V86_OP_H__

#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <v86/vm.h>

/* instruction set */

typedef void v86opfunc(struct v86vm *vm, struct v86op *op);

struct v86vmop {
    long       narg;
    v86opfunc *func;
};

/* NOT r/a, r */
#define _v86opnot(vm, op, ret)                                          \
    do {                                                                \
        _v86getargs(vm, op);                                            \
        dest = ~src;                                                    \
        pc += sizeof(uint32_t);                                         \
        vmsetureg(dptr, dest);                                          \
        ret = dest;                                                     \
        vmsetpc(pc);                                                    \
    } while (0)

/* FIXME: addressing modes below */

/* NOT r, r */
#define _v86opand(vm, op, ret)                                          \
    do {                                                                \
        _v86getargs(vm, op);                                            \
        dest &= src;                                                    \
        pc += sizeof(uint32_t);                                         \
        vmsetureg(dptr, dest);                                          \
        ret = dest;                                                     \
        vmsetpc(pc);                                                    \
    } while (0)
 
/* OR r, r */
#define _v86opor(vm, op, ret)                                           \
    do {                                                                \
        _v86getargs(vm, op);                                            \
        dest |= src;                                                    \
        pc += sizeof(uint32_t);                                         \
        vmsetureg(dptr, dest);                                          \
        ret = dest;                                                     \
        vmsetpc(pc);                                                    \
    } while (0)

/* XOR r, r */
#define _v86opxor(vm, op, ret)                                          \
    do {                                                                \
        _v86getargs(vm, op);                                            \
        dest ^= src;                                                    \
        pc += sizeof(uint32_t);                                         \
        vmsetureg(dptr, dest);                                          \
        ret = dest;                                                     \
        vmsetpc(pc);                                                    \
    } while (0)

/* SHL i, r */
#define _v86opshl(vm, op, ret)                                          \
    do {                                                                \
        v86reg res;                                                     \
                                                                        \
        _v86getargs(vm, op);                                            \
        res = dest << src;                                              \
        pc += sizeof(uint32_t);                                         \
        vmsetureg(dptr, res);                                           \
        ret = res;                                                      \
        vmsetpc(pc);                                                    \
    } while (0)

/* SHR i, r */
#define _v86opshr(vm, op, ret)                                          \
    do {                                                                \
        v86reg res;                                                     \
                                                                        \
        ret = res;                                                      \
    } while (0)

/* SAR i, r */
#define _v86opsar(vm, op, ret)                                          \
    do {                                                                \
        v86reg res;                                                     \
                                                                        \
        ret = res;                                                      \
    } while (0)

/* NOT r/a, r */
#define _v86opadd(vm, op, ret)                                          \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86opsub(vm, op, ret)                                          \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86opcmp(vm, op, ret)                                          \
    do {                                                                \
        v86reg res;                                                     \
                                                                        \
        ret = res;                                                      \
    } while (0)

/* NOT r/a, r */
#define _v86opmul(vm, op, ret)                                          \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86opdiv(vm, op, ret)                                          \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86opjmp(vm, op, ret)                                          \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86opljmp(vm, op, ret)                                         \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86opjz(vm, op, ret)                                           \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86opjnz(vm, op, ret)                                          \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86opjc(vm, op, ret)                                           \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86opjnc(vm, op, ret)                                          \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86opjo(vm, op, ret)                                           \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86opjno(vm, op, ret)                                          \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86opjle(vm, op, ret)                                          \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86opjgt(vm, op, ret)                                          \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86opjge(vm, op, ret)                                          \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86opcall(vm, op, ret)                                         \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86opret(vm, op, ret)                                          \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86opldr(vm, op, ret)                                          \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86opstr(vm, op, ret)                                          \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86oppush(vm, op, ret)                                         \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86oppusha(vm, op, ret)                                        \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86oppop(vm, op, ret)                                          \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86oppopa(vm, op, ret)                                         \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86opin(vm, op, ret)                                           \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86opout(vm, op, ret)                                          \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

/* NOT r/a, r */
#define _v86ophlt(vm, op, ret)                                          \
    do {                                                                \
        ret = dest;                                                     \
    } while (0)

#if (V86_WORD_SIZE == 4)
#define v86chkof(res) ((res) & ~(v86ureg)UINT32_MAX)
#define v86chkuf(res) ((res) < (v86ureg)INT32_MIN)
#endif

#define _v86getargs(vm, op)                                             \
    dptr = NULL;                                                        \
    dest = (((op)->opflg & V86_DESTINATION_REGISTER)                    \
            ? (dptr = &(vm)->usrregs[(op)->dreg], *dptr)                \
            : (v86reg)((op)->arg));                                     \
    src = (((op)->opflg & V86_IMMEDIATE_OPERAND)                        \
           ? (v86reg)((op)->imm8)                                       \
           : (((op)->opflg & V86_SOURCE_REGISTER)                       \
              ? (v86reg)((vm)->usrregs[(op)->sreg])                     \
              : (v86reg)((op)->arg)));                                  \
    pc = (vm)->sysregs[V86_PC_REGISTER]

#define vmsetpc(val)                                                    \
    ((vm)->sysregs[V86_PC_REGISTER] = (val))
#define vmsetureg(ptr, val)                                             \
    (*ptr = (val))

#endif /* __V86_OP_H__ */

