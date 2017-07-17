#ifndef __ZPM_TYPES_H__
#define __ZPM_TYPES_H__

#include <zpm/conf.h>
#include <stdint.h>

typedef int8_t   zpmbyte;
typedef uint8_t  zpmubyte;
typedef int16_t  zpmword;
typedef uint16_t zpmuword;
typedef int32_t  zpmlong;
typedef uint32_t zpmulong;
#if defined(ZPM64BIT)
typedef int64_t  zpmquad;
typedef uint64_t zpmuquad;
typedef zpmquad  zpmreg;
typedef zpmuquad zpmureg;
#else
typedef zpmlong  zpmreg;
typedef zpmulong zpmureg;
#endif
typedef zpmureg  zpmadr;

#endif /* __ZPM_TYPES_H__ */

