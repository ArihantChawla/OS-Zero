#ifndef __IO_DRV_CHR_CON_H__
#define __IO_DRV_CHR_CON_H__

#include <gfx/rgb.h>

#if 0
extern void     (*conputs)(char *str);
extern void     (*conputchar)(int ch);

extern argb32_t   confgcolor;
extern argb32_t   conbgcolor;
#endif

typedef void conputsfunc(char *str);
typedef void conputcharfunc(int ch);

struct con {
    conputsfunc    *puts;       // function to draw string
    conputcharfunc *putchar;    // function to draw a character
    int32_t         fg;         // foreground pixel value
    int32_t         bg;         // background pixel value
    void           *buf;        // draw buffer (text or framebuffer)
    uint8_t         x;          // current x-coordinate or column
    uint8_t         y;          // current y-coordinate or row
    uint8_t         w;          // width in pixels
    uint8_t         h;          // height in pixels
    uint16_t        chatr;      // text attributes such as color
    long            nbufln;     // number of buffered lines
    void           *data;       // text buffers
} PACK;

extern struct con contab[NCON];
extern long       concur;

#endif /* __IO_DRV_CHR_CON_H__ */

