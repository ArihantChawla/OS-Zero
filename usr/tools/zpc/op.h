#ifndef __PCALC_OP_H__
#define __PCALC_OP_H__

typedef int64_t zpccop_t(int64_t, int64_t);
typedef double  zpcfop_t(double, double);

/* cop.c */
int64_t not64(int64_t arg1, int64_t dummy);
int64_t and64(int64_t arg1, int64_t arg2);
int64_t or64(int64_t arg1, int64_t arg2);
int64_t xor64(int64_t arg1, int64_t arg2);
int64_t shl64(int64_t arg1, int64_t arg2);
int64_t shr64(int64_t arg1, int64_t arg2);
int64_t shrl64(int64_t arg1, int64_t arg2);
int64_t ror64(int64_t arg1, int64_t arg2);
int64_t rol64(int64_t arg1, int64_t arg2);
int64_t inc64(int64_t arg1, int64_t arg2);
int64_t dec64(int64_t arg1, int64_t arg2);
int64_t add64(int64_t arg1, int64_t arg2);
int64_t sub64(int64_t arg1, int64_t arg2);
int64_t mul64(int64_t arg1, int64_t arg2);
int64_t div64(int64_t arg1, int64_t arg2);
int64_t mod64(int64_t arg1, int64_t arg2);

double fadd64(double arg1, double arg2);
double fsub64(double arg1, double arg2);
double fmul64(double arg1, double arg2);
double fdiv64(double arg1, double arg2);

typedef void zpczerofunc_t(void *, void *, void *, void *);

/* zero.c */
void abs32(void *arg1, void *dummy1, void *dummy2, void *ret);
void abs64(void *arg1, void *dummy1, void *dummy2, void *ret);
void avgu32(void *arg1, void *arg2, void *dummy, void *ret);
void avgu64(void *arg1, void *arg2, void *dummy, void *ret);
void min32(void *arg1, void *arg2, void *dummy, void *ret);
void min64(void *arg1, void *arg2, void *dummy, void *ret);
void max32(void *arg1, void *arg2, void *dummy, void *ret);
void max64(void *arg1, void *arg2, void *dummy, void *ret);
void bitset32(void *arg1, void *arg2, void *dummy, void *ret);
void bitset64(void *arg1, void *arg2, void *dummy, void *ret);
void setbit32(void *arg1, void *arg2, void *dummy, void *ret);
void setbit64(void *arg1, void *arg2, void *dummy, void *ret);
void clrbit32(void *arg1, void *arg2, void *dummy, void *ret);
void clrbit64(void *arg1, void *arg2, void *dummy, void *ret);
void setbits32(void *arg1, void *arg2, void *arg3, void *ret);
void setbits64(void *arg1, void *arg2, void *arg3, void *ret);
void clrbits32(void *arg1, void *arg2, void *arg3, void *ret);
void clrbits64(void *arg1, void *arg2, void *arg3, void *ret);
void mergebits32(void *arg1, void *arg2, void *arg3, void *ret);
void mergebits64(void *arg1, void *arg2, void *arg3, void *ret);
void copybits32(void *arg1, void *arg2, void *arg3, void *ret);
void copybits64(void *arg1, void *arg2, void *arg3, void *ret);
void is2pow32(void *arg1, void *dummy1, void *dummy2, void *ret);
void is2pow64(void *arg1, void *dummy1, void *dummy2, void *ret);
void rnd2up32(void *arg1, void *arg2, void *dummy, void *ret);
void rnd2up64(void *arg1, void *arg2, void *dummy, void *ret);
void rnd2down32(void *arg1, void *arg2, void *dummy, void *ret);
void rnd2down64(void *arg1, void *arg2, void *dummy, void *ret);
void trailz32(void *arg1, void *dummy1, void *dummy2, void *ret);
void trailz64(void *arg1, void *dummy1, void *dummy2, void *ret);
void leadz32(void *arg1, void *dummy1, void *dummy2, void *ret);
void leadz64(void *arg1, void *dummy1, void *dummy2, void *ret);
void ceil2pow32(void *arg1, void *dummy1, void *dummy2, void *ret);
void ceil2pow64(void *arg1, void *dummy1, void *dummy2, void *ret);
void isleapyear32(void *arg1, void *dummy1, void *dummy2, void *ret);
void isleapyear64(void *arg1, void *dummy1, void *dummy2, void *ret);

typedef void zpcstkfunc_t(void);

#endif /* __PCALC_OP_H__ */

