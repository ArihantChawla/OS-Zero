#ifndef __ZVM_OP_H__
#define __ZVM_OP_H__

void zvmopnot(struct zvmopcode *op);
void zvmopand(struct zvmopcode *op);
void zvmopor(struct zvmopcode *op);
void zvmopxor(struct zvmopcode *op);
void zvmopshr(struct zvmopcode *op);
void zvmopsar(struct zvmopcode *op);
void zvmopshl(struct zvmopcode *op);
void zvmopror(struct zvmopcode *op);
void zvmoprol(struct zvmopcode *op);
void zvmopinc(struct zvmopcode *op);
void zvmopdec(struct zvmopcode *op);
void zvmopadd(struct zvmopcode *op);
void zvmopsub(struct zvmopcode *op);
void zvmopcmp(struct zvmopcode *op);
void zvmopmul(struct zvmopcode *op);
void zvmopdiv(struct zvmopcode *op);
void zvmopmod(struct zvmopcode *op);
void zvmopjmp(struct zvmopcode *op);
void zvmopbz(struct zvmopcode *op);
void zvmopbnz(struct zvmopcode *op);
void zvmopblt(struct zvmopcode *op);
void zvmopble(struct zvmopcode *op);
void zvmopbgt(struct zvmopcode *op);
void zvmopbge(struct zvmopcode *op);
void zvmopbo(struct zvmopcode *op);
void zvmopbno(struct zvmopcode *op);
void zvmopbc(struct zvmopcode *op);
void zvmopbnc(struct zvmopcode *op);
void zvmoppop(struct zvmopcode *op);
void zvmoppush(struct zvmopcode *op);
void zvmoppusha(struct zvmopcode *op);
void zvmopmovl(struct zvmopcode *op);
void zvmopmovb(struct zvmopcode *op);
void zvmopmovw(struct zvmopcode *op);
#if (!ZAS32BIT)
void zvmopmovq(struct zvmopcode *op);
#endif
void zvmopcall(struct zvmopcode *op);
void zvmopenter(struct zvmopcode *op);
void zvmopleave(struct zvmopcode *op);
void zvmopret(struct zvmopcode *op);
void zvmoplmsw(struct zvmopcode *op);
void zvmopsmsw(struct zvmopcode *op);
void zvmopreset(struct zvmopcode *op);
void zvmophlt(struct zvmopcode *op);

#define ZVMOPNOP    0xff // dummy operation
/* logical operations */
#define ZVMOPLOGIC  0x00
#define ZVMOPNOT    0x01 // 2's complement
#define ZVMOPAND    0x02 // logical AND
#define ZVMOPOR     0x03 // logical OR
#define ZVMOPXOR    0x04 // logical exclusive OR
/* shift operations */
#define ZVMOPSHIFT  0x01
#define ZVMOPSHR    0x01 // logical shift right (fill with zero)
#define ZVMOPSAR    0x02 // arithmetic shift right (fill with sign)
#define ZVMOPSHL    0x03 // shift left (fill with zero)
#define ZVMOPROR    0x04 // rotate right
#define ZVMOPROL    0x05 // rotate left
/* arithmetic operations */
#define ZVMOPARITH  0x02
#define ZVMOPINC    0x01 // increment by one
#define ZVMOPDEC    0x02 // decrement by one
#define ZVMOPADD    0x03 // addition
#define ZVMOPSUB    0x04 // subtraction
#define ZVMOPCMP    0x05 // compare
#define ZVMOPMUL    0x06 // multiplication
#define ZVMOPDIV    0x07 // division
#define ZVMOPMOD    0x08 // modulus
/* branch instructions */
#define ZVMOPBRANCH 0x03
#define ZVMOPJMP    0x01 // unconditional jump to given address
#define ZVMOPBZ     0x02 // branch if zero
#define ZVMOPBNZ    0x03 // branch if not zero
#define ZVMOPBLT    0x04 // branch if less than
#define ZVMOPBLE    0x05 // branch if less than or equal to
#define ZVMOPBGT    0x06 // branch if greater than
#define ZVMOPBGE    0x07 // branch if greater than or equal to
#define ZVMOPBO     0x08 // branch if overflow
#define ZVMOPBNO    0x09 // branch if no overflow
#define ZVMOPBC     0x0a // branch if carry
#define ZVMOPBNC    0x0b // branch if no carry
/* stack operations */
#define ZVMOPSTACK  0x04
#define ZVMOPPOP    0x01 // pop from stack
#define ZVMOPPUSH   0x02 // push to stack
#define ZVMOPPUSHA  0x03 // push all registers to stack
/* load-store */
#define ZVMOPMOV    0x05
#define ZVMOPMOVL   0x01 // load/store 32-bit longword
#define ZVMOPMOVB   0x02 // load/store 8-bit byte
#define ZVMOPMOVW   0x03 // load/store 16-bit word
#define ZVMOPMOVQ   0x03 // load/store 64-bit quadword
/* function calls */
#define ZVMOPFUNC   0x06
#define ZVMOPCALL   0x01 // call subroutine
#define ZVMOPENTER  0x02 // subroutine prologue
#define ZVMOPLEAVE  0x03 // subroutine epilogue
#define ZVMOPRET    0x04 // return from subroutine
#define ZVMOPTHR    0x05 // launch a new thread
/* machine status word manipulation */
#define ZVMOPMSW    0x07
#define ZVMOPLMSW   0x01 // load machine status word
#define ZVMOPSMSW   0x02 // store machine status word
/* machine state */
#define ZVMOPMACH   0x08
#define ZVMOPRESET  0x01 // reset into well-known state
#define ZVMOPHLT    0x02 // halt execution
/* I/O */
#define ZVMOPIO     0x09
#define ZVMINB      0x01
#define ZVMINW      0x02
#define ZVMINL      0x03
#if (!ZAS32BIT)
#define ZVMINQ      0x04
#endif
#define ZVMOUTB     0x05
#define ZVMOUTW     0x06
#define ZVMOUTL     0x07
#if (!ZAS32BIT)
#define ZVMOUTQ     0x08
#endif

#define zvmgetarg(op, arg1t, ptr)                                       \
    (ptr = &zvm.regs[(op)->reg1],                                       \
     (arg1t) == ZVMARGREG                                               \
     ? zvm.regs[(op)->reg1]                                             \
     : (op)->args[0])
#define zvmgetarg1(op, arg1t)                                           \
    ((arg1t) == ZVMARGREG                                               \
     ? zvm.regs[(op)->reg1]                                             \
     : (op)->args[0])
#define zvmgetarg2(op, arg1t, arg2t, ptr)                               \
    (ptr = &zvm.regs[(op)->reg2],                                       \
     ((arg2t) == ZVMARGREG)                                             \
     ? zvm.regs[op->reg2]                                               \
     : (((arg1t) == ZVMARGREG)                                          \
        ? (op)->args[0]                                                 \
        : (op)->args[1]))
#define zvmgetarg1mov(op, arg1t, arg2t)                                 \
    (((arg1t) == ZVMARGREG)                                             \
     ? (((op)->reg1 & ZASREGINDEX)                                      \
        ? zvmgetmemt(zvm.regs[(op)->reg1 & ZASREGMASK] + op->args[0],   \
                     zasword_t)                                         \
        : (((op)->reg1 & ZASREGINDIR)                                   \
           ? zvmgetmemt(zvm.regs[(op)->reg1 & ZASREGMASK], zasword_t)   \
           : zvm.regs[(op)->reg1 & ZASREGMASK]))                        \
     : (((arg1t) == ZVMARGADR)                                          \
        ? *(zasword_t *)zvmadrtoptr((op)->args[0])                      \
        : (op)->args[0]))
#define zvmgetarg2mov(op, arg1t, arg2t, ptr, t)                         \
    (((arg1t) == ZVMARGREG)                                             \
     ? (((arg2t) == ZVMARGREG)                                          \
        ? (((op)->reg2 & ZASREGINDEX)                                   \
           ? (ptr = (t *)zvmadrtoptr(zvm.regs[(op)->reg2 & ZASREGMASK] + (op)->args[0]), \
              *(ptr))                                                   \
           : (((op)->reg2 & ZASREGINDIR)                                \
              ? (ptr = (t *)zvmadrtoptr(zvm.regs[(op)->reg2 & ZASREGMASK]), \
                 *(ptr))                                                \
              : (ptr = (t *)&zvm.regs[(op)->reg2],                      \
                 *(ptr))))                                              \
        : (ptr = (t *)zvmadrtoptr((op)->args[0]),                       \
           *(ptr)))                                                     \
     : (((arg2t) == ZVMARGREG)                                          \
        ? (((op)->reg2 & ZASREGINDEX)                                   \
           ? (ptr = (t *)zvmadrtoptr(zvm.regs[(op)->reg2 & ZASREGMASK] + (op)->args[1]), \
              *(ptr))                                                   \
           : (((op)->reg2 & ZASREGINDIR)                                \
              ? (ptr = (t *)zvmadrtoptr(zvm.regs[(op)->reg2 & ZASREGMASK]), \
                 *(ptr))                                                \
              : (ptr = (t *)&zvm.regs[(op)->reg2],                      \
                 *(ptr))))                                              \
        : (ptr = (t *)zvmadrtoptr((op)->args[1]),                       \
           *(ptr))))
#if 0
#define zvmgetarg2mov(op, arg1t, arg2t, ptr, t)                         \
    ((((arg1t) == ZVMARGREG)                                            \
      ? ((((arg2t) == ZVMARGREG)                                        \
          ? (ptr = (t *)&zvm.regs[(op)->reg2],                          \
             *(ptr))                                                    \
          : (((arg2t == ZVMARGADR)                                      \
              ? (ptr = (t *)&(op)->args[0],                             \
                 *(ptr))                                                \
              : (ptr = (t *)&zvm.regs[(op)->args[0]],                   \
                 *(ptr))))))                                            \
      : (((arg2t == ZVMARGREG)                                          \
          ? (ptr = (t *)&zvm.regs[(op)->reg2],                          \
             *(ptr))                                                    \
          : (((arg2t == ZVMARGADR)                                      \
              ? (ptr = (t *)&(op)->args[1],                             \
                 *(ptr))                                                \
              : (ptr = (t *)&zvm.regs[(op)->args[1]],                   \
                 *(ptr))))))))
#endif
#define zvmsetzf(val)                                                   \
    (!(val) ? (zvm.msw |= ZVMZF) : (zvm.msw &= ~ZVMZF))

#endif /* __ZVM_OP_H__ */

