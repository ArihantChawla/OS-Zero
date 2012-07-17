#ifndef __I387_MATH_H__
#define __I387_MATH_H__

#define getnan(d)                                                       \
    (dsetexp(d, 0x7ff), dsetmant(d, 0x000fffffffffffff), (d))
#define getsnan(d) \
    (dsetsign(d), dsetexp(d, 0x7ff), dsetmant(d, 0x000fffffffffffff), (d))
#define getnanf(f)                                                      \
    (fsetexp(f, 0x7ff), fsetmant(f, 0x007fffff), (f))
#define getsnanf(f)                                                     \
    (fsetsign(f), fsetexp(f, 0x7ff), fsetmant(f, 0x007fffff), (f))
#define getnanl(ld)                                                      \
    (ldsetexp(f, 0x7fff), ldsetmant(ld, 0xffffffffffffffff), (ld))
#define getsnanl(ld) \
    (ldsetsign(ld), ldsetexp(ld, 0x7fff), ldsetmant(ld, 0xffffffffffffffff), (ld))

#endif /* __I387_MATH_H__ */

