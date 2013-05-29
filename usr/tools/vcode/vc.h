#ifndef __VCODE_VC_H__
#define __VCODE_VC_H__

#include <stdint.h>

typedef int64_t vcint;
typedef int64_t vcbool;
typedef double  vcfloat;
typedef uint8_t vcchar;
typedef int64_t vcsegdes;

#define VC_INT   0x01
#define VC_FLOAT 0x02
#define VC_BOOL  0x03
#define VC_FALSE INT64_C(0)
#define VC_TRUE  INT64_C(~0)
struct vcval {
    union {
        vcfloat f;
        vcint   i;
    } data;
};

struct vcvec {
    long          type;
    size_t        nval;
    struct vcval *data;
    struct vcvec *next;
};

struct vcsegdes {
    size_t  nseg;
    vcint  *data;
};

extern struct vcvec *vcvecstk;

struct vcvec    * vcgetvec(char *str, char **retstr);
struct vcsegdes * vcgetsegdes(char *str, char **retstr);
struct vcvec    * vcaddv(struct vcvec *src, struct vcvec *dest);
struct vcvec    * vcadds(struct vcvec *src, struct vcvec *dest);
struct vcvec    * vcsubv(struct vcvec *src, struct vcvec *dest);
struct vcvec    * vcsubs(struct vcvec *src, struct vcvec *dest);
struct vcvec    * vcmulv(struct vcvec *src, struct vcvec *dest);
struct vcvec    * vcmuls(struct vcvec *src, struct vcvec *dest);
struct vcvec    * vcdivv(struct vcvec *src, struct vcvec *dest);
struct vcvec    * vcdivs(struct vcvec *src, struct vcvec *dest);
struct vcvec    * vcmodv(struct vcvec *src, struct vcvec *dest);
struct vcvec    * vcmods(struct vcvec *src, struct vcvec *dest);

static __inline__ void
vcpushvec(struct vcvec *vec)
{
    vec->next = vcvecstk;
    vcvecstk = vec;

    return;
}

static __inline__ struct vcvec *
vcpopvec(void)
{
    struct vcvec *vec = vcvecstk;

    if (vec) {
        vcvecstk = vec->next;
    }

    return vec;
}

/* instruction IDs (opcodes) */
#define OPVADD       0x01
#define OPVSUB       0x02
#define OPVMUL       0x03
#define OPVDIV       0x04
#define OPVMOD       0x05
#define OPVLT        0x06
#define OPVLTE       0x07
#define OPVGT        0x08
#define OPVGTE       0x09
#define OPVEQ        0x0a
#define OPVINEQ      0x0b
#define OPVSHL       0x0c
#define OPVSHR       0x0d
#define OPVNOT       0x0e
#define OPVAND       0x0f
#define OPVOR        0x10
#define OPVXOR       0x11
#define OPVSELECT    0x12
#define OPVRAND      0x13
#define OPVFLOOR     0x14
#define OPVCEIL      0x15
#define OPVTRUNC     0x16
#define OPVROUND     0x17
#define OPVITOF      0x18
#define OPVITOB      0x19
#define OPBTOI       0x1a
#define OPVLOG       0x1b
#define OPVSQRT      0x1c
#define OPVEXP       0x1d
#define OPVSIN       0x1e
#define OPVCOS       0x1f
#define OPVTAN       0x20
#define OPVASIN      0x21
#define OPVACOS      0x22
#define OPVATAN      0x23
#define OPVSINH      0x24
#define OPVCOSH      0x25
#define OPVTANH      0x26
/* vector instructions */
#define OPVPLSCAN    0x27
#define OPVMULSCAN   0x28
#define OPVMAXSCAN   0x29
#define OPVMINSCAN   0x2a
#define OPVANDSACN   0x2b
#define OPVORSCAN    0x2c
#define OPVXORSCAN   0x2d
#define OPVPLREDUCE  0x2e
#define OPVMULREDUCE 0x2f
#define OPVMAXREDUCE 0x30
#define OPVMINREDUCE 0x31
#define OPVANDREDUCE 0x32
#define OPVORREDUCE  0x33
#define OPVXORREDUCE 0x34
#define OPVPERMUTE   0x35
#define OPVDPERMUTE  0x36
#define OPVFPERMUTE  0x37
#define OPVBPERMUTE  0x38
#define OPVBFPERMUTE 0x39
#define OPVDFPERMUTE 0x3a
#define OPVEXTRACT   0x3b
#define OPVREPLACE   0x3c
#define OPVPACK      0x3d
#define OPVRANKUP    0x3e
#define OPVRANKDOWN  0x3f
#define OPVDIST      0x40
#define OPVINDEX     0x41
#define OPVLENGTH    0x42
/* segment descriptor instructions */
#define OPVMKDES     0x43
#define OPVLENGTHS   0x44
#define OPVCOPY      0x45
#define OPVPOP       0x46
#define OPVCPOP      0x47
#define OPVPAIR      0x48
#define OPVUNPAIR    0x49

#endif /* __VCODE_VC_H__ */

