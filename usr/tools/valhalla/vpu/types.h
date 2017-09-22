#ifndef __VPU_TYPES_H__
#define __VPU_TYPES_H__

#include <vpu/conf.h>
#include <stdint.h>

typedef int8_t   vpubyte;
typedef uint8_t  vpuubyte;
typedef int16_t  vpuword;
typedef uint16_t vpuuword;
typedef int32_t  vpulong;
typedef uint32_t vpuulong;
#if defined(VPU64BIT)
typedef int64_t  vpuquad;
typedef uint64_t vpuuquad;
typedef vpuquad  vpureg;
typedef vpuuquad vpuureg;
#else
typedef vpulong  vpureg;
typedef vpuulong vpuureg;
#endif
typedef vpuureg  vpuadr;

#endif /* __VPU_TYPES_H__ */

