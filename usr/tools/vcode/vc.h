#ifndef __VCODE_VC_H__
#define __VCODE_VC_H__

#include <stdint.h>
#include <float.h>

typedef int64_t vcint;
typedef int64_t vcbool;
typedef double  vcfloat;
typedef uint8_t vcchar;
typedef int64_t vcsegdes;

#define VC_INT   0x01
#define VC_FLOAT 0x02
struct vcval {
    long type;
    union {
        vcfloat f;
        vcint   i;
    }    data;
};

struct vcvec {
    size_t        nval;
    struct vcval *data;
    struct vcvec *next;
};

struct vcsegdes {
    size_t  nseg;
    vcint  *data;
};

#endif /* __VCODE_VC_H__ */

