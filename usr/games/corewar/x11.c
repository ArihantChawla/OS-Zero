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
#define ZEUSSIMW     200
#define ZEUSSIMH     40

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
        x11->fontasc = font->ascent;
    } else {
        fprintf(stderr, "failed to load font\n");

        exit(1);
    }
    fprintf(stderr, "FONT: h == %d, asc == %d, desc == %d\n",
            x11->fonth, x11->fontasc, font->descent);

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
#if (ZEUSHOVERTOOLTIP) || (ZEUSCLICKTOOLTIP)
    int                  winh = x11->fonth + (ZEUSTEXTNROW + ZEUSDBNROW) * x11->fonth + ZEUSSIMH * 5;
#else
    int                  winh = (ZEUSTEXTNROW + ZEUSDBNROW) * x11->fonth + ZEUSSIMH * 5;
#endif
    long                 x = 0;
    long                 y = 0;

    atr.background_pixel = BlackPixel(x11->disp, x11->screen);
#if (ZEUSHOVERTOOLTIP) || (ZEUSCLICKTOOLTIP)
//    atr.override_redirect = True;
    atr.override_redirect = False;
    win = XCreateWindow(x11->disp,
                        parent,
                        0, 0,
                        x11->fontw * 80, x11->fonth,
                        0,
                        x11->depth,
                        InputOutput,
                        x11->visual,
                        CWBackPixel | CWOverrideRedirect,
                        &atr);
    if (!win) {
        fprintf(stderr, "failed to create tooltip window\n");

        exit(1);
    }
    x11->tipwin = win;
    y += x11->fonth;
#endif
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
    gcval.foreground = WhitePixel(x11->disp, DefaultScreen(x11->disp));
    gc = XCreateGC(x11->disp, x11->textwin,
                   GCForeground | GCFunction | GCLineWidth | GCFont,
                   &gcval);
    if (!gc) {
        fprintf(stderr, "failed to create GC\n");

        exit(1);
    }
    x11->textgc = gc;
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
        XSelectInput(info->disp, info->tipwin, ExposureMask);
        XSelectInput(info->disp, info->simwin,
                     KeyPressMask
                     | ButtonPressMask
                     | EnterWindowMask
                     | LeaveWindowMask
                     | PointerMotionMask
//                     | PointerMotionHintMask
                     | ExposureMask);
        XMapRaised(info->disp, info->mainwin);
        XMapRaised(info->disp, info->tipwin);
        XMapRaised(info->disp, info->simwin);
        XMapRaised(info->disp, info->textwin);
        XMapRaised(info->disp, info->dbwin);
    } else {
        fprintf(stderr, "failed to allocate x11 structure\n");

        exit(1);
    }

    return info;
}

void
zeusprinttip(struct zeusx11 *x11, int simx, int simy)
{
    int ndx;

    simx /= 5;
    simy /= 5;
    ndx = simy * (x11->simw / 5) + simx;
    if (x11->tipstr) {
        free(x11->tipstr);
        x11->tipstr = NULL;
    }
    x11->tipstr = zeusdisasm(&cwoptab[ndx], &x11->tiplen);
    x11->tiplen = strlen(x11->tipstr);
    fprintf(stderr, "DRAW(%d @ (%d, %d)): %s\n", x11->tiplen, 0, x11->fontasc, x11->tipstr);
    XDrawString(x11->disp, x11->tipwin, x11->textgc,
                0, x11->fontasc,
                x11->tipstr, x11->tiplen);
//    XMapRaised(x11->disp, x11->tipwin);

    return;
}

void
zeusprintdb(struct zeusx11 *x11, int simx, int simy, int x, int y)
{
    int ndx;

    simx /= 5;
    simy /= 5;
    ndx = simy * (x11->simw / 5) + simx;
    if (x11->tipstr) {
        free(x11->tipstr);
        x11->tipstr = NULL;
    }
    x11->tipstr = zeusdisasm(&cwoptab[ndx], &x11->tiplen);
    fprintf(stderr, "DRAW(%d): %s\n", x11->tiplen, x11->tipstr);
    XDrawString(x11->disp, x11->tipwin, x11->textgc,
                x, y,
                x11->tipstr, x11->tiplen);
//    XMapRaised(x11->disp, x11->tipwin);

    return;
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
#if (ZEUSHOVERTOOLTIP)
            case MotionNotify:
                zeusprinttip(x11, ev.xmotion.x, ev.xmotion.y);

                break;
            case EnterNotify:
                zeusprinttip(x11, ev.xcrossing.x, ev.xcrossing.y);

                break;
            case LeaveNotify:
                XUnmapWindow(x11->disp, x11->tipwin);

                break;
#endif
#if (ZEUSCLICKTOOLTIP)
            case ButtonPress:
                zeusprinttip(x11, ev.xbutton.x, ev.xbutton.y);

                break;
#else
            case ButtonPress:
                exit(0);

                /* NOTREACHED */
                break;
#endif
            case Expose:
                if (!ev.xexpose.count) {
                    XCopyArea(x11->disp, x11->pixbuf, x11->simwin,
                              x11->bggc,
                              0, 0,
                              x11->simw, x11->simh,
                              0, 0);
                }

                break;
            case KeyPress:
                exit(0);

                /* NOTREACHED */
                break;
            default:
                
                break;
        }
#if (ZEUSHOVERTOOLTIP)
    } else if (ev.xany.window == x11->tipwin) {
        switch (ev.type) {
            case Expose:
                if (!ev.xexpose.count) {
                    fprintf(stderr, "DRAW(%d): %s\n", x11->tiplen, x11->tipstr);
                    XDrawString(x11->disp, x11->tipwin, x11->textgc,
                                0, x11->fontasc,
                                x11->tipstr, x11->tiplen);
                }

                break;
            default:

                break;
        }
#endif
    } else if (ev.xany.window == x11->textwin) {
        ;
    } else if (ev.xany.window == x11->dbwin) {
        ;
    }
//    XSync(x11->disp, False);

    return;
}

void
zeusdrawdb(struct zeusx11 *x11, long ip)
{
    struct cwinstr *op = &cwoptab[ip];
    int             len;
    char           *line = zeusdisasm(op, &len);
    long            nline = x11->ndbline;

    if ((line) && (len)) {
        XDrawString(x11->disp, x11->dbwin, x11->bggc,
                    0, nline * x11->fonth,
                    line, len);
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

