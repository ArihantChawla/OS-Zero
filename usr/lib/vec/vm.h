#ifndef __VEC_VM_H__
#define __VEC_VM_H__

#include <wpm/conf.h>

#if (WPMVEC)

#include <math.h>
#include <zero/trix.h>
#include <vec/vec.h>

void vecopadd(struct wpmopcode *op);
void vecopsub(struct wpmopcode *op);
void vecopmul(struct wpmopcode *op);
void vecopdiv(struct wpmopcode *op);
void vecopmod(struct wpmopcode *op);
void vecoplt(struct wpmopcode *op);
void vecoplte(struct wpmopcode *op);
void vecopgt(struct wpmopcode *op);
void vecopgte(struct wpmopcode *op);
void vecopeq(struct wpmopcode *op);
void vecopineq(struct wpmopcode *op);
void vecopshl(struct wpmopcode *op);
void vecopshr(struct wpmopcode *op);
void vecopnot(struct wpmopcode *op);
void vecopand(struct wpmopcode *op);
void vecopor(struct wpmopcode *op);
void vecopxor(struct wpmopcode *op);
void vecopselect(struct wpmopcode *op);
void vecoprand(struct wpmopcode *op);
void vecopfloor(struct wpmopcode *op);
void vecopceil(struct wpmopcode *op);
void vecoptrunc(struct wpmopcode *op);
void vecopround(struct wpmopcode *op);
void vecopitof(struct wpmopcode *op);
void vecopitob(struct wpmopcode *op);
void vecopbtoi(struct wpmopcode *op);
void vecoplog(struct wpmopcode *op);
void vecopsqrt(struct wpmopcode *op);
void vecopexp(struct wpmopcode *op);
void vecopsin(struct wpmopcode *op);
void vecopcos(struct wpmopcode *op);
void vecoptan(struct wpmopcode *op);
void vecopasin(struct wpmopcode *op);
void vecopacos(struct wpmopcode *op);
void vecopatan(struct wpmopcode *op);
void vecopsinh(struct wpmopcode *op);
void vecopcosh(struct wpmopcode *op);
void vecoptanh(struct wpmopcode *op);
void vecopplscan(struct wpmopcode *op);
void vecopmulscan(struct wpmopcode *op);
void vecopmaxscan(struct wpmopcode *op);
void vecopminscan(struct wpmopcode *op);
void vecopandscan(struct wpmopcode *op);
void vecoporscan(struct wpmopcode *op);
void vecopxorscan(struct wpmopcode *op);
void vecopplreduce(struct wpmopcode *op);
void vecopmulreduce(struct wpmopcode *op);
void vecopmaxreduce(struct wpmopcode *op);
void vecopminreduce(struct wpmopcode *op);
void vecopandreduce(struct wpmopcode *op);
void vecoporreduce(struct wpmopcode *op);
void vecopxorreduce(struct wpmopcode *op);
void vecoppermute(struct wpmopcode *op);
void vecopdpermute(struct wpmopcode *op);
void vecopfpermute(struct wpmopcode *op);
void vecopbpermute(struct wpmopcode *op);
void vecopbfpermute(struct wpmopcode *op);
void vecopdfpermute(struct wpmopcode *op);
void vecopextract(struct wpmopcode *op);
void vecopreplace(struct wpmopcode *op);
void vecoppack(struct wpmopcode *op);
void vecoprankup(struct wpmopcode *op);
void vecoprankdown(struct wpmopcode *op);
void vecopdist(struct wpmopcode *op);
void vecopindex(struct wpmopcode *op);
void vecoplength(struct wpmopcode *op);
void vecopmkdes(struct wpmopcode *op);
void vecoplengths(struct wpmopcode *op);
void vecopcopy(struct wpmopcode *op);
void vecoppop(struct wpmopcode *op);
void vecopcpop(struct wpmopcode *op);
void vecoppair(struct wpmopcode *op);
void vecopunpair(struct wpmopcode *op);

typedef void vecophandler_t(void *, void *, size_t);

#define vecadd(v1, v2)       ((v1) + (v2))
#define vecsub(v1, v2)       ((v1) - (v2))
#define vecmul(v1, v2)       ((v1) * (v2))
#define vecdiv(v1, v2)       ((v1) / (v2))
#define vecmod(v1, v2)       ((v1) % (v2))
#define veclt(v1, v2)        ((v1) < (v2) ? VEC_TRUE : VEC_FALSE)
#define veclte(v1, v2)       ((v1) <= (v2) ? VEC_TRUE : VEC_FALSE)
#define vecgt(v1, v2)        ((v1) > (v2) ? VEC_TRUE : VEC_FALSE)
#define vecgte(v1, v2)       ((v1) >= (v2) ? VEC_TRUE : VEC_FALSE)
#define veceq(v1, v2)        ((v1) == (v2) ? VEC_TRUE : VEC_FALSE)
#define vecineq(v1, v2)      ((v1) != (v2) ? VEC_TRUE : VEC_FALSE)
#define vecshl(v1, v2)       ((v1) << (v2))
#define vecshr(v1, v2)       ((v1) >> (v2))
#define vecnot(v1)           (~(v1))
#define vecand(v1, v2)       ((v1) & (v2))
#define vecor(v1, v2)        ((v1) | (v2))
#define vecxor(v1, v2)       ((v1) ^ (v2))
#define vecselect(b, v1, v2) ((b) ? (v1) : (v2))
#define vecrand(v1)          (rand() % (v1))
#define vecfloor(v1)         (floor(v1))
#define vecceil(v1)          (ceil(v1))
#define vectrunc(v1)         (trunc(v1))
#define vecround(v1)         (round(v1))
#define vecitof(v1)          ((vecfloat)(v1))
#define vecitob(v1)          ((v1) ? VEC_TRUE : VEC_FALSE)
#define vecbtoi(v1)          ((v1) ? 1 : 0)
#define veclog(v1)           (log(v1))
#define vecsqrt(v1)          (sqrt(v1))
#define vecexp(v1)           (exp(v1))
#define vecsin(v1)           (sin(v1))
#define veccos(v1)           (cos(v1))
#define vectan(v1)           (tan(v1))
#define vecasin(v1)          (asin(v1))
#define vecacos(v1)          (acos(v1))
#define vecatan(v1)          (atan(v1))
#define vecsinh(v1)          (sinh(v1))
#define veccosh(v1)          (cosh(v1))
#define vectanh(v1)          (tanh(v1))

#define vecintop1b(adr1, len1, OP)                                      \
    do {                                                                \
        wpmmemadr_t ptr1 = adr1;                                        \
        int8_t      val1;                                               \
        int8_t      res;                                                \
                                                                        \
        while (len1--) {                                                \
            val1 = memfetchq(ptr1);                                     \
            res = OP(val1);                                             \
            memstoreq(res, ptr1);                                       \
            ptr1++;                                                     \
        }                                                               \
    } while (0);

#define vecintop1w(adr1, len1, OP)                                      \
    do {                                                                \
        wpmmemadr_t ptr1 = adr1;                                        \
        int16_t     val1;                                               \
        int16_t     res;                                                \
                                                                        \
        while (len1--) {                                                \
            val1 = memfetchq(ptr1);                                     \
            res = OP(val1);                                             \
            memstoreq(res, ptr1);                                       \
            ptr1 += 2;                                                  \
        }                                                               \
    } while (0);

#define vecintop1l(adr1, len1, OP)                                      \
    do {                                                                \
        wpmmemadr_t ptr1 = adr1;                                        \
        int32_t     val1;                                               \
        int32_t     res;                                                \
                                                                        \
        while (len1--) {                                                \
            val1 = memfetchq(ptr1);                                     \
            res = OP(val1);                                             \
            memstoreq(res, ptr1);                                       \
            ptr1 += 4;                                                  \
        }                                                               \
    } while (0);

#define vecintop1q(adr1, len1, OP)                                      \
    do {                                                                \
        wpmmemadr_t ptr1 = adr1;                                        \
        int64_t     val1;                                               \
        int64_t     res;                                                \
                                                                        \
        while (len1--) {                                                \
            val1 = memfetchq(ptr1);                                     \
            res = OP(val1);                                             \
            memstoreq(res, ptr1);                                       \
            ptr1 += 8;                                                  \
        }                                                               \
    } while (0);

#define vecintop2b(adr1, adr2, len1, len2, OP, flg)                     \
    do {                                                                \
        wpmmemadr_t ptr1 = adr1;                                        \
        wpmmemadr_t ptr2 = adr2;                                        \
        int16_t     val1;                                               \
        int16_t     val2;                                               \
        int16_t     res;                                                \
                                                                        \
        if (len1 == 1) {                                                \
            val1 = memfetchb(ptr1);                                     \
            if (flg & OP_SATU) {                                        \
                while (len2--) {                                        \
                    val2 = memfetchb(ptr2);                             \
                    res = OP(val1, val2);                               \
                    res = max(res, 0xff);                               \
                    memstoreb(res, ptr2);                               \
                    ptr2++;                                             \
                }                                                       \
            } else if (flg & OP_SATS) {                                 \
                while (len2--) {                                        \
                    val2 = memfetchb(ptr2);                             \
                    res = OP(val1, val2);                               \
                    res = max(res, 0);                                  \
                    res = max(res, 0x7f);                               \
                    memstoreb(res, ptr2);                               \
                    ptr2++;                                             \
                }                                                       \
            } else {                                                    \
                while (len2--) {                                        \
                    val2 = memfetchb(ptr2);                             \
                    res = OP(val1, val2);                               \
                    memstoreb(res, ptr2);                               \
                    ptr2++;                                             \
                }                                                       \
            }                                                           \
        } else {                                                        \
            len1 = min(len1, len2);                                     \
            if (flg & OP_SATU) {                                        \
                while (len1--) {                                        \
                    val1 = memfetchb(ptr1);                             \
                    val2 = memfetchb(ptr2);                             \
                    res = OP(val1, val2);                               \
                    res = max(res, 0xff);                               \
                    memstoreb(res, ptr2);                               \
                    ptr1++;                                             \
                    ptr2++;                                             \
                }                                                       \
            } else if (flg & OP_SATS) {                                 \
                while (len1--) {                                        \
                    val1 = memfetchb(ptr1);                             \
                    val2 = memfetchb(ptr2);                             \
                    res = OP(val1, val2);                               \
                    res = max(res, 0);                                  \
                    res = max(res, 0x7f);                               \
                    memstoreb(res, ptr2);                               \
                    ptr1++;                                             \
                    ptr2++;                                             \
                }                                                       \
            } else {                                                    \
                while (len1--) {                                        \
                    val1 = memfetchb(ptr1);                             \
                    val2 = memfetchb(ptr2);                             \
                    res = OP(val1, val2);                               \
                    memstoreb(res, ptr2);                               \
                    ptr1++;                                             \
                    ptr2++;                                             \
                }                                                       \
            }                                                           \
        }                                                               \
    } while (0)

#define vecintop2w(adr1, adr2, len1, len2, OP, flg)                     \
    do {                                                                \
        wpmmemadr_t ptr1 = adr1;                                        \
        wpmmemadr_t ptr2 = adr2;                                        \
        int16_t     val1;                                               \
        int16_t     val2;                                               \
        int16_t     res;                                                \
                                                                        \
        if (len1 == 1) {                                                \
            val1 = memfetchw(ptr1);                                     \
            while (len2--) {                                            \
                val2 = memfetchw(ptr2);                                 \
                res = OP(val1, val2);                                   \
                memstorew(res, ptr2);                                   \
                ptr2 += 2;                                              \
            }                                                           \
        } else {                                                        \
            len1 = min(len1, len2);                                     \
            while (len1--) {                                            \
                val1 = memfetchw(ptr1);                                 \
                val2 = memfetchw(ptr2);                                 \
                res = OP(val1, val2);                                   \
                memstorew(res, ptr2);                                   \
                ptr1 += 2;                                              \
                ptr2 += 2;                                              \
            }                                                           \
        }                                                               \
    } while (0)

#define vecintop2l(adr1, adr2, len1, len2, OP, flg)                     \
    do {                                                                \
        wpmmemadr_t ptr1 = adr1;                                        \
        wpmmemadr_t ptr2 = adr2;                                        \
        int32_t     val1;                                               \
        int32_t     val2;                                               \
        int32_t     res;                                                \
                                                                        \
        if (len1 == 1) {                                                \
            val1 = memfetchl(ptr1);                                     \
            while (len2--) {                                            \
                val2 = memfetchl(ptr2);                                 \
                res = OP(val1, val2);                                   \
                memstorew(res, ptr2);                                   \
                ptr2 += 4;                                              \
            }                                                           \
        } else {                                                        \
            len1 = min(len1, len2);                                     \
            while (len1--) {                                            \
                val1 = memfetchl(ptr1);                                 \
                val2 = memfetchl(ptr2);                                 \
                res = OP(val1, val2);                                   \
                memstorel(res, ptr2);                                   \
                ptr1 += 4;                                              \
                ptr2 += 4;                                              \
            }                                                           \
        }                                                               \
    } while (0)

#define vecintop2q(adr1, adr2, len1, len2, OP, flg)                     \
    do {                                                                \
        wpmmemadr_t ptr1 = adr1;                                        \
        wpmmemadr_t ptr2 = adr2;                                        \
        int64_t     val1;                                               \
        int64_t     val2;                                               \
        int64_t     res;                                                \
                                                                        \
        if (len1 == 1) {                                                \
            val1 = memfetchq(ptr1);                                     \
            while (len2--) {                                            \
                val2 = memfetchq(ptr2);                                 \
                res = OP(val1, val2);                                   \
                memstoreq(res, ptr2);                                   \
                ptr2 += 8;                                              \
            }                                                           \
        } else {                                                        \
            len1 = min(len1, len2);                                     \
            while (len1--) {                                            \
                val1 = memfetchq(ptr1);                                 \
                val2 = memfetchq(ptr2);                                 \
                res = OP(val1, val2);                                   \
                memstoreq(res, ptr2);                                   \
                ptr1 += 8;                                              \
                ptr2 += 8;                                              \
            }                                                           \
        }                                                               \
    } while (0)

#define vecfloatop1(adr1, len1, OP)                                     \
    do {                                                                \
        wpmmemadr_t ptr1 = adr1;                                        \
        vecfloat    val1;                                               \
        vecfloat    res;                                                \
                                                                        \
        while (len1--) {                                                \
            val1 = memfetchq(ptr1);                                     \
            res = OP(val1);                                             \
            memstoreq(res, ptr1);                                       \
            ptr1 += 8;                                                  \
        }                                                               \
    } while (0);

#define vecfloatop2(adr1, adr2, len1, len2, OP)                         \
    do {                                                                \
        wpmmemadr_t ptr1 = adr1;                                        \
        wpmmemadr_t ptr2 = adr2;                                        \
        vecfloat   val1;                                                \
        vecfloat   val2;                                                \
        vecfloat   res;                                                 \
                                                                        \
        if (len1 == 1) {                                                \
            val1 = memfetchq(ptr1);                                     \
            while (len2--) {                                            \
                val2 = memfetchq(ptr2);                                 \
                res = OP(val1, val2);                                   \
                memstoreq(res, ptr2);                                   \
                ptr2 += 8;                                              \
            }                                                           \
        } else {                                                        \
            len1 = min(len1, len2);                                     \
            while (len1--) {                                            \
                val1 = memfetchq(ptr1);                                 \
                val2 = memfetchq(ptr2);                                 \
                res = OP(val1, val2);                                   \
                memstoreq(res, ptr2);                                   \
                ptr1 += 8;                                              \
                ptr2 += 8;                                              \
            }                                                           \
        }                                                               \
    } while (0)

#define vecintcmp(adr1, adr2, len1, len2, OP)                           \
    do {                                                                \
        wpmmemadr_t ptr1 = adr1;                                        \
        wpmmemadr_t ptr2 = adr2;                                        \
        vecint      val1;                                               \
        vecint      val2;                                               \
        vecint      res;                                                \
                                                                        \
        len1 = min(len1, len2);                                         \
        while (len1--) {                                                \
            val1 = memfetchq(ptr1);                                     \
            val2 = memfetchq(ptr2);                                     \
            res = OP(val1, val2);                                       \
            memstoreq(res, ptr2);                                       \
            ptr1 += 8;                                                  \
            ptr2 += 8;                                                  \
        }                                                               \
    } while (0)

#define vecfloatcmp(adr1, adr2, len1, len2, OP)                         \
        do {                                                            \
        wpmmemadr_t ptr1 = adr1;                                        \
        wpmmemadr_t ptr2 = adr2;                                        \
        vecfloat    val1;                                               \
        vecfloat    val2;                                               \
        vecfloat    res;                                                \
                                                                        \
        len1 = min(len1, len2);                                         \
        while (len1--) {                                                \
            val1 = memfetchq(ptr1);                                     \
            val2 = memfetchq(ptr2);                                     \
            res = OP(val1, val2);                                       \
            memstoreq(res, ptr2);                                       \
            ptr1 += 8;                                                  \
            ptr2 += 8;                                                  \
        }                                                               \
    } while (0)

#define vecvmop1(vop, OP)                                               \
    do {                                                                \
        long                 reg1 = vop->reg1;                          \
        long                 reg2 = vop->reg2;                          \
        wpmmemadr_t          adr1 = wpm->cpustat.varegs[reg1];          \
        wpmmemadr_t          adr2 = wpm->cpustat.varegs[reg2];          \
        uint64_t             len1 = wpm->cpustat.vlregs[reg1];          \
        uint64_t             len2 = wpm->cpustat.vlregs[reg2];          \
                                                                        \
        switch (vecoptype(vop)) {                                       \
            case OP_FLOAT:                                              \
                vecfloatop1(adr1, adr2, len1, len2, OP);                \
                                                                        \
                break;                                                  \
            case OP_BYTE:                                               \
                vecintop1b(adr1, adr2, len1, len2, OP, vecopflg(vop));  \
                                                                        \
                break;                                                  \
            case OP_WORD:                                               \
                vecintop1w(adr1, adr2, len1, len2, OP, vecopflg(vop));  \
                                                                        \
                break;                                                  \
            case OP_LONG:                                               \
                vecintop1l(adr1, adr2, len1, len2, OP, vecopflg(vop));  \
                                                                        \
                break;                                                  \
            case OP_QUAD:                                               \
                vecintop1q(adr1, adr2, len1, len2, OP, vecopflg(vop));  \
                                                                        \
                break;                                                  \
        }                                                               \
    } while (0)

#define vecvmfloatop1(vop, OP)                                          \
    do {                                                                \
        long                 reg1 = vop->reg1;                          \
        wpmmemadr_t          adr1 = wpm->cpustat.varegs[reg1];          \
        uint64_t             len1 = wpm->cpustat.vlregs[reg1];          \
                                                                        \
        vecfloatop1(adr1, len1, OP);                                    \
    } while (0)

#define vecvmintop1(vop, OP)                                            \
    do {                                                                \
        long                 reg1 = vop->reg1;                          \
        wpmmemadr_t          adr1 = wpm->cpustat.varegs[reg1];          \
        uint64_t             len1 = wpm->cpustat.vlregs[reg1];          \
                                                                        \
        switch (vecoptype(vop)) {                                       \
            case OP_BYTE:                                               \
                vecintop1b(adr1, len1, OP);                             \
                                                                        \
                break;                                                  \
            case OP_WORD:                                               \
                vecintop1w(adr1, len1, OP);                             \
                                                                        \
                break;                                                  \
            case OP_LONG:                                               \
                vecintop1l(adr1, len1, OP);                             \
                                                                        \
                break;                                                  \
            case OP_QUAD:                                               \
                vecintop1q(adr1, len1, OP);                             \
                                                                        \
                break;                                                  \
        }                                                               \
    } while (0)

#define vecvmop2(vop, OP)                                               \
    do {                                                                \
        long                 reg1 = vop->reg1;                          \
        long                 reg2 = vop->reg2;                          \
        wpmmemadr_t          adr1 = wpm->cpustat.varegs[reg1];          \
        wpmmemadr_t          adr2 = wpm->cpustat.varegs[reg2];          \
        uint64_t             len1 = wpm->cpustat.vlregs[reg1];          \
        uint64_t             len2 = wpm->cpustat.vlregs[reg2];          \
                                                                        \
        switch (vecoptype(vop)) {                                       \
            case OP_FLOAT:                                              \
                vecfloatop2(adr1, adr2, len1, len2, OP);                \
                                                                        \
                break;                                                  \
            case OP_BYTE:                                               \
                vecintop2b(adr1, adr2, len1, len2, OP, vecopflg(vop));  \
                                                                        \
                break;                                                  \
            case OP_WORD:                                               \
                vecintop2w(adr1, adr2, len1, len2, OP, vecopflg(vop));  \
                                                                        \
                break;                                                  \
            case OP_LONG:                                               \
                vecintop2l(adr1, adr2, len1, len2, OP, vecopflg(vop));  \
                                                                        \
                break;                                                  \
            case OP_QUAD:                                               \
                vecintop2q(adr1, adr2, len1, len2, OP, vecopflg(vop));  \
                                                                        \
                break;                                                  \
        }                                                               \
    } while (0)

#define vecvmintop2(vop, OP)                                            \
    do {                                                                \
        long                 reg1 = vop->reg1;                          \
        long                 reg2 = vop->reg2;                          \
        wpmmemadr_t          adr1 = wpm->cpustat.varegs[reg1];          \
        wpmmemadr_t          adr2 = wpm->cpustat.varegs[reg2];          \
        uint64_t             len1 = wpm->cpustat.vlregs[reg1];          \
        uint64_t             len2 = wpm->cpustat.vlregs[reg2];          \
                                                                        \
        switch (vecoptype(vop)) {                                       \
            case OP_BYTE:                                               \
                vecintop2b(adr1, adr2, len1, len2, OP, vecopflg(vop));  \
                                                                        \
                break;                                                  \
            case OP_WORD:                                               \
                vecintop2w(adr1, adr2, len1, len2, OP, vecopflg(vop));  \
                                                                        \
                break;                                                  \
            case OP_LONG:                                               \
                vecintop2l(adr1, adr2, len1, len2, OP, vecopflg(vop));  \
                                                                        \
                break;                                                  \
            case OP_QUAD:                                               \
                vecintop2q(adr1, adr2, len1, len2, OP, vecopflg(vop));  \
                                                                        \
                break;                                                  \
        }                                                               \
    } while (0)

#endif /* WPMVEC */

#endif /* __VEC_VM_H__ */

