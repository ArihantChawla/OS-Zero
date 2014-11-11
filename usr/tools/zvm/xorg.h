#ifndef __ZVM_XORG_H__
#define __ZVM_XORG_H__

#include <Imlib2.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <X11/Xutil.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#define ZVMHRULER       "../../share/img/hruler.png"
#define ZVMVRULER       "../../share/img/vruler.png"
#define ZVMTEXTFONT     "fixed"
#define XORGBUTTONNORMAL 0
#define XORGBUTTONHOVER  1
#define XORGBUTTONPRESS  2
#define XORGNBUTTONSTATE 3

void zvminitui(void);

typedef void zvmevfunc_t(Window, void *);

struct zvmxorgui {
    Display     *disp;
    int          connfd;
    Window       mainwin;
    Window       uiwin;
    Window       dbwin;
#if 0
    Window       hrulerwin;
    Pixmap       hrulerpmap;
#endif
    GC           uigc;
    GC           dbgc;
    FT_Library   ftlib;
    XFontStruct *textfont;
    int          fontw;
    int          fonth;
    Imlib_Image *hruler;
    Imlib_Image *vruler;
    int          hrulerh;
    int          vrulerw;
    zvmevfunc_t *uievfunctab;
    zvmevfunc_t *dbevfunctab;
};

#endif /* __ZVM_XORG_H__ */

