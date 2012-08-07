#ifndef __ZPC_X11_H__
#define __ZPC_X11_H__

#define NBUTTON 3
#define toevbutton(b) ((b) - 1)

typedef void x11evfunc_t(void *, XEvent *);
typedef void copfunc_t(void *, void *, void *);

struct x11win {
    Window         id;
    const char    *str;
    const char    *asmstr;
    long           num;
    long           narg;
    long           row;
    long           col;
    struct x11win *next;
    x11evfunc_t   *evfunc[LASTEvent];
    copfunc_t     *clickfunc[NBUTTON];
    copfunc_t     *clickfuncflt[NBUTTON];
    copfunc_t     *clickfuncdbl[NBUTTON];
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

