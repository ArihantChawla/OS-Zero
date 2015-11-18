#ifndef __ZERO_X86_TYPES_H__
#define __ZERO_X86_TYPES_H__

/* far pointer structure */
#include <zero/pack.h>
struct m_farptr {
    uint16_t lim;
    uint32_t adr;
} PACKED;
#include <zero/nopack.h>

#endif /* __ZERO_X86_TYPES_H__ */

