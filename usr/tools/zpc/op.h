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

/* zero.c */
void abs8(void *arg1, void *dummy, void *ret);
void abs16(void *arg1, void *dummy, void *ret);
void abs32(void *arg1, void *dummy, void *ret);
void abs64(void *arg1, void *dummy, void *ret);

/* TODO: hack support for big endian architectures */
#define wpcgetarg8(ptr)                                                 \
    (*(int8_t *)ptr)
#define wpcgetarg16(ptr)                                                \
    (*(int16_t *)ptr)
#define wpcgetarg32(ptr)                                                \
    (*(int32_t *)ptr)
#define wpcgetarg64(ptr)                                                \
    (*(int64_t *)ptr)
#define wpcsetval8(ptr, val)                                            \
    *(int8_t *)(ptr) = (val)
#define wpcsetval16(ptr, val)                                           \
    *(int16_t *)(ptr) = (val)
#define wpcsetval32(ptr, val)                                           \
    *(int32_t *)(ptr) = (val)
#define wpcsetval64(ptr, val)                                           \
    *(int64_t *)(ptr) = (val)

#endif /* __PCALC_OP_H__ */

