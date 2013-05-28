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

struct vcvec * vcaddv(struct vcvec *src, struct vcvec *dest);
struct vcvec * vcadds(struct vcvec *src, struct vcvec *dest);
struct vcvec * vcsubv(struct vcvec *src, struct vcvec *dest);
struct vcvec * vcsubs(struct vcvec *src, struct vcvec *dest);
struct vcvec * vcmulv(struct vcvec *src, struct vcvec *dest);
struct vcvec * vcmuls(struct vcvec *src, struct vcvec *dest);
struct vcvec * vcdivv(struct vcvec *src, struct vcvec *dest);
struct vcvec * vcdivs(struct vcvec *src, struct vcvec *dest);
struct vcvec * vcmodv(struct vcvec *src, struct vcvec *dest);
struct vcvec * vcmods(struct vcvec *src, struct vcvec *dest);

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

#endif /* __VCODE_VC_H__ */

