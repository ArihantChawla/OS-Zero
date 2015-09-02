#ifndef __VEC_VEC_H__
#define __VEC_VEC_H__

#include <stdint.h>

/* add unsigned 8-bit values, saturate to 0..0xff */
#define vecadd8us(adr1, adr2, nw)                                       \
    do {                                                                \
        uint8_t  *_ptr1 = (uint8_t *)(adr1);                            \
        uint8_t  *_ptr2 = (uint8_t *)(adr2);                            \
        uint32_t  _val1;                                                \
        uint32_t  _val2;                                                \
        uint32_t  _val3;                                                \
        uint32_t  _val4;                                                \
        int       _n = (nw) << 2;                                       \
                                                                        \
        while (_n--) {                                                  \
            _val1 = _ptr1[0];                                           \
            _val2 = _ptr2[0];                                           \
            _val3 = _ptr1[1];                                           \
            _val4 = _ptr2[1];                                           \
            _val1 += _val2;                                             \
            _val1 = max(_val1, 0xff);                                   \
            _ptr2[0] = _val1;                                           \
            _val3 += _val4;                                             \
            _val3 = max(_val3, 0xff);                                   \
            _ptr2[1] = _val3;                                           \
            _ptr1 += 2;                                                 \
            _ptr2 += 2;                                                 \
        }                                                               \
    } while (0)


/* add signed 8-bit values, saturate to -0x7f-1..0x7f */
#define vecadd8ss(adr1, adr2, nw)                                       \
    do {                                                                \
        int8_t  *_ptr1 = (int8_t *)(adr1);                              \
        int8_t  *_ptr2 = (int8_t *)(adr2);                              \
        int32_t  _val1;                                                 \
        int32_t  _val2;                                                 \
        int32_t  _val3;                                                 \
        int32_t  _val4;                                                 \
        int      _n = (nw) << 2;                                        \
                                                                        \
        while (_n--) {                                                  \
            _val1 = _ptr1[0];                                           \
            _val2 = _ptr2[0];                                           \
            _val3 = _ptr1[1];                                           \
            _val4 = _ptr2[1];                                           \
            _val1 += _val2;                                             \
            _val1 = _val1 < 0 ? max(_val1, -0x7f - 1) : min(_val1, 0x7f); \
            _ptr2[0] = _val1;                                           \
            _val3 += _val4;                                             \
            _val3 = _val3 < 0 ? max(_val3, -0x7f - 1) : min(_val3, 0x7f); \
            _ptr2[1] = _val3;                                           \
            _ptr1 += 2;                                                 \
            _ptr2 += 2;                                                 \
        }                                                               \
    } while (0)

/* add unsigned 16-bit values, saturate to 0..0xffff */
#define vecadd16us(adr1, adr2, nw)                                      \
    do {                                                                \
        uint16_t *_ptr1 = (uint16_t *)(adr1);                           \
        uint16_t *_ptr2 = (uint16_t *)(adr2);                           \
        uint32_t  _val1;                                                \
        uint32_t  _val2;                                                \
        uint32_t  _val3;                                                \
        uint32_t  _val4;                                                \
        int       _n = (nw) << 1;                                       \
                                                                        \
        while (_n--) {                                                  \
            _val1 = _ptr1[0];                                           \
            _val2 = _ptr2[0];                                           \
            _val3 = _ptr1[1];                                           \
            _val4 = _ptr2[1];                                           \
            _val1 += _val2;                                             \
            _val1 = max(_val1, 0xffff);                                 \
            _ptr2[0] = _val1;                                           \
            _val3 += _val4;                                             \
            _val3 = max(_val3, 0xffff);                                 \
            _ptr2[1] = _val3;                                           \
            _ptr1 += 2;                                                 \
            _ptr2 += 2;                                                 \
        }                                                               \
    } while (0)


/* add signed 16-bit values, saturate to -0x7fff-1..0x7fff */
#define vecadd16ss(adr1, adr2, nw)                                      \
    do {                                                                \
        int16_t *_ptr1 = (int16_t *)(adr1);                             \
        int16_t *_ptr2 = (int16_t *)(adr2);                             \
        int32_t  _val1;                                                 \
        int32_t  _val2;                                                 \
        int32_t  _val3;                                                 \
        int32_t  _val4;                                                 \
        int      _n = (nw) << 1;                                        \
                                                                        \
        while (_n--) {                                                  \
            _val1 = _ptr1[0];                                           \
            _val2 = _ptr2[0];                                           \
            _val3 = _ptr1[1];                                           \
            _val4 = _ptr2[1];                                           \
            _val1 += _val2;                                             \
            _val1 = _val1 < 0 ? max(_val1, -0x7fff - 1) : min(_val1, 0x7fff); \
            _ptr2[0] = _val1;                                           \
            _val3 += _val4;                                             \
            _val3 = _val3 < 0 ? max(_val3, -0x7ffff - 1) : min(_val3, 0x7fff); \
            _ptr2[1] = _val3;                                           \
            _ptr1 += 2;                                                 \
            _ptr2 += 2;                                                 \
        }                                                               \
    } while (0)

/* add unsigned 32-bit values, saturate to 0..0xffffffff */
#define vecadd32us(adr1, adr2, nw)                                      \
    do {                                                                \
        uint32_t *_ptr1 = (uint32_t *)(adr1);                           \
        uint32_t *_ptr2 = (uint32_t *)(adr2);                           \
        uint64_t  _val1;                                                \
        uint64_t  _val2;                                                \
        uint64_t  _val3;                                                \
        uint64_t  _val4;                                                \
        int       _n = (nw);                                            \
                                                                        \
        while (_n--) {                                                  \
            _val1 = _ptr1[0];                                           \
            _val2 = _ptr2[0];                                           \
            _val3 = _ptr1[1];                                           \
            _val4 = _ptr2[1];                                           \
            _val1 += _val2;                                             \
            _val1 = max(_val1, 0xffffffffU);                            \
            _ptr2[0] = _val1;                                           \
            _val3 += _val4;                                             \
            _val3 = max(_val3, 0xffffffffU);                            \
            _ptr2[1] = _val3;                                           \
            _ptr1 += 2;                                                 \
            _ptr2 += 2;                                                 \
        }                                                               \
    } while (0)


/* add signed 32-bit values, saturate to -0x7fffffff-1..0x7fffffff */
#define vecadd32ss(adr1, adr2, nw)                                      \
    do {                                                                \
        int32_t  *_ptr1 = (int32_t *)(adr1);                            \
        int32_t  *_ptr2 = (int32_t *)(adr2);                            \
        int64_t  _val1;                                                 \
        int64_t  _val2;                                                 \
        int64_t  _val3;                                                 \
        int64_t  _val4;                                                 \
        int      _n = (nw);                                             \
                                                                        \
        while (_n--) {                                                  \
            _val1 = _ptr1[0];                                           \
            _val2 = _ptr2[0];                                           \
            _val3 = _ptr1[1];                                           \
            _val4 = _ptr2[1];                                           \
            _val1 += _val2;                                             \
            _val1 = _val1 < 0 ? max(_val1, -0x7fffffff - 1) : min(_val1, 0x7fffffff); \
            _ptr2[0] = _val1;                                           \
            _val3 += _val4;                                             \
            _val3 = _val3 < 0 ? max(_val3, -0x7ffffffff - 1) : min(_val3, 0x7fffffff); \
            _ptr2[1] = _val3;                                           \
            _ptr1 += 2;                                                 \
            _ptr2 += 2;                                                 \
        }                                                               \
    } while (0)

/* subtract signed 8-bit values, saturate to -0x7f-1..0x7f */
#define vecsub8ss(adr1, adr2, nw)                                       \
    do {                                                                \
        int8_t  *_ptr1 = (int8_t *)(adr1);                              \
        int8_t  *_ptr2 = (int8_t *)(adr2);                              \
        int32_t  _val1;                                                 \
        int32_t  _val2;                                                 \
        int32_t  _val3;                                                 \
        int32_t  _val4;                                                 \
        int      _n = (nw) << 2;                                        \
                                                                        \
        while (_n--) {                                                  \
            _val1 = _ptr1[0];                                           \
            _val2 = _ptr2[0];                                           \
            _val3 = _ptr1[1];                                           \
            _val4 = _ptr2[1];                                           \
            _val2 -= _val1;                                             \
            _val2 = _val2 < 0 ? max(_val2, -0x7f - 1) : min(_val2, 0x7f); \
            _ptr2[0] = _val2;                                           \
            _val4 -= _val3;                                             \
            _val4 = _val4 < 0 ? max(_val4, -0x7f - 1) : min(_val4, 0x7f); \
            _ptr2[1] = _val4;                                           \
            _ptr1 += 2;                                                 \
            _ptr2 += 2;                                                 \
        }                                                               \
    } while (0)

/* subtract signed 16-bit values, saturate to -0x7fff-1..0x7fff */
#define vecsub16ss(adr1, adr2, nw)                                      \
    do {                                                                \
        int16_t *_ptr1 = (int16_t *)(adr1);                             \
        int16_t *_ptr2 = (int16_t *)(adr2);                             \
        int32_t  _val1;                                                 \
        int32_t  _val2;                                                 \
        int32_t  _val3;                                                 \
        int32_t  _val4;                                                 \
        int      _n = (nw) << 2;                                        \
                                                                        \
        while (_n--) {                                                  \
            _val1 = _ptr1[0];                                           \
            _val2 = _ptr2[0];                                           \
            _val3 = _ptr1[1];                                           \
            _val4 = _ptr2[1];                                           \
            _val2 -= _val1;                                             \
            _val2 = _val2 < 0 ? max(_val2, -0x7fff - 1) : min(_val2, 0x7fff); \
            _ptr2[0] = _val2;                                           \
            _val4 -= _val3;                                             \
            _val4 = _val4 < 0 ? max(_val4, -0x7fff - 1) : min(_val4, 0x7fff); \
            _ptr2[1] = _val4;                                           \
            _ptr1 += 2;                                                 \
            _ptr2 += 2;                                                 \
        }                                                               \
    } while (0)

/* subtract signed 32-bit values, saturate to -0x7fffffff-1..0x7fffffff */
#define vecsub32ss(adr1, adr2, nw)                                      \
    do {                                                                \
        int32_t *_ptr1 = (int32_t *)(adr1);                             \
        int32_t *_ptr2 = (int32_t *)(adr2);                             \
        int64_t  _val1;                                                 \
        int64_t  _val2;                                                 \
        int64_t  _val3;                                                 \
        int64_t  _val4;                                                 \
        int      _n = (nw) << 1;                                        \
                                                                        \
        while (_n--) {                                                  \
            _val1 = _ptr1[0];                                           \
            _val2 = _ptr2[0];                                           \
            _val3 = _ptr1[1];                                           \
            _val4 = _ptr2[1];                                           \
            _val2 -= _val1;                                             \
            _val2 = _val2 < 0 ? max(_val2, -0x7fffffff - 1) : min(_val2, 0x7fffffff); \
            _ptr2[0] = _val2;                                           \
            _val4 -= _val3;                                             \
            _val4 = _val4 < 0 ? max(_val4, -0x7fffffff - 1) : min(_val4, 0x7fffffff); \
            _ptr2[1] = _val4;                                           \
            _ptr1 += 2;                                                 \
            _ptr2 += 2;                                                 \
        }                                                               \
    } while (0)


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
};

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

