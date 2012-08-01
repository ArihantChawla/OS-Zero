#ifndef __PCALC_OP_H__
#define __PCALC_OP_H__

typedef void pcalcop_t(void *, void *, void *);

/* cop.c */
void copnot8(void *arg1, void *dummy, void *ret);
void copnot16(void *arg1, void *dummy, void *ret);
void copnot32(void *arg1, void *dummy, void *ret);
void copnot64(void *arg1, void *dummy, void *ret);
void copand8(void *arg1, void *arg2, void *ret);
void copand16(void *arg1, void *arg2, void *ret);
void copand32(void *arg1, void *arg2, void *ret);
void copand64(void *arg1, void *arg2, void *ret);
void copor8(void *arg1, void *arg2, void *ret);
void copor16(void *arg1, void *arg2, void *ret);
void copor32(void *arg1, void *arg2, void *ret);
void copor64(void *arg1, void *arg2, void *ret);
void copxor8(void *arg1, void *arg2, void *ret);
void copxor16(void *arg1, void *arg2, void *ret);
void copxor32(void *arg1, void *arg2, void *ret);
void copxor64(void *arg1, void *arg2, void *ret);
void copshl8(void *arg1, void *arg2, void *ret);
void copshl16(void *arg1, void *arg2, void *ret);
void copshl32(void *arg1, void *arg2, void *ret);
void copshl64(void *arg1, void *arg2, void *ret);
void copshr8(void *arg1, void *arg2, void *ret);
void copshr16(void *arg1, void *arg2, void *ret);
void copshr32(void *arg1, void *arg2, void *ret);
void copshr64(void *arg1, void *arg2, void *ret);
void copshrl8(void *arg1, void *arg2, void *ret);
void copshrl16(void *arg1, void *arg2, void *ret);
void copshrl32(void *arg1, void *arg2, void *ret);
void copshrl64(void *arg1, void *arg2, void *ret);
void copror8(void *arg1, void *arg2, void *ret);
void copror16(void *arg1, void *arg2, void *ret);
void copror32(void *arg1, void *arg2, void *ret);
void copror64(void *arg1, void *arg2, void *ret);
void coprol8(void *arg1, void *arg2, void *ret);
void coprol16(void *arg1, void *arg2, void *ret);
void coprol32(void *arg1, void *arg2, void *ret);
void coprol64(void *arg1, void *arg2, void *ret);
void copinc8(void *arg1, void *arg2, void *ret);
void copinc16(void *arg1, void *arg2, void *ret);
void copinc32(void *arg1, void *arg2, void *ret);
void copinc64(void *arg1, void *arg2, void *ret);
void copdec8(void *arg1, void *arg2, void *ret);
void copdec16(void *arg1, void *arg2, void *ret);
void copdec32(void *arg1, void *arg2, void *ret);
void copdec64(void *arg1, void *arg2, void *ret);
void copadd8(void *arg1, void *arg2, void *ret);
void copadd16(void *arg1, void *arg2, void *ret);
void copadd32(void *arg1, void *arg2, void *ret);
void copadd64(void *arg1, void *arg2, void *ret);
void copsub8(void *arg1, void *arg2, void *ret);
void copsub16(void *arg1, void *arg2, void *ret);
void copsub32(void *arg1, void *arg2, void *ret);
void copsub64(void *arg1, void *arg2, void *ret);
void copmul8(void *arg1, void *arg2, void *ret);
void copmul16(void *arg1, void *arg2, void *ret);
void copmul32(void *arg1, void *arg2, void *ret);
void copmul64(void *arg1, void *arg2, void *ret);
void copdiv8(void *arg1, void *arg2, void *ret);
void copdiv16(void *arg1, void *arg2, void *ret);
void copdiv32(void *arg1, void *arg2, void *ret);
void copdiv64(void *arg1, void *arg2, void *ret);
void copmod8(void *arg1, void *arg2, void *ret);
void copmod16(void *arg1, void *arg2, void *ret);
void copmod32(void *arg1, void *arg2, void *ret);
void copmod64(void *arg1, void *arg2, void *ret);

#endif /* __PCALC_OP_H__ */

