#include <stdlib.h>
#include <stdio.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <X11/Xutil.h>
#include <corewar/x11.h>

#include <dungeon/cell.h>

struct cellx11 {
    long                   candraw;
    Display               *disp;
    Pixmap                 pixbuf;
    Window                 mainwin;
    GC                     cavegc;
    GC                     corgc;
    int                    depth;
    Colormap               colormap;
    Visual                *visual;
    int                    screen;
    int                    w;
    int                    h;
};

static struct cellx11  cellx11;
static struct celldng *testdng;

void
cellbuttonpress(void)
{
    exit(1);
}

void
cellinitx11(struct cellx11 *x11)
{
    Display *disp;

    XInitThreads();
    disp = XOpenDisplay(NULL);
    if (!disp) {
        fprintf(stderr, "failed to open display\n");
        
        exit(1);
    }
    x11->disp = disp;
    x11->screen = DefaultScreen(disp);
    x11->colormap = DefaultColormap(disp, x11->screen);
    x11->depth = DefaultDepth(disp, x11->screen);
    x11->visual = DefaultVisual(disp, x11->screen);

    return;
}

void
cellinitx11win(struct cellx11 *x11)
{
    XSetWindowAttributes atr = { 0 };
    Window               win;
    Window               parent = RootWindow(x11->disp, x11->screen);

    atr.background_pixel = BlackPixel(x11->disp, x11->screen);
    win = XCreateWindow(x11->disp,
                        parent,
                        0, 0,
                        1024, 768,
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
    x11->w = 1024;
    x11->h = 768;
    XMapRaised(x11->disp, win);

    return;
}

void
cellinitx11ev(struct cellx11 *x11)
{
    XSelectInput(x11->disp, x11->mainwin, ExposureMask | ButtonPressMask);

    return;
}

void
cellinitx11gc(struct cellx11 *x11)
{
    XGCValues gcval;
    GC        gc;
    XColor    color;

    gcval.foreground = WhitePixel(x11->disp, DefaultScreen(x11->disp));
    gcval.graphics_exposures = False;
    gc = XCreateGC(x11->disp,
                   x11->mainwin,
                   GCForeground,
                   &gcval);
    if (!gc) {
        fprintf(stderr, "failed to create GC\n");
        
        exit(1);
    }
    x11->cavegc = gc;
    if (!XParseColor(x11->disp,
                     x11->colormap,
                     "green",
                     &color)) {
        fprintf(stderr, "failed to parse color\n");

        exit(1);
    }
    gcval.foreground = color.pixel;
    gc = XCreateGC(x11->disp,
                   x11->mainwin,
                   GCForeground,
                   &gcval);
    if (!gc) {
        fprintf(stderr, "failed to create GC\n");

        exit(1);
    }
    x11->corgc = gc;

    return;
}

void
celldrawx11win(XEvent *ev)
{
    struct cellcave *cave;
    void            *map;
    long             id;
    long              y;
    long              x;
    long              w = testdng->width;

    for (id = 0 ; id < testdng->ncave ; id++) {
        cave = testdng->cavetab[id];
        map = cave->map;
        for (y = 0 ; y < 768 / 8 ; y++) {
            for (x = 0 ; x < 1024 / 8 ; x++) {
                if (bitset(map, y * w + x)) {
#if 0
                    XDrawPoint(cellx11.disp,
                               cellx11.mainwin,
                               cellx11.cavegc,
                               x, y);
#endif
                    XDrawRectangle(cellx11.disp,
                                   cellx11.mainwin,
                                   cellx11.cavegc,
                                   x * 8, y * 8,
                                   8, 8);
                }
            }
        }
    }

    return;
}

int
cellx11loop(struct celldng *dng)
{
    XEvent ev;

    testdng = dng;
    cellinitx11(&cellx11);
    cellinitx11win(&cellx11);
    cellinitx11ev(&cellx11);
    cellinitx11gc(&cellx11);
    while (1) {
        XNextEvent(cellx11.disp, &ev);
        switch (ev.type) {
            case Expose:
                cellx11.candraw = 1;
                celldrawx11win(&ev);

                break;
            case ButtonPress:

                exit(0);
            default:

                break;
        }
    }

    exit(0);
}

