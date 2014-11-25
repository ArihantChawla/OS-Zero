#ifndef __COREWAR_X11_H__
#define __COREWAR_X11_H__

#if (ZEUSWINX11)

#if (ZEUSIMLIB2)
#include <Imlib2.h>
#endif

#define ZEUSNBUTTON 6

struct zeusx11win {
    Display *disp;
    Window   id;
};

#define ZEUSBUTTONNORMAL 0
#define ZEUSBUTTONHOVER  1
#define ZEUSBUTTONCLICK  2
#define ZEUSNBUTTONSTATE 3

struct zeusx11 {
    Display               *disp;
    Pixmap                 pixbuf;
    Window                 mainwin;
    Window                 simwin;
    Window                 buttonwin;
    Window                 dbwin;
    Window                 tipwin;
    int                    simw;
    int                    simh;
    int                    textw;
    int                    texth;
    int                    dbw;
    int                    dbh;
    GC                     bggc;
    GC                     selgc;
    GC                     buttongc;
    GC                     datgc;
    GC                     textgc;
    GC                     prog1gc;
    GC                     prog1datgc;
    GC                     prog2gc;
    GC                     prog2datgc;
    XFontStruct           *font;
    int                    fontw;
    int                    fonth;
    int                    fontasc;
    int                    depth;
    Colormap               colormap;
    Visual                *visual;
    int                    screen;
    int                    w;
    int                    h;
    Window               **buttons;
};

typedef void zeusx11buttonfunc(struct zeusx11 *, XEvent *);
struct zeusx11buttonfuncs {
    zeusx11buttonfunc *enter;
    zeusx11buttonfunc *leave;
    zeusx11buttonfunc *click;
    zeusx11buttonfunc *release;
    zeusx11buttonfunc *expose;
};

struct zeusx11buttonpmaps {
    Pixmap norm;
    Pixmap normmask;
    Pixmap hover;
    Pixmap hovermask;
    Pixmap click;
    Pixmap clickmask;
};

struct zeusx11buttons {
    Window                     wins[ZEUSNBUTTON];
    char                      *strs[ZEUSNBUTTON];
    int                        strlens[ZEUSNBUTTON];
    long                       exposed[ZEUSNBUTTON];
    zeusx11buttonfunc         *functab[ZEUSNBUTTON];
    struct zeusx11buttonpmaps  pmaps;
    struct zeusx11buttonfuncs  funcs;
};

void   zeusinitx11(struct zeusx11 *info);
void   zeusprocev(struct zeusx11 *x11);
void   zeusdrawdb(struct zeusx11 *x11, long pc);
void   zeusdrawsim(struct zeusx11 *x11);

#endif /* ZEUSWINX11 */

#endif /* __COREWAR_X11_H__ */

