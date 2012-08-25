#ifndef __ZPC_X11_H__
#define __ZPC_X11_H__

#include <zpc/op.h>

#define NBUTTON 3
#define toevbutton(b) ((b) - 1)

typedef void x11evfunc_t(void *, XEvent *);

void x11drawdisp(void);

typedef void x11eventhandler_t(void *, XEvent *);
typedef void x11keyhandler_t(char ch);

#if (ZPCIMLIB2)
void   imlib2init(struct x11app *app);
Pixmap imlib2loadimage(struct x11app *app, const char *filename, int w, int h);
#endif

struct x11wininfo {
    Window             id;
    const char        *str;
    const char        *topstr1;
    const char        *topstr2;
    GC                 textgc;
    long               type;
    long               num;
    long               parm;
    long               row;
    long               col;
    struct x11wininfo *next;
    x11evfunc_t       *evfunc[LASTEvent];
    zpccop_t          *clickfunc[NBUTTON];
    zpcfop_t          *clickfuncdbl[NBUTTON];
};

struct x11app {
    Window   win;
    Display *display;
    int      screen;
    int      depth;
    Visual  *visual;
    Colormap colormap;
};

#endif /* __ZPC_X11_H__ */

