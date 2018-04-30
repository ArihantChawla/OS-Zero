#ifndef __V0_TYPES_H__
#define __V0_TYPES_H__

#include <v0/conf.h>
#include <stdint.h>

typedef int8_t    v0byte;
typedef uint8_t   v0ubyte;
typedef int16_t   v0word;
typedef uint16_t  v0uword;
typedef int32_t   v0long;
typedef uint32_t  v0ulong;
#if defined(V064BIT)
typedef int64_t   v0quad;
typedef uint64_t  v0uquad;
typedef v0quad    v0reg;
typedef v0uquad   v0ureg;
#else
typedef v0long    v0reg;
typedef v0ulong   v0ureg;
#endif
typedef v0ureg    v0adr;
typedef char     *v0ptr;

#endif /* __V0_TYPES_H__ */

