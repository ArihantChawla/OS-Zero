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

void v86opnot(struct v86vm *vm, struct v86op *op);
void v86opand(struct v86vm *vm, struct v86op *op);
void v86opor(struct v86vm *vm, struct v86op *op);
void v86opxor(struct v86vm *vm, struct v86op *op);
void v86opshl(struct v86vm *vm, struct v86op *op);
void v86opshr(struct v86vm *vm, struct v86op *op);
void v86opsar(struct v86vm *vm, struct v86op *op);
void v86opadd(struct v86vm *vm, struct v86op *op);
void v86opsub(struct v86vm *vm, struct v86op *op);
void v86opcmp(struct v86vm *vm, struct v86op *op);
void v86opmul(struct v86vm *vm, struct v86op *op);
void v86opdiv(struct v86vm *vm, struct v86op *op);
void v86opjmp(struct v86vm *vm, struct v86op *op);
void v86opljmp(struct v86vm *vm, struct v86op *op);
void v86opjz(struct v86vm *vm, struct v86op *op);
void v86opjnz(struct v86vm *vm, struct v86op *op);
void v86opjc(struct v86vm *vm, struct v86op *op);
void v86opjnc(struct v86vm *vm, struct v86op *op);
void v86opjo(struct v86vm *vm, struct v86op *op);
void v86opjno(struct v86vm *vm, struct v86op *op);
void v86opjle(struct v86vm *vm, struct v86op *op);
void v86opjgt(struct v86vm *vm, struct v86op *op);
void v86opjge(struct v86vm *vm, struct v86op *op);
void v86opcall(struct v86vm *vm, struct v86op *op);
void v86opret(struct v86vm *vm, struct v86op *op);
void v86opldr(struct v86vm *vm, struct v86op *op);
void v86opstr(struct v86vm *vm, struct v86op *op);
void v86oppush(struct v86vm *vm, struct v86op *op);
void v86oppusha(struct v86vm *vm, struct v86op *op);
void v86oppop(struct v86vm *vm, struct v86op *op);
void v86oppopa(struct v86vm *vm, struct v86op *op);
void v86opin(struct v86vm *vm, struct v86op *op);
void v86opout(struct v86vm *vm, struct v86op *op);
void v86ophlt(struct v86vm *vm, struct v86op *op);

#endif /* __V86_OP_H__ */

