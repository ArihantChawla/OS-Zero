#ifndef __ZERO_X86_TYPES_H__
#define __ZERO_X86_TYPES_H__

/* far pointer structure */
#include <zero/pack.h>
struct m_farptr {
    uint16_t lim;
    uint32_t adr;
} PACKED;
#include <zero/nopack.h>

/* FPU registers */

struct fpreg {
    int16_t _sig[4];
    int16_t _exp;
};

struct fpxreg {
    int16_t _sig[4];
    int16_t _exp;
    int16_t _pad[3];
};

struct xmmreg {
    int32_t _elem[4];
};

#endif /* __ZERO_X86_TYPES_H__ */

