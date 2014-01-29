#ifndef __IO_DRV_CHR_CONS_H__
#define __IO_DRV_CHR_CONS_H__

#include <stdint.h>

#include <gfx/rgb.h>

void consinit(int w, int h);

typedef void consputsfunc(char *str);
typedef void consputcharfunc(int ch);

struct cons {
    consputsfunc    *puts;       // function to draw string
    consputcharfunc *putchar;    // function to draw a character
    int32_t          fg;         // foreground pixel value
    int32_t          bg;         // background pixel value
    void            *buf;        // draw buffer (text or framebuffer)
    uint8_t          x;          // current x-coordinate or column
    uint8_t          y;          // current y-coordinate or row
    uint8_t          w;          // width in pixels
    uint8_t          h;          // height in pixels
    uint16_t         chatr;      // text attributes such as color
    long             nbufln;     // number of buffered lines
    void            *data;       // text buffers
} PACK();

extern struct cons constab[NCONS];
extern long        conscur;

#endif /* __IO_DRV_CHR_CONS_H__ */

