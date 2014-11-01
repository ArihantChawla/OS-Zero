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

#endif /* __ZVM_OP_H__ */

