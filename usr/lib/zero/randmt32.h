#ifndef __ZERO_RANDMT32_H__
#define __ZERO_RANDMT32_H__

#include <stdint.h>

#define RANDMT32_MAX 0xffffffff

void          srandmt32(int32_t seed);
unsigned long randmt32(void);

#endif /* __ZERO_RANDMT32_H__ */

