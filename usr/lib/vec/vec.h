#ifndef __VEC_VEC_H__
#define __VEC_VEC_H__

#include <stdint.h>

typedef int64_t vecint;
typedef int64_t vecbool;
typedef double  vecfloat;
typedef uint8_t vecchar;
typedef int64_t vecsegdes;

#define VEC_INT   0x01
#define VEC_FLOAT 0x02
#define VEC_BOOL  0x03
#define VEC_FALSE INT64_C(0)
#define VEC_TRUE  INT64_C(1)
struct vecval {
    union {
        vecfloat f;
        vecint   i;
    } data;
};

struct vec {
    long           type;
    size_t         nval;
    struct vecval *data;
    struct vec    *next;
};

struct vec32 {
    int32_t i1;
    int32_t i2;
} PACKED();

struct vecsegdes {
    size_t  nseg;
    vecint *data;
};

#if 0
extern struct vec *vecstk;
#endif

struct vec    * vecgetvec(char *str, char **retstr);
struct vecsegdes * vecgetsegdes(char *str, char **retstr);
struct vec    * vecaddv(struct vec *src, struct vec *dest);
struct vec    * vecadds(struct vec *src, struct vec *dest);
struct vec    * vecsubv(struct vec *src, struct vec *dest);
struct vec    * vecsubs(struct vec *src, struct vec *dest);
struct vec    * vecmulv(struct vec *src, struct vec *dest);
struct vec    * vecmuls(struct vec *src, struct vec *dest);
struct vec    * vecdivv(struct vec *src, struct vec *dest);
struct vec    * vecdivs(struct vec *src, struct vec *dest);
struct vec    * vecmodv(struct vec *src, struct vec *dest);
struct vec    * vecmods(struct vec *src, struct vec *dest);

#if 0
static __inline__ void
vecpushvec(struct vec *vec)
{
    vec->next = vecstk;
    vecstk = vec;

    return;
}

static __inline__ struct vec *
vecpopvec(void)
{
    struct vec *vec = vecstk;

    if (vec) {
        vecstk = vec->next;
    }

    return vec;
}
#endif

#endif /* __VEC_VEC_H__ */

