#ifndef __IO_DRV_CHR_CONS_H__
#define __IO_DRV_CHR_CONS_H__

#include <stdint.h>
#include <gfx/rgb.h>

#define CONSNTEXTROW 4096

void consinit(int w, int h);

typedef uint8_t conschar_t;

typedef void consputsfunc(char *str);
typedef void consputcharfunc(int ch);

struct cons {
    consputsfunc    *puts;      // function to draw a string
    consputcharfunc *putchar;   // function to draw a character
    gfxargb32_t      fg;        // foreground pixel value
    gfxargb32_t      bg;        // background pixel value
    void            *buf;       // draw buffer (text or framebuffer)
    long             col;       // current column
    long             row;       // current row
    long             ncol;      // # of columns
    long             nrow;      // # of rows
//    long             w;         // width in pixels
//    long             h;         // height in pixels
    long             chatr;     // text attributes such as color
    long             ntextrow;  // number of buffer lines
    void            *textbuf;   // text buffers
} PACK();

extern struct cons constab[NCONS];
extern long        conscur;

#endif /* __IO_DRV_CHR_CONS_H__ */

