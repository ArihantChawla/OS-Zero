#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <zpc/zpc.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include "x11.h"

extern struct zpcstkitem *zpcstktab[NSTKREG];
#define NHASHITEM 1024
static struct x11win     *winhash[NHASHITEM] ALIGNED(PAGESIZE);
static Window             buttonwintab[ZPC_NROW][ZPC_NCOLUMN];
static const char        *buttonstrtab[ZPC_NROW][ZPC_NCOLUMN]
= {
    { "d", "e", "f", "~", ">>>", "++" },
    { "a", "b", "c", "%", ">>", "--" },
    { "7", "8", "9", "/", "<<", "..>" },
    { "4", "5", "6", "*", "^", "<.." },
    { "1", "2", "3", "-", "|", NULL },
    { "0", ".", "SPC", "+", "&", "ENTER" }
};
#define BUTTONNORMAL  0
#define BUTTONHOVER   1
#define BUTTONCLICKED 2
#define NBUTTONSTATE  3
static Pixmap         buttonpmaps[NBUTTONSTATE];
static struct x11app *app;
static Window         mainwin;

void
x11addwin(struct x11win *win)
{
    long           key = win->id & (NHASHITEM - 1);

    win->next = winhash[key];
    winhash[key] = win;
}

struct x11win *
x11findwin(Window id)
{
    struct x11win *hashwin = NULL;
    long           key = id & (NHASHITEM - 1);

    hashwin = winhash[key];
    while ((hashwin) && hashwin->id != id) {
        hashwin = hashwin->next;
    }

    return hashwin;
}

void
x11initpmaps(void)
{
    buttonpmaps[BUTTONNORMAL] = imlib2loadimage("mosaic.png",
                                                ZPC_BUTTON_WIDTH,
                                                ZPC_BUTTON_HEIGHT);
    buttonpmaps[BUTTONHOVER] = imlib2loadimage("gaussian.png",
                                               ZPC_BUTTON_WIDTH,
                                               ZPC_BUTTON_HEIGHT);
#if 0
    buttonpmaps[BUTTONCLICKED] = imlib2loadimage("button_clicked.png",
                                                 ZPC_BUTTON_WIDTH,
                                                 ZPC_BUTTON_HEIGHT);
#endif
    fprintf(stderr, "PIXMAPS: %x, %x, %x\n",
            (int)buttonpmaps[BUTTONNORMAL],
            (int)buttonpmaps[BUTTONHOVER],
            (int)buttonpmaps[BUTTONCLICKED]);

    return;
}

struct x11app *
x11initapp(char *displayname)
{
    struct x11app *app;
    Display       *disp;

    app = calloc(1, sizeof(struct x11app));
    XInitThreads();
    disp = XOpenDisplay(displayname);
    if (!disp) {
        fprintf(stderr, "failed to open display\n");

        exit(1);
    }
    app->display = disp;
    app->screen = DefaultScreen(disp);
    app->depth = DefaultDepth(disp, app->screen);
    app->visual = DefaultVisual(disp, app->screen);
    app->colormap = DefaultColormap(disp, app->screen);

    return app;
}

Window
x11initwin(struct x11app *app, Window parent, int x, int y, int w, int h)
{
    XSetWindowAttributes atr;
    Window               win;

    if (!parent) {
        parent = RootWindow(app->display, app->screen);
    }
    memset(&atr, 0, sizeof(atr));
    atr.background_pixel = WhitePixel(app->display, app->screen);
    win = XCreateWindow(app->display,
                        parent,
                        x,
                        y,
                        w,
                        h,
                        0,
                        app->depth,
                        InputOutput,
                        app->visual,
                        CWBackPixel,
                        &atr);
    XMapRaised(app->display, win);

    return win;
}

void
buttonexpose(void *arg, XEvent *event)
{
    struct x11win *win = arg;

#if 0
    if (!event->xexpose.count) {
        XSetWindowBackgroundPixmap(app->display, win->id,
                                   buttonpmaps[BUTTONNORMAL]);
        XClearWindow(app->display, win->id);
        XSync(app->display, False);
    }
#endif
    XClearWindow(app->display, win->id);
    XSync(app->display, False);

    return;
}

#if 0
void
buttonenter(void *arg, XEvent *event)
{
    struct x11win *win = arg;

    if (!event->xexpose.count) {
        XSetWindowBackgroundPixmap(app->display, win->id,
                                   buttonpmaps[BUTTONHOVER]);
        XClearWindow(app->display, win->id);
        XSync(app->display, False);
    }

    return;
}

void
buttonleave(void *arg, XEvent *event)
{
    struct x11win *win = arg;

    if (!event->xexpose.count) {
        XSetWindowBackgroundPixmap(app->display, win->id,
                                   buttonpmaps[BUTTONNORMAL]);
        XClearWindow(app->display, win->id);
        XSync(app->display, False);
    }

    return;
}
#endif

void
buttonpress(void *arg, XEvent *event)
{
    struct x11win *win = arg;
    int            evbut = toevbutton(event->xbutton.button);
    copfunc_t     *func;
    int64_t       *src = &zpcstktab[1]->data.i64;
    int64_t       *dest = &zpcstktab[0]->data.i64;

#if 0
    XSetWindowBackgroundPixmap(app->display, win->id,
                               buttonpmaps[BUTTONCLICKED]);
    XClearWindow(app->display, win->id);
    XSync(app->display, False);
#endif
    if (evbut < NBUTTON) {
        func = win->clickfunc[evbut];
        if (func) {
            func(src, dest, &zpcstktab[0]->data.i64);
        }
    }
}

#if 0
void
buttonrelease(void *arg, XEvent *event)
{
    struct x11win *win = arg;
    int            evbut = toevbutton(event->xbutton.button);
    copfunc_t     *func;
    int64_t       *src = &zpcstktab[1]->data.i64;
    int64_t       *dest = &zpcstktab[0]->data.i64;

    XSetWindowBackgroundPixmap(app->display, win->id,
                               buttonpmaps[BUTTONNORMAL]);
    XClearWindow(app->display, win->id);
    XSync(app->display, False);
}
#endif

void
x11init(void)
{
    Window         win;
    int            row;
    int            col;
    struct x11win *wininfo;

    app = x11initapp(NULL);
    mainwin = x11initwin(app, 0, 0, 0, ZPC_WINDOW_WIDTH, ZPC_WINDOW_HEIGHT);
    app->win = mainwin;
    imlib2init(app);
    x11initpmaps();
    for (row = 0 ; row < ZPC_NROW ; row++) {
        for (col = 0 ; col < ZPC_NCOLUMN ; col++) {
            win = x11initwin(app,
                             mainwin,
                             col * ZPC_BUTTON_WIDTH,
                             row * ZPC_BUTTON_HEIGHT,
                             ZPC_BUTTON_WIDTH,
                             ZPC_BUTTON_HEIGHT);
            if (win) {
                wininfo = calloc(1, sizeof(struct x11win));
                wininfo->id = win;
                wininfo->str = buttonstrtab[row][col];
//                wininfo->evfunc[EnterNotify] = buttonenter;
//                wininfo->evfunc[LeaveNotify] = buttonleave;
                wininfo->evfunc[ButtonPress] = buttonpress;
                wininfo->evfunc[Expose] = buttonexpose;
                buttonwintab[row][col] = win;
                x11addwin(wininfo);
                XSetWindowBackgroundPixmap(app->display, win,
                                           buttonpmaps[BUTTONNORMAL]);
                XClearWindow(app->display, win);
                XSync(app->display, False);
                XSelectInput(app->display, win,
                             ExposureMask | ButtonPressMask);
                XMapRaised(app->display, win);
            } else {
                fprintf(stderr, "failed to create window\n");

                exit(1);
            }
        }
    }
}

void
x11nextevent(void)
{
    XEvent         event;
    struct x11win *win;
    x11evfunc_t   *func;

    XNextEvent(app->display, &event);
    win = x11findwin(event.xany.window);
    fprintf(stderr, "EVENT: %d - ", event.type);
    if (win) {
        func = win->evfunc[event.type];
        if (func) {
            fprintf(stderr, "HANDLED\n");
            func(win, &event);
        } else {
            fprintf(stderr, "NOT HANDLED\n");
        }
    }
}
