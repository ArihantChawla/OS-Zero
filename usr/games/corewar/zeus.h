#ifndef __COREWAR_ZEUS_H__
#define __COREWAR_ZEUS_H__

#define ZEUSWINX11 1

#if (ZEUSWINX11)
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <X11/Xutil.h>
#endif

#if (ZEUSWINX11)

struct zeusx11win {
    Display *disp;
    Window   id;
};

struct zeusx11 {
    Display     *disp;
    GC           textgc;
    GC           datgc;
    GC           prog1gc;
    GC           prog2gc;
    XFontStruct *font;
    int          fontw;
    int          fonth;
    int          depth;
    Colormap     colormap;
    Visual      *visual;
    int          screen;
    int          w;
    int          h;
    Window       mainwin;
    Window       simwin;
    Window       textwin;
    Window       dbwin;
    int          simw;
    int          simh;
    int          textw;
    int          texth;
    int          dbw;
    int          dbh;
    Pixmap       pixbuf;
    int          ndbline;
    void        *dbdata;
};

struct zeusx11 * zeusinitx11(void);

#endif /* ZEUSWINX11 */

#endif /* __COREWAR_ZEUS_H__ */

