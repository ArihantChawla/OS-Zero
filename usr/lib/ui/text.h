#ifndef __UI_TEXT_H__
#define __UI_TEXT_H__

#include <stdint.h>
#include <zero/utf8.h>

#if (UITEXT8BIT)
typedef uint8_t  uichar_t;
#elif (UITEXTASCII)
typedef char     uichar_t;
#endif
#if (UITEXTUCS2)
typedef uint16_t uiucs2_t;
#endif
#if (UITEXTUCS4)
typedef int32_t  uiucs4_t;
#endif

#endif /* __UI_TEXT_H__ */

