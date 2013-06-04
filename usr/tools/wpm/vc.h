#ifndef __WPM_VC_H__
#define __WPM_VC_H__

#include <wpm/conf.h>

#if (WPMVEC)

#include <math.h>
#include <zero/trix.h>
#include <vcode/vc.h>
#include <wpm/vc.h>

void opvadd(struct wpmopcode *op);
void opvsub(struct wpmopcode *op);
void opvmul(struct wpmopcode *op);
void opvdiv(struct wpmopcode *op);
void opvmod(struct wpmopcode *op);
void opvlt(struct wpmopcode *op);
void opvlte(struct wpmopcode *op);
void opvgt(struct wpmopcode *op);
void opvgte(struct wpmopcode *op);
void opveq(struct wpmopcode *op);
void opvineq(struct wpmopcode *op);
void opvshl(struct wpmopcode *op);
void opvshr(struct wpmopcode *op);
void opvnot(struct wpmopcode *op);
void opvand(struct wpmopcode *op);
void opvor(struct wpmopcode *op);
void opvxor(struct wpmopcode *op);
void opvselect(struct wpmopcode *op);
void opvrand(struct wpmopcode *op);
void opvfloor(struct wpmopcode *op);
void opvceil(struct wpmopcode *op);
void opvtrunc(struct wpmopcode *op);
void opvround(struct wpmopcode *op);
void opvitof(struct wpmopcode *op);
void opvitob(struct wpmopcode *op);
void opvbtoi(struct wpmopcode *op);
void opvlog(struct wpmopcode *op);
void opvsqrt(struct wpmopcode *op);
void opvexp(struct wpmopcode *op);
void opvsin(struct wpmopcode *op);
void opvcos(struct wpmopcode *op);
void opvtan(struct wpmopcode *op);
void opvasin(struct wpmopcode *op);
void opvacos(struct wpmopcode *op);
void opvatan(struct wpmopcode *op);
void opvsinh(struct wpmopcode *op);
void opvcosh(struct wpmopcode *op);
void opvtanh(struct wpmopcode *op);
void opvplscan(struct wpmopcode *op);
void opvmulscan(struct wpmopcode *op);
void opvmaxscan(struct wpmopcode *op);
void opvminscan(struct wpmopcode *op);
void opvandscan(struct wpmopcode *op);
void opvorscan(struct wpmopcode *op);
void opvxorscan(struct wpmopcode *op);
void opvplreduce(struct wpmopcode *op);
void opvmulreduce(struct wpmopcode *op);
void opvmaxreduce(struct wpmopcode *op);
void opvminreduce(struct wpmopcode *op);
void opvandreduce(struct wpmopcode *op);
void opvorreduce(struct wpmopcode *op);
void opvxorreduce(struct wpmopcode *op);
void opvpermute(struct wpmopcode *op);
void opvdpermute(struct wpmopcode *op);
void opvfpermute(struct wpmopcode *op);
void opvbpermute(struct wpmopcode *op);
void opvbfpermute(struct wpmopcode *op);
void opvdfpermute(struct wpmopcode *op);
void opvextract(struct wpmopcode *op);
void opvreplace(struct wpmopcode *op);
void opvpack(struct wpmopcode *op);
void opvrankup(struct wpmopcode *op);
void opvrankdown(struct wpmopcode *op);
void opvdist(struct wpmopcode *op);
void opvindex(struct wpmopcode *op);
void opvlength(struct wpmopcode *op);
void opvmkdes(struct wpmopcode *op);
void opvlengths(struct wpmopcode *op);
void opvcopy(struct wpmopcode *op);
void opvpop(struct wpmopcode *op);
void opvcpop(struct wpmopcode *op);
void opvpair(struct wpmopcode *op);
void opvunpair(struct wpmopcode *op);

typedef void vcophandler_t(void *, void *, size_t);

#define vcadd(v1, v2)       ((v1) + (v2))
#define vcsub(v1, v2)       ((v1) - (v2))
#define vcmul(v1, v2)       ((v1) * (v2))
#define vcdiv(v1, v2)       ((v1) / (v2))
#define vcmod(v1, v2)       ((v1) % (v2))
#define vclt(v1, v2)        ((v1) < (v2) ? VC_TRUE : VC_FALSE)
#define vclte(v1, v2)       ((v1) <= (v2) ? VC_TRUE : VC_FALSE)
#define vcgt(v1, v2)        ((v1) > (v2) ? VC_TRUE : VC_FALSE)
#define vcgte(v1, v2)       ((v1) >= (v2) ? VC_TRUE : VC_FALSE)
#define vceq(v1, v2)        ((v1) == (v2) ? VC_TRUE : VC_FALSE)
#define vcineq(v1, v2)      ((v1) != (v2) ? VC_TRUE : VC_FALSE)
#define vcshl(v1, v2)       ((v1) << (v2))
#define vcshr(v1, v2)       ((v1) >> (v2))
#define vcnot(v1)           (~(v1))
#define vcand(v1, v2)       ((v1) & (v2))
#define vcor(v1, v2)        ((v1) | (v2))
#define vcxor(v1, v2)       ((v1) ^ (v2))
#define vcselect(b, v1, v2) ((b) ? (v1) : (v2))
#define vcrand(v1)          (rand() % (v1))
#define vcfloor(v1)         (floor(v1))
#define vcceil(v1)          (ceil(v1))
#define vctrunc(v1)         (trunc(v1))
#define vcround(v1)         (round(v1))
#define vcitof(v1)          ((vcfloat)(v1))
#define vcitob(v1)          ((v1) ? VC_TRUE : VC_FALSE)
#define vcbtoi(v1)          ((v1) ? 1 : 0)
#define vclog(v1)           (log(v1))
#define vcsqrt(v1)          (sqrt(v1))
#define vcexp(v1)           (exp(v1))
#define vcsin(v1)           (sin(v1))
#define vccos(v1)           (cos(v1))
#define vctan(v1)           (tan(v1))
#define vcasin(v1)          (asin(v1))
#define vcacos(v1)          (acos(v1))
#define vcatan(v1)          (atan(v1))
#define vcsinh(v1)          (sinh(v1))
#define vccosh(v1)          (cosh(v1))
#define vctanh(v1)          (tanh(v1))

#define vcintop1(adr1, len1, OP)                                        \
    do {                                                                \
        wpmmemadr_t ptr1 = adr1;                                        \
        vcint       val1;                                               \
        vcint       res;                                                \
                                                                        \
        while (len1--) {                                                \
            val1 = memfetchq(ptr1);                                     \
            res = OP(val1);                                             \
            memstoreq(res, ptr1);                                       \
            ptr1 += 8;                                                  \
        }                                                               \
    } while (0);

#define vcintop2(adr1, adr2, len1, len2, OP)                            \
    do {                                                                \
        wpmmemadr_t ptr1 = adr1;                                        \
        wpmmemadr_t ptr2 = adr2;                                        \
        vcint       val1;                                               \
        vcint       val2;                                               \
        vcint       res;                                                \
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
                fprintf(stderr, "%llx -> %llx\n", res, ptr2);           \
                memstoreq(res, ptr2);                                   \
                ptr1 += 8;                                              \
                ptr2 += 8;                                              \
            }                                                           \
        }                                                               \
    } while (0)

#define vcfloatop1(adr1, len1, OP)                                      \
    do {                                                                \
        wpmmemadr_t ptr1 = adr1;                                        \
        vcfloat     val1;                                               \
        vcfloat     res;                                                \
                                                                        \
        while (len1--) {                                                \
            val1 = memfetchq(ptr1);                                     \
            res = OP(val1);                                             \
            memstoreq(res, ptr1);                                       \
            ptr1 += 8;                                                  \
        }                                                               \
    } while (0);

#define vcfloatop2(adr1, adr2, len1, len2, OP)                          \
    do {                                                                \
        wpmmemadr_t ptr1 = adr1;                                        \
        wpmmemadr_t ptr2 = adr2;                                        \
        vcfloat     val1;                                               \
        vcfloat     val2;                                               \
        vcfloat     res;                                                \
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

#define vcintcmp(adr1, adr2, len1, len2, OP)                            \
    do {                                                                \
        wpmmemadr_t ptr1 = adr1;                                        \
        wpmmemadr_t ptr2 = adr2;                                        \
        vcint       val1;                                               \
        vcint       val2;                                               \
        vcint       res;                                                \
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

#define vcfloatcmp(adr1, adr2, len1, len2, OP)                          \
        do {                                                            \
        wpmmemadr_t ptr1 = adr1;                                        \
        wpmmemadr_t ptr2 = adr2;                                        \
        vcfloat     val1;                                               \
        vcfloat     val2;                                               \
        vcfloat     res;                                                \
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

#endif /* WPMVEC */

#endif /* __VPM_V_H__ */

