#include <stdlib.h>
#include <string.h>
#include <zero/trix.h>
#include <corewar/cw.h>
#include <corewar/rc.h>
#include <corewar/zeus.h>
#if (ZEUSWINX11)
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <X11/Xutil.h>
#endif

#define ZEUSTEXTNCOL 80
#define ZEUSTEXTNROW 16
#define ZEUSDBNCOL   80
#define ZEUSDBNROW   4
#define ZEUSSIMW     256
#define ZEUSSIMH     32

extern struct cwinstr *cwoptab;

void
zeusinitx11font(struct zeusx11 *x11)
{
    XFontStruct *font;

    font = XLoadQueryFont(x11->disp, "fixed");
    if (font) {
        x11->font = font;
        x11->fontw = font->max_bounds.width;
        x11->fonth = font->ascent + font->descent;
    } else {
        fprintf(stderr, "failed to load font\n");

        exit(1);
    }

    return;
}

void
zeusinitx11win(struct zeusx11 *x11)
{
    XSetWindowAttributes atr = { 0 };
    Window               win;
    Window               parent = RootWindow(x11->disp, x11->screen);
    int                  winw = max((ZEUSTEXTNCOL + ZEUSDBNCOL) * x11->fontw,
                                    ZEUSSIMW * 5);
    int                  winh = (ZEUSTEXTNROW + ZEUSDBNROW) * x11->fonth + ZEUSSIMH * 5;
    long                 x = 0;
    long                 y = 0;

    atr.background_pixel = BlackPixel(x11->disp, x11->screen);
    win = XCreateWindow(x11->disp,
                        parent,
                        0, 0,
                        winw, winh,
                        0,
                        x11->depth,
                        InputOutput,
                        x11->visual,
                        CWBackPixel,
                        &atr);
    if (!win) {
        fprintf(stderr, "failed to create main window\n");

        exit(1);
    }
    x11->mainwin = win;
    x11->w = winw;
    x11->h = winh;
    parent = win;
    winw = ZEUSSIMW * 5;
    winh = ZEUSSIMH * 5;
    win = XCreateWindow(x11->disp,
                        parent,
                        x, y,
                        winw, winh,
                        0,
                        x11->depth,
                        InputOutput,
                        x11->visual,
                        CWBackPixel,
                        &atr);
    if (!win) {
        fprintf(stderr, "failed to create simulation window\n");

        exit(1);
    }
    x11->simwin = win;
    x11->simw = winw;
    x11->simh = winh;
    y += winh;
    winw = ZEUSTEXTNCOL * x11->fontw;
    winh = ZEUSTEXTNROW * x11->fonth;
    win = XCreateWindow(x11->disp,
                        parent,
                        x, y,
                        winw, winh,
                        0,
                        x11->depth,
                        InputOutput,
                        x11->visual,
                        CWBackPixel,
                        &atr);
    if (!win) {
        fprintf(stderr, "failed to create text window\n");

        exit(1);
    }
    x11->textwin = win;
    x11->textw = winw;
    x11->texth = winh;
    y += winh;
    winw = ZEUSDBNCOL * x11->fontw;
    winh = ZEUSDBNROW * x11->fonth;
    win = XCreateWindow(x11->disp,
                        parent,
                        x, y,
                        winw, winh,
                        0,
                        x11->depth,
                        InputOutput,
                        x11->visual,
                        CWBackPixel,
                        &atr);
    if (!win) {
        fprintf(stderr, "failed to create debug window\n");

        exit(1);
    }
    x11->dbwin = win;
    x11->dbw = winw;
    x11->dbh = winh;

    return;
}

void
zeusinitx11buf(struct zeusx11 *x11)
{
    Pixmap pmap = XCreatePixmap(x11->disp,
                                x11->simwin,
                                x11->w,
                                x11->h,
                                x11->depth);

    if (!pmap) {
        fprintf(stderr, "failed to create buffer pixmap\n");

        exit(1);
    }
    XFillRectangle(x11->disp, pmap,
                   x11->bggc,
                   0, 0,
                   x11->w, x11->h);
    x11->pixbuf = pmap;

    return;
}

void
zeusinitx11gc(struct zeusx11 *x11)
{
    XGCValues       gcval;
    GC              gc;
    XColor          color;

    gcval.foreground = BlackPixel(x11->disp, DefaultScreen(x11->disp));
    gcval.graphics_exposures = False;
    gcval.font = x11->font->fid;
    gcval.function = GXcopy;
    gcval.line_width = 1;
    gc = XCreateGC(x11->disp, x11->textwin,
                   GCForeground | GCFunction | GCLineWidth | GCFont,
                   &gcval);
    if (!gc) {
        fprintf(stderr, "failed to create GC\n");

        exit(1);
    }
    x11->bggc = gc;
    if (!XParseColor(x11->disp,
                     x11->colormap,
                     "magenta",
                     &color)) {
        fprintf(stderr, "failed to parse color\n");

        exit(1);
    }
    if (!XAllocColor(x11->disp,
                     x11->colormap,
                     &color)) {
        fprintf(stderr, "failed to allocate color\n");

        exit(1);
    }
    gcval.foreground = color.pixel;
    gc = XCreateGC(x11->disp, x11->textwin,
                   GCForeground | GCFunction | GCLineWidth | GCFont,
                   &gcval);
    if (!gc) {
        fprintf(stderr, "failed to create GC\n");

        exit(1);
    }
    x11->datgc = gc;
    if (!XParseColor(x11->disp,
                     x11->colormap,
                     "green",
                     &color)) {
        fprintf(stderr, "failed to parse color\n");

        exit(1);
    }
    if (!XAllocColor(x11->disp,
                     x11->colormap,
                     &color)) {
        fprintf(stderr, "failed to allocate color\n");

        exit(1);
    }
    gcval.foreground = color.pixel;
    gc = XCreateGC(x11->disp, x11->textwin,
                   GCForeground | GCFunction | GCLineWidth | GCFont,
                   &gcval);
    if (!gc) {
        fprintf(stderr, "failed to create GC\n");

        exit(1);
    }
    x11->prog1gc = gc;
    if (!XParseColor(x11->disp,
                     x11->colormap,
                     "dark green",
                     &color)) {
        fprintf(stderr, "failed to parse color\n");

        exit(1);
    }
    if (!XAllocColor(x11->disp,
                     x11->colormap,
                     &color)) {
        fprintf(stderr, "failed to allocate color\n");

        exit(1);
    }
    gcval.foreground = color.pixel;
    gc = XCreateGC(x11->disp, x11->textwin,
                   GCForeground | GCFunction | GCLineWidth | GCFont,
                   &gcval);
    if (!gc) {
        fprintf(stderr, "failed to create GC\n");

        exit(1);
    }
    x11->prog1datgc = gc;
    if (!XParseColor(x11->disp,
                     x11->colormap,
                     "yellow",
                     &color)) {
        fprintf(stderr, "failed to parse color\n");

        exit(1);
    }
    if (!XAllocColor(x11->disp,
                     x11->colormap,
                     &color)) {
        fprintf(stderr, "failed to allocate color\n");

        exit(1);
    }
    gcval.foreground = color.pixel;
    gc = XCreateGC(x11->disp, x11->textwin,
                   GCForeground | GCFunction | GCLineWidth | GCFont,
                   &gcval);
    if (!gc) {
        fprintf(stderr, "failed to create GC\n");

        exit(1);
    }
    x11->prog2gc = gc;
    if (!XParseColor(x11->disp,
                     x11->colormap,
                     "gold",
                     &color)) {
        fprintf(stderr, "failed to parse color\n");

        exit(1);
    }
    if (!XAllocColor(x11->disp,
                     x11->colormap,
                     &color)) {
        fprintf(stderr, "failed to allocate color\n");

        exit(1);
    }
    gcval.foreground = color.pixel;
    gc = XCreateGC(x11->disp, x11->textwin,
                   GCForeground | GCFunction | GCLineWidth | GCFont,
                   &gcval);
    if (!gc) {
        fprintf(stderr, "failed to create GC\n");

        exit(1);
    }
    x11->prog2datgc = gc;

    return;
}

struct zeusx11 *
zeusinitx11(void)
{
    Display        *disp;
    struct zeusx11 *info = calloc(1, sizeof(struct zeusx11));

    if (info) {
        XInitThreads();
        disp = XOpenDisplay(NULL);
        if (!disp) {
            fprintf(stderr, "failed to open display\n");

            exit(1);
        }
        info->disp = disp;
        info->screen = DefaultScreen(disp);
        info->colormap = DefaultColormap(disp, info->screen);
        info->depth = DefaultDepth(disp, info->screen);
        info->visual = DefaultVisual(disp, info->screen);
        zeusinitx11font(info);
        zeusinitx11win(info);
        zeusinitx11gc(info);
        zeusinitx11buf(info);
        XSelectInput(info->disp, info->simwin, ButtonPressMask | ExposureMask);
        XMapRaised(info->disp, info->simwin);
        XMapRaised(info->disp, info->textwin);
        XMapRaised(info->disp, info->dbwin);
        XMapRaised(info->disp, info->mainwin);
    } else {
        fprintf(stderr, "failed to allocate x11 structure\n");

        exit(1);
    }

    return info;
}

void
zeusprocev(struct zeusx11 *x11)
{
    XEvent ev;

    XNextEvent(x11->disp, &ev);
    if (ev.xany.window == x11->mainwin) {
        switch (ev.type) {
            case Expose:
                /* IGNORE */

                break;
            default:
                
                break;
            }
    } else if (ev.xany.window == x11->simwin) {
        switch (ev.type) {
            case Expose:
                if (!ev.xexpose.count) {
                    XCopyArea(x11->disp, x11->pixbuf, x11->simwin,
                              x11->bggc,
                              0, 0,
                              x11->simw, x11->simh,
                              0, 0);
                }

                break;
            case ButtonPress:
                exit(0);

                /* NOTREACHED */
                break;
            default:
                
                break;
        }
    } else if (ev.xany.window == x11->textwin) {
        ;
    } else if (ev.xany.window == x11->dbwin) {
        ;
    }

    return;
}

void
zeusdrawdb(struct zeusx11 *x11, long ip)
{
    struct cwinstr *op = &cwoptab[ip];
    char           *line = zeusdisasm(op);
    long            nline = x11->ndbline;

    if (line) {
        XDrawString(x11->disp, x11->dbwin, x11->bggc,
                    0, nline * x11->fonth,
                    line, strlen(line));
//        nline++;
        free(line);
    }

    return;
}

void
zeusdrawsim(struct zeusx11 *x11)
{
    struct cwinstr *op;
    long            l;
    int             x;
    int             y;

    l = 0;
    XFillRectangle(x11->disp, x11->pixbuf,
                   x11->bggc,
                   0, 0,
                   x11->simh, x11->simh);
    for (y = 0 ; y < x11->simh ; y += 5) {
        for (x = 0 ; x < x11->simw ; x += 5) {
            op = &cwoptab[l];
            if (op->op == CWOPDAT) {
//                fprintf(stderr, "%ld: draw: (%d, %d)\n", l, x, y);
                if (!op->a && !op->b && !op->aflg && !op->bflg) {
                    XFillRectangle(x11->disp, x11->pixbuf,
                                   x11->datgc,
                                   x, y,
                                   4, 4);
                } else if (op->pid) {
                    XFillRectangle(x11->disp, x11->pixbuf,
                                   x11->prog2datgc,
                                   x, y,
                                   4, 4);
                } else {
                    XFillRectangle(x11->disp, x11->pixbuf,
                                   x11->prog1datgc,
                                   x, y,
                                   4, 4);
                }
            } else if (op->pid) {
                    XFillRectangle(x11->disp, x11->pixbuf,
                                   x11->prog2gc,
                                   x, y,
                                   4, 4);
                
            } else {
                    XFillRectangle(x11->disp, x11->pixbuf,
                                   x11->prog1gc,
                                   x, y,
                                   4, 4);
            }
            l++;
        }
    }
    XCopyArea(x11->disp, x11->pixbuf, x11->simwin,
              x11->bggc,
              0, 0,
              x11->simw, x11->simh,
              0, 0);
    XSync(x11->disp, False);

    return;
}

