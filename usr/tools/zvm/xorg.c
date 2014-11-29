#include <zvm/conf.h>

#if (ZVMXORG)

#include <stdio.h>
#include <stdlib.h>
#if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200112L)
#include <sys/select.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#include <zvm/xorg.h>
#include <Imlib2.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <X11/Xutil.h>

static struct zvmxorgui  xorgui;
static const char       *xorgbuttonnames[]
= {
    "print",
    "step",
    "break",
    "disasm",
    "show",
    NULL
};
static const char       *xorgbuttonimgnames[XORGNBUTTONSTATE]
= {
    "../../share/img/menunormal.png",
    "../../share/img/menuhilite.png",
    "../../share/img/menupress.png"
};
Imlib_Image             *xorgbuttonimgs[XORGNBUTTONSTATE];
Pixmap                   xorgbuttonpmaps[XORGNBUTTONSTATE];
Window                   xorgbuttonwins[64];
Pixmap                   xorgbuttontexts[64];
void
zvminituiwins(void)
{
    XSetWindowAttributes atr;
    int                  screen = DefaultScreen(xorgui.disp);
    Window               win;
    int                  i;
    Pixmap               pmap;

    atr.background_pixel = BlackPixel(xorgui.disp, screen);
//    atr.foreground_pixel = WhitePixel(xorgui.disp, screen);
    atr.colormap = DefaultColormap(xorgui.disp, screen);
    atr.save_under = True;
    atr.backing_store = True;
    atr.override_redirect = False;
    win = XCreateWindow(xorgui.disp,
                        RootWindow(xorgui.disp, screen),
                        0, 0,
                        640, 480,
                        0,
                        DefaultDepth(xorgui.disp, screen),
                        InputOutput,
                        DefaultVisual(xorgui.disp, screen),
                        CWBackPixel
                        | CWColormap
                        | CWSaveUnder
                        | CWBackingStore
                        | CWOverrideRedirect,
                        &atr);
    if (!win) {
        fprintf(stderr, "ERROR: failed to create UI window\n");

        exit(1);
    }
    xorgui.uiwin = win;
    win = XCreateWindow(xorgui.disp,
                        win,
                        0, 0,
                        640, 480,
                        0,
                        DefaultDepth(xorgui.disp, screen),
                        InputOutput,
                        DefaultVisual(xorgui.disp, screen),
                        CWBackPixel
                        | CWColormap
                        | CWSaveUnder
                        | CWBackingStore
                        | CWOverrideRedirect,
                        &atr);
    if (!win) {
        fprintf(stderr, "ERROR: failed to create DB window\n");

        exit(1);
    }
    xorgui.dbwin = win;
    for ( i = 0 ; (xorgbuttonnames[i]) ; i++) {
        win = XCreateWindow(xorgui.disp,
                            xorgui.dbwin,
                            i * 128, 0,
                            128, 24,
                            0,
                            DefaultDepth(xorgui.disp, screen),
                            InputOutput,
                            DefaultVisual(xorgui.disp, screen),
                            CWBackPixel
                            | CWColormap
                            | CWSaveUnder
                            | CWBackingStore
                            | CWOverrideRedirect,
                            &atr);
        if (!win) {
            fprintf(stderr, "ERROR: failed to create %s window\n",
                    xorgbuttonnames[i]);
            
            exit(1);
        }
        xorgbuttonwins[i] = win;
        XSelectInput(xorgui.disp, win,
                     ButtonPressMask
                     | ButtonReleaseMask
                     | EnterWindowMask
                     | LeaveWindowMask);
        XMapWindow(xorgui.disp, win);
    }
#if 0
    win = XCreateWindow(xorgui.disp,
                        xorgui.dbwin,
                        0, 24,
                        640, 5,
                        0,
                        DefaultDepth(xorgui.disp, screen),
                        InputOutput,
                        DefaultVisual(xorgui.disp, screen),
                        CWBackPixel
                        | CWColormap
                        | CWSaveUnder
                        | CWBackingStore
                        | CWOverrideRedirect,
                        &atr);
    if (!win) {
        fprintf(stderr, "ERROR: failed to create ruler window\n");
                
        exit(1);
    }
    xorgui.hrulerwin = win;
    XSetWindowBackgroundPixmap(xorgui.disp, win, xorgui.hrulerpmap);
    XMapWindow(xorgui.disp, win);
#endif
    XMapWindow(xorgui.disp, xorgui.uiwin);
    XMapWindow(xorgui.disp, xorgui.dbwin);

    return;
}

XFontStruct *
zvminituifonts(void)
{
    XFontStruct *font;
    FT_Error     error;
    FT_Face      face;

    error = FT_Init_FreeType(&xorgui.ftlib);
    if (error) {
        fprintf(stderr, "ERROR: failed to initialize freetype2\n");

        exit(1);
    }
    error = FT_New_Face(xorgui.ftlib,
                        "../../share/fonts/bladerunner.ttf",
                        0,
                        &face);
    if (error) {
        fprintf(stderr, "ERROR: failed to load bladerunner font\n");

        exit(1);
    }
    font = XLoadQueryFont(xorgui.disp, ZVMTEXTFONT);
    if (!font) {
        fprintf(stderr, "ERROR: failed to load font %s\n", ZVMTEXTFONT);

        exit(1);
    }
    xorgui.textfont = font;
    xorgui.fontw = font->max_bounds.width;
    xorgui.fonth = font->ascent + font->descent;

    return font;
}

void
zvminituigcs(void)
{
    XGCValues gcval;
    XColor    color;

    gcval.foreground = WhitePixel(xorgui.disp, DefaultScreen(xorgui.disp));
    gcval.background = BlackPixel(xorgui.disp, DefaultScreen(xorgui.disp));
    gcval.font = xorgui.textfont->fid;
    xorgui.uigc = XCreateGC(xorgui.disp,
                            xorgui.uiwin,
                            GCForeground | GCBackground | GCFont,
                            &gcval);
    xorgui.dbgc = XCreateGC(xorgui.disp,
                            xorgui.uiwin,
                            GCForeground | GCBackground | GCFont,
                            &gcval);
}

void
zvminituiimgs(void)
{
    Imlib_Image *img;
    int          i;
    Pixmap       pmap;
    Pixmap       mask;

    imlib_context_set_display(xorgui.disp);
    imlib_context_set_drawable(xorgui.uiwin);
    imlib_context_set_visual(DefaultVisual(xorgui.disp,
                                           DefaultScreen(xorgui.disp)));
    imlib_context_set_colormap(DefaultColormap(xorgui.disp,
                                               DefaultScreen(xorgui.disp)));
    img = imlib_load_image(ZVMHRULER);
    if (!img) {
        fprintf(stderr, "ERROR: failed to load horizontal ruler image\n");

        exit(1);
    }
    xorgui.hruler = img;
    imlib_context_set_image(img);
    xorgui.hrulerh = imlib_image_get_height();
    img = imlib_load_image(ZVMVRULER);
    if (!img) {
        fprintf(stderr, "ERROR: failed to load vertical ruler image\n");

        exit(1);
    }
    xorgui.vruler = img;
    imlib_context_set_image(img);
    xorgui.vrulerw = imlib_image_get_width();
#if 0
    imlib_context_set_drawable(xorgui.hrulerwin);
    imlib_render_pixmaps_for_whole_image_at_size(&xorgui.hrulerpmap,
                                                 NULL,
                                                 xorgui.vrulerw,
                                                 imlib_image_get_height());
    XSetWindowBackgroundPixmap(xorgui.disp,
                               xorgui.hrulerwin, xorgui.hrulerpmap);
    XClearWindow(xorgui.disp, xorgui.hrulerwin);
    XMapWindow(xorgui.disp, xorgui.hrulerwin);
#endif
    for ( i = 0; i < XORGNBUTTONSTATE ; i++ ) {
        img = imlib_load_image(xorgbuttonimgnames[i]);
        xorgbuttonimgs[i] = img;
        imlib_context_set_image(img);
        imlib_render_pixmaps_for_whole_image_at_size(&pmap,
                                                     &mask,
                                                     128, 24);
        xorgbuttonpmaps[i] = pmap;
    }
    for ( i = 0 ; (xorgbuttonnames[i]) ; i++) {
        XSetWindowBackgroundPixmap(xorgui.disp,
                                   xorgbuttonwins[i],
                                   xorgbuttonpmaps[XORGBUTTONNORMAL]);
        XClearWindow(xorgui.disp, xorgbuttonwins[i]);
    }

    return;
}

void
zvminitui(void)
{
    Display *disp = XOpenDisplay(NULL);

    if (!disp) {
        fprintf(stderr, "ERROR: failed to open display\n");

        exit(1);
    }
    xorgui.disp = disp;
    xorgui.connfd = XConnectionNumber(disp);
    zvminituifonts();
    zvminituiwins();
    zvminituigcs();
    zvminituiimgs();

    return;
}

void
zvmdrawuiwin(void)
{
    ;
}

void
zvmdouievent(void)
{
    XEvent         ev;
    char           str[64];
    KeySym         keysym;
    XComposeStatus compose;
    int            len;

    if (XPending(xorgui.disp)) {
        XNextEvent(xorgui.disp, &ev);
        if (ev.xany.window == xorgui.uiwin) {
            switch (ev.type) {
                case Expose:
                    zvmdrawuiwin();
                    
                    break;
                case KeyRelease:
                    len = XLookupString(&ev.xkey, str,
                                        64,
                                        &keysym,
                                        &compose);
            }
        } else if (ev.xany.window == xorgui.dbwin) {
            /* debug window */
        } else {
            switch (ev.type) {
                case ButtonPress:
                    XSetWindowBackgroundPixmap(xorgui.disp,
                                               ev.xany.window,
                                               xorgbuttonpmaps[XORGBUTTONPRESS]);
                    XClearWindow(xorgui.disp,
                                 ev.xany.window);
                    XFlush(xorgui.disp);

                    break;
                case ButtonRelease:
                    XSetWindowBackgroundPixmap(xorgui.disp,
                                               ev.xany.window,
                                               xorgbuttonpmaps[XORGBUTTONHOVER]);
                    XClearWindow(xorgui.disp,
                                 ev.xany.window);
                    XFlush(xorgui.disp);

                    break;
                case EnterNotify:
                    XSetWindowBackgroundPixmap(xorgui.disp,
                                               ev.xany.window,
                                               xorgbuttonpmaps[XORGBUTTONHOVER]);
                    XClearWindow(xorgui.disp,
                                 ev.xany.window);
                    XFlush(xorgui.disp);

                    break;
                case LeaveNotify:
                    XSetWindowBackgroundPixmap(xorgui.disp,
                                               ev.xany.window,
                                               xorgbuttonpmaps[XORGBUTTONNORMAL]);
                    XClearWindow(xorgui.disp,
                                 ev.xany.window);
                    XFlush(xorgui.disp);

                    break;
                case Expose:
                    XClearWindow(xorgui.disp, ev.xany.window);
                    XFlush(xorgui.disp);

                    break;
            }
        }
#if 0
    } else {
        fd_set fdset;

        FD_ZERO(&fdset);
        FD_SET(xorgui.connfd, &fdset);
        select(xorgui.connfd + 1, &fdset, NULL, NULL, NULL);
#endif
    }

    return;
}

#endif

