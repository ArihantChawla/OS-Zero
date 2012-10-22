#ifndef __PCALC_OP_H__
#define __PCALC_OP_H__

#include <stdint.h>
#include <zpc/zpc.h>

typedef int64_t zpccop_t(struct zpctoken *, struct zpctoken *);
typedef double  zpcfop_t(double, double);

/* cop.c */
int64_t not64(struct zpctoken *arg1, struct zpctoken *dummy);
int64_t and64(struct zpctoken *arg1, struct zpctoken *arg2);
int64_t or64(struct zpctoken *arg1, struct zpctoken *arg2);
int64_t xor64(struct zpctoken *arg1, struct zpctoken *arg2);
int64_t shl64(struct zpctoken *arg1, struct zpctoken *arg2);
int64_t shra64(struct zpctoken *arg1, struct zpctoken *arg2);
int64_t shr64(struct zpctoken *arg1, struct zpctoken *arg2);
int64_t ror64(struct zpctoken *arg1, struct zpctoken *arg2);
int64_t rol64(struct zpctoken *arg1, struct zpctoken *arg2);
int64_t inc64(struct zpctoken *arg1, struct zpctoken *arg2);
int64_t dec64(struct zpctoken *arg1, struct zpctoken *arg2);
int64_t add64(struct zpctoken *arg1, struct zpctoken *arg2);
int64_t sub64(struct zpctoken *arg1, struct zpctoken *arg2);
int64_t mul64(struct zpctoken *arg1, struct zpctoken *arg2);
int64_t div64(struct zpctoken *arg1, struct zpctoken *arg2);
int64_t mod64(struct zpctoken *arg1, struct zpctoken *arg2);

double fadd64(double arg1, double arg2);
double fsub64(double arg1, double arg2);
double fmul64(double arg1, double arg2);
double fdiv64(double arg1, double arg2);

typedef int64_t zpczerofunc_t(int64_t, int64_t, int64_t);

/* zero.c */
int64_t abs64(int64_t arg1, int64_t dummy1, int64_t dummy2);
int64_t min64(int64_t arg1, int64_t arg2, int64_t dummy);
int64_t max64(int64_t arg1, int64_t arg2, int64_t dummy);
int64_t bitset64(int64_t arg1, int64_t arg2, int64_t dummy);
int64_t setbit64(int64_t arg1, int64_t arg2, int64_t dummy);
int64_t clrbit64(int64_t arg1, int64_t arg2, int64_t dummy);
int64_t setbits64(int64_t arg1, int64_t arg2, int64_t arg3);
int64_t clrbits64(int64_t arg1, int64_t arg2, int64_t arg3);
int64_t mergebits64(int64_t arg1, int64_t arg2, int64_t arg3);
int64_t copybits64(int64_t arg1, int64_t arg2, int64_t arg3);
int64_t is2pow64(int64_t arg1, int64_t dummy1, int64_t dummy2);
int64_t rnd2up64(int64_t arg1, int64_t arg2, int64_t dummy);
int64_t rnd2down64(int64_t arg1, int64_t arg2, int64_t dummy);
int64_t trailz64(int64_t arg1, int64_t dummy1, int64_t dummy2);
int64_t leadz64(int64_t arg1, int64_t dummy1, int64_t dummy2);
int64_t ceil2pow64(int64_t arg1, int64_t dummy1, int64_t dummy2);
int64_t isleapyear64(int64_t arg1, int64_t dummy1, int64_t dummy2);

typedef void zpcstkfunc_t(void);

#endif /* __PCALC_OP_H__ */

