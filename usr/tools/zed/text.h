#ifndef __ZED_TEXT_H__
#define __ZED_TEXT_H__

#include <stdint.h>

#if (ZED8BIT)
typedef uint8_t  zedtext;
#elif (ZEDUCS2)
typedef uint16_t zedtext;
#else
typedef int32_t  zedtext;
#endif

#endif /* __ZED_TEXT_H__ */
