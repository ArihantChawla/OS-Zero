#ifndef __ZERO_LKMAP_H__
#define __ZERO_LKMAP_H__

typedef struct {
    volatile long  lk;
    long           nbit;
    long          *bits;
} zerolkmap;

long lkmapinit(long n, zerolkmap *lkmap);

#endif /* __ZERO_LKMAP_H__ */

