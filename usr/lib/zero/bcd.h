#ifndef __ZERO_BCD_H__
#define __ZERO_BCD_H__

#include <zero/trix.h>

#define bcd2bin(val) (((val) & 0x0f) + ((val) >> 4) * 10)
#define bin2bcd(val) ((divu10(val) << 4) + modu10(val))

#endif /* __ZERO_BCD_H__ */

