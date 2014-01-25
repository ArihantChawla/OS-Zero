#ifndef __IO_DRV_CHAR_CON_H__
#define __IO_DRV_CHAR_CON_H__

#include <gfx/rgb.h>

extern void     (*conputs)(char *str);
extern void     (*conputchar)(int ch);

extern argb32_t   confgcolor;
extern argb32_t   conbgcolor;

#endif /* __IO_DRV_CHAR_CON_H__ */

