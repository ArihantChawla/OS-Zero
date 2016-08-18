#ifndef __ZERO_X86_TYPES_H__
#define __ZERO_X86_TYPES_H__

/* far pointer structure */
#include <zero/pack.h>
struct m_farptr {
    uint16_t lim;
    uint32_t adr;
} PACKED();
#include <zero/nopack.h>

/* FPU registers */

struct m_fpreg {
    int16_t _sig[4];
    int16_t _exp;
};

struct m_fpxreg {
    int16_t _sig[4];
    int16_t _exp;
    int16_t _pad[3];
};

struct m_xmmreg {
    int32_t _elem[4];
};

#define FPX_MAGIC1      0x46505853U
#define FPX_MAGIC2      0x46505845U
#define FPX_MAGIC2_SIZE sizeof(FPX_MAGIC2)

struct m_fpdata {
    int32_t _magic1;    // FPX_MAGIC1
    int32_t _extsize;
    int64_t _xstate;
    int32_t _xsize;
    int32_t _pad[7];
};

#endif /* __ZERO_X86_TYPES_H__ */

