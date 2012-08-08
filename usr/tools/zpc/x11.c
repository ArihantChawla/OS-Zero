#if (ZPCX11)

/* TODO: add buttons '<', '>', and 'u' */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <zpc/zpc.h>
#include <zpc/op.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include "x11.h"

#define ZPC_TITLE "Zero Programmer's Calculator"

void imlib2init(struct x11app *app);
Pixmap imlib2loadimage(struct x11app *app, const char *filename, int w, int h);

void stkenterinput(void);
void stkqueueinput(const char *str);

extern struct zpctoken   *zpcregstk[];
extern struct zpcstkitem *zpcinputitem;

#define NHASHITEM 1024
static struct x11win     *winhash[NHASHITEM] ALIGNED(PAGESIZE);
static zpccfunc_t        *buttonopertab[ZPC_NROW][ZPC_NCOLUMN]
= {
    { NULL, NULL, NULL, not64, shr64, inc64, NULL, NULL },
    { NULL, NULL, NULL, mod64, shrl64, dec64, NULL, NULL },
    { NULL, NULL, NULL, div64, shl64, ror64, NULL, NULL },
    { NULL, NULL, NULL, mul64, xor64, rol64, NULL, NULL },
    { NULL, NULL, NULL, sub64, or64, NULL, NULL, NULL },
    { NULL, NULL, NULL, add64, and64, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};
static zpccfunc_t        *buttonopertabflt[ZPC_NROW][ZPC_NCOLUMN]
= {
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, fdiv32, NULL, NULL, NULL },
    { NULL, NULL, NULL, mul32, NULL, NULL, NULL },
    { NULL, NULL, NULL, fsub32, NULL, NULL, NULL },
    { NULL, NULL, NULL, fadd32, NULL, NULL, NULL},
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};
static zpccfunc_t        *buttonopertabdbl[ZPC_NROW][ZPC_NCOLUMN]
= {
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, fdiv64, NULL, NULL, NULL },
    { NULL, NULL, NULL, mul64, NULL, NULL, NULL },
    { NULL, NULL, NULL, fsub64, NULL, NULL, NULL },
    { NULL, NULL, NULL, fadd64, NULL, NULL, NULL},
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};
static Window             buttonwintab[ZPC_NROW][ZPC_NCOLUMN];
static uint8_t            buttontypetab[ZPC_NROW][ZPC_NCOLUMN] =
{
    { ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONUTIL, ZPCBUTTONUTIL },
    { ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONUTIL, ZPCBUTTONUTIL },
    { ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONUTIL, ZPCBUTTONUTIL },
    { ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONUTIL, ZPCBUTTONUTIL },
    { ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONUTIL, ZPCBUTTONUTIL },
    { ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONUTIL, ZPCBUTTONUTIL },
    { ZPCBUTTONREG, ZPCBUTTONREG, ZPCBUTTONREG, ZPCBUTTONREG, ZPCBUTTONREG, ZPCBUTTONREG, ZPCBUTTONREG, ZPCBUTTONREG },
    { ZPCBUTTONREG, ZPCBUTTONREG, ZPCBUTTONREG, ZPCBUTTONREG, ZPCBUTTONREG, ZPCBUTTONREG, ZPCBUTTONREG, ZPCBUTTONREG }
};
static const char        *buttonstrtab[ZPC_NROW][ZPC_NCOLUMN]
= {
    { "d", "e", "f", "~", ">>>", "++", "(", "CLR" },
    { "a", "b", "c", "%", ">>", "--", ")", "COPY" },
    { "7", "8", "9", "/", "<<", "..>", "0x", "POP" },
    { "4", "5", "6", "*", "^", "<..", "0b", "DUP" },
    { "1", "2", "3", "-", "|", "=", "f", "DEL" },
    { "0", ".", ",", "+", "&", NULL, " ", NULL },
    { "%st0", "%st1", "%st2", "%st3", "%st4", "%st5", "%st6", "%st7" },
    { "%st8", "%st9", "%st10", "%st11", "%st12", "%st13", "%st14", "%st15" }
};
static const char        *buttonlabeltab[ZPC_NROW][ZPC_NCOLUMN]
= {
    { "d", "e", "f", "~", ">>>", "++", "(", "CLR" },
    { "a", "b", "c", "%", ">>", "--", ")", "COPY" },
    { "7", "8", "9", "/", "<<", "..>", "0x", "POP" },
    { "4", "5", "6", "*", "^", "<..", "0b", "DUP" },
    { "1", "2", "3", "-", "|", "=", "f", "DEL" },
    { "0", ".", ",", "+", "&", "EVAL", "SPACE", "ENTER" },
    { " %st0", " %st1", " %st2", " %st3", " %st4", " %st5", " %st6", " %st7" },
    { " %st8", " %st9", "%st10", "%st11", "%st12", "%st13", "%st14", "%st15" }
};
static const char        *buttonasmtab[ZPC_NROW][ZPC_NCOLUMN]
= {
    { NULL, NULL, NULL, "not", "shra", "inc", NULL, NULL },
    { NULL, NULL, NULL, "mod", "shr", "dec", NULL, NULL },
    { NULL, NULL, NULL, "div", "shl", "ror", NULL, NULL },
    { NULL, NULL, NULL, "mul", "xor", "rol", NULL, NULL },
    { NULL, NULL, NULL, "sub", "or", NULL, NULL, NULL },
    { NULL, NULL, NULL, "add", "and", NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};
static const char        *buttonfltasmtab[ZPC_NROW][ZPC_NCOLUMN]
= {
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, "fdiv", NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, "fmul", NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, "fsub", NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, "fadd", NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};
#define ENTER 0xff
#define EVAL  0xfe
#define CLR   0xfd
#define COPY  0xfc
#define POP   0xfb
#define DUP   0xfa
#define DEL   0xef
static const uint8_t      parmtab[ZPC_NROW][ZPC_NCOLUMN]
= {
    { 0, 0, 0, 1, 2, 1, 0, CLR },
    { 0, 0, 0, 2, 2, 1, 0, COPY },
    { 0, 0, 0, 2, 2, 2, 0, POP },
    { 0, 0, 0, 2, 2, 2, 0, DUP },
    { 0, 0, 0, 2, 2, 0, 0, DEL },
    { 0, 0, 0, 2, 2, EVAL, 0, ENTER },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }
};
static const uint8_t      isflttab[ZPC_NROW][ZPC_NCOLUMN]
= {
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }
};
#define BUTTONNORMAL  0
#define BUTTONHOVER   1
#define BUTTONCLICKED 2
#define NBUTTONSTATE  3
static Window         stkwintab[NREGSTK];
static Pixmap         buttonpmaps[NBUTTONSTATE];
XFontStruct          *font;
int                   fontw;
int                   fonth;
static struct x11app *app;
static Window         mainwin;
static Window         inputwin;
GC                    textgc;
GC                    asmtextgc;
GC                    numtextgc;
GC                    optextgc;
GC                    utiltextgc;

void
x11drawdisp(void)
{
    struct zpctoken   *token;
    struct zpcstkitem *item;
    int                i;
    int                len;
    Window             win;
    int                x;

    i = NSTKREG;
    while (i--) {
        token = zpcregstk[i];
        win = stkwintab[i];
        x = 8;
        XClearWindow(app->display, win);
        while ((token) && (token->str)) {
            len = strlen(token->str);
            XDrawString(app->display, win, textgc,
                        x,
                        (fonth + 8) >> 1,
                        token->str, len);
            x += len * fonth;
            token = token->next;
        }
    }
    x = 8;
    item = zpcinputitem;
    len = strlen(item->str);
    win = inputwin;
    XClearWindow(app->display, win);
    if (item->scur != item->str) {
        XDrawString(app->display, win, textgc,
                    x,
                    (fonth + 8) >> 1,
                    item->str, len);
    }

    return;
}

void x11inittitle(void)
{
    XTextProperty titleprop;
    char          *str = ZPC_TITLE;

    XStringListToTextProperty(&str, 1, &titleprop);
    XSetWMName(app->display, mainwin, &titleprop);
    XFree(titleprop.value);

    return;
}

void
x11initgcs(void)
{
    XGCValues gcval;
    XColor    color;

#if (ZPCREVERSE)
    gcval.foreground = WhitePixel(app->display, DefaultScreen(app->display));
#else
    gcval.foreground = BlackPixel(app->display, DefaultScreen(app->display));
#endif
    gcval.font = font->fid;
    gcval.graphics_exposures = False;
    textgc = XCreateGC(app->display, app->win,
                       GCForeground | GCFont | GCGraphicsExposures,
                       &gcval);
    if (!textgc) {
        fprintf(stderr, "failed to create GC\n");

        exit(1);
    }
    numtextgc = XCreateGC(app->display, app->win,
                          GCForeground | GCFont | GCGraphicsExposures,
                          &gcval);
    if (!numtextgc) {
        fprintf(stderr, "failed to create GC\n");
        
        exit(1);
    }
    if (!XParseColor(app->display,
                     app->colormap,
                     "orange",
                     &color)) {
        fprintf(stderr, "failed to parse color 'orange'\n");

        exit(1);
    }
    if (!XAllocColor(app->display,
                     app->colormap,
                     &color)) {
        fprintf(stderr, "failed to parse color 'orange'\n");

        exit(1);
    }
    gcval.foreground = color.pixel;
    asmtextgc = XCreateGC(app->display, app->win,
                       GCForeground | GCFont | GCGraphicsExposures,
                       &gcval);
    if (!asmtextgc) {
        fprintf(stderr, "failed to create GC\n");

        exit(1);
    }
    if (!XParseColor(app->display,
                     app->colormap,
                     "green",
                     &color)) {
        fprintf(stderr, "failed to parse color 'green'\n");

        exit(1);
    }
    if (!XAllocColor(app->display,
                     app->colormap,
                     &color)) {
        fprintf(stderr, "failed to parse color 'green'\n");

        exit(1);
    }
    gcval.foreground = color.pixel;
    optextgc = XCreateGC(app->display, app->win,
                         GCForeground | GCFont | GCGraphicsExposures,
                         &gcval);
    if (!optextgc) {
        fprintf(stderr, "failed to create GC\n");

        exit(1);
    }
    if (!XParseColor(app->display,
                     app->colormap,
                     "light blue",
                     &color)) {
        fprintf(stderr, "failed to parse color 'light blue'\n");

        exit(1);
    }
    if (!XAllocColor(app->display,
                     app->colormap,
                     &color)) {
        fprintf(stderr, "failed to parse color 'light blue'\n");

        exit(1);
    }
    gcval.foreground = color.pixel;
    utiltextgc = XCreateGC(app->display, app->win,
                          GCForeground | GCFont | GCGraphicsExposures,
                          &gcval);
    if (!utiltextgc) {
        fprintf(stderr, "failed to create GC\n");

        exit(1);
    }

    return;
}

XFontStruct *
x11loadfont(const char *fontname)
{
    XFontStruct *newfont;

    newfont = XLoadQueryFont(app->display, fontname);
    if (newfont) {
        fontw = newfont->max_bounds.width;
        fonth = newfont->ascent + newfont->descent;
    }

    return newfont;
}

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

#if (ZPCIMLIB2)
void
x11initpmaps(void)
{
    buttonpmaps[BUTTONNORMAL] = imlib2loadimage(app, "button.png",
                                                ZPC_BUTTON_WIDTH,
                                                ZPC_BUTTON_HEIGHT);
    buttonpmaps[BUTTONHOVER] = buttonpmaps[BUTTONNORMAL];
    buttonpmaps[BUTTONCLICKED] = buttonpmaps[BUTTONNORMAL];
#if 0
    buttonpmaps[BUTTONHOVER] = imlib2loadimage(app, "button.png",
                                               ZPC_BUTTON_WIDTH,
                                               ZPC_BUTTON_HEIGHT);
    buttonpmaps[BUTTONCLICKED] = imlib2loadimage(app, "button.png",
                                                 ZPC_BUTTON_WIDTH,
                                                 ZPC_BUTTON_HEIGHT);
#endif
#if 0
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
#endif

    return;
}
#endif

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
x11initwin(struct x11app *app, Window parent, int x, int y, int w, int h,
           int rvid)
{
    XSetWindowAttributes atr;
    Window               win;

    memset(&atr, 0, sizeof(atr));
    if (!parent) {
        parent = RootWindow(app->display, app->screen);
    }
    if (rvid) {
        atr.background_pixel = BlackPixel(app->display, app->screen);
    } else {
        atr.background_pixel = WhitePixel(app->display, app->screen);
    }
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

    return win;
}

void
displayexpose(void *arg, XEvent *event)
{
    struct x11win   *win = arg;
    struct zpctoken *token = zpcregstk[win->num];
    int              x;
    int              len;

    if (!event->xexpose.count) {
        x = 8;
        XClearWindow(app->display, win->id);
        while ((token) && (token->str)) {
            len = strlen(token->str);
            XDrawString(app->display, win->id, textgc,
                        x,
                        (fonth + 8) >> 1,
                        token->str, len);
            x += len * fonth;
            token = token->next;
        }
    }

    return;
}

void
buttonexpose(void *arg, XEvent *event)
{
    struct x11win *win = arg;
    size_t         len;

    if (!event->xexpose.count) {
        if (buttonpmaps[BUTTONNORMAL]) {
            XSetWindowBackgroundPixmap(app->display, win->id,
                                       buttonpmaps[BUTTONNORMAL]);
        }
        XClearWindow(app->display, win->id);
    } else {
        XClearWindow(app->display, win->id);
    }
    if (win->str) {
        if (win->fasmstr) {
            len = strlen(win->fasmstr);
            XDrawString(app->display, win->id, asmtextgc,
                        ZPC_BUTTON_WIDTH - len * fontw - 4,
                        ZPC_BUTTON_HEIGHT - fonth + 8,
                        win->fasmstr, len);
            if (win->asmstr) {
                len = strlen(win->asmstr);
                XDrawString(app->display, win->id, asmtextgc,
                            4,
                            ZPC_BUTTON_HEIGHT - fonth + 8,
                            win->asmstr, len);
            }
        } else if (win->asmstr) {
            len = strlen(win->asmstr);
            if (len) {
                XDrawString(app->display, win->id, asmtextgc,
                            (ZPC_BUTTON_WIDTH - len * fontw) >> 1,
                            ZPC_BUTTON_HEIGHT - fonth + 8,
                            win->asmstr, len);
            }
        }
        len = strlen(win->str);
        if (len) {
            if (win->asmstr) {
                XDrawString(app->display, win->id, win->textgc,
                            (ZPC_BUTTON_WIDTH - len * fontw) >> 1,
                            (ZPC_BUTTON_HEIGHT + fonth) >> 1,
                            win->str, len);
            } else {
                XDrawString(app->display, win->id, win->textgc,
                            (ZPC_BUTTON_WIDTH - len * fontw) >> 1,
                            (ZPC_BUTTON_HEIGHT + fonth) >> 1,
                            win->str, len);
            }
        }
    }
    XSync(app->display, False);

    return;
}

#if 0
void
buttonenter(void *arg, XEvent *event)
{
    struct x11win *win = arg;

    if (!event->xexpose.count) {
        if (buttonpmaps[BUTTONHOVER]) {
            XSetWindowBackgroundPixmap(app->display, win->id,
                                       buttonpmaps[BUTTONHOVER]);
        }
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
    struct x11win     *win = arg;
    int                evbut = toevbutton(event->xbutton.button);
    struct zpctoken   *token;
    struct zpcstkitem *item = zpcinputitem;
    copfunc_t         *func;
    uint64_t          *usrc = NULL;
    uint64_t          *udest = NULL;
    uint64_t           ures64;
    int64_t           *src = NULL;
    int64_t           *dest = NULL;
    int64_t            res64;
    float             *fsrc = NULL;
    float             *fdest = NULL;
    float              fres;
    double            *dsrc = NULL;
    double            *ddest = NULL;
    double             dres;
    int                type = 0;

    if (win->parm == ENTER) {
        stkenterinput();
        x11drawdisp();

        return;
    } else if (win->parm == EVAL) {
        if (item->scur == item->str) {
//            zpcregstk[0] = zpctokenize(zpcregstk[0]->str);
            zpcregstk[0] = zpcparse(zpcregstk[0]);
//            zpcprintqueue(zpcregstk[0]);
            zpcregstk[0] = zpceval(zpcregstk[0]);
        }
    } else if (win->parm == CLR) {
    } else if (win->parm == COPY) {
    } else if (win->parm == POP) {
    } else if (win->parm == DUP) {
    } else if (win->parm == DEL) {
        if (item->str != item->scur) {
            *--item->scur = '\0';
            x11drawdisp();
        }
    } else if (!win->parm || item->scur != item->str) {
        stkqueueinput(buttonstrtab[win->row][win->col]);
        x11drawdisp();

        return;
    } else {
        if (item->scur == item->str) {
            if (zpcregstk[0]) {
                token = zpcregstk[0];
                if (token) {
                    zpcfreequeue(token->next);
                    token->next = NULL;
                    if (token->type == ZPCUINT64) {
                        type = ZPCUINT64;
                        usrc = &token->data.u64;
                    } else if (token->type == ZPCINT64) {
                        type = ZPCINT64;
                        src = &token->data.i64;
                    } else if (token->type == ZPCFLOAT) {
                        type = ZPCFLOAT;
                        fsrc = &token->data.f32;
                    } else if (token->type == ZPCDOUBLE) {
                        dsrc = &token->data.f64;
                    } else {
                        fprintf(stderr, "EEEE\n");
                        
                        return;
                    }
                }
            } else {
                fprintf(stderr, "EEEE\n");
                
                return;
            }
            if (win->parm == 2) {
                token = zpcregstk[1];
                if (token) {
                    zpcfreequeue(token->next);
                    token->next = NULL;
                    if (type == ZPCUINT64 && token->type == ZPCUINT64) {
                        udest = &token->data.u64;
                    } else if (type == ZPCINT64 && token->type == ZPCINT64) {
                        dest = &token->data.i64;
                    } else if (type == ZPCFLOAT && token->type == ZPCFLOAT) {
                        fdest = &token->data.f32;
                    } else if (type == ZPCDOUBLE && token->type == ZPCDOUBLE) {
                        ddest = &token->data.f64;
                    } else {
                        fprintf(stderr, "EEEE\n");
                        
                        return;
                    }
                }
            }
        }
    }
#if 0
    XSetWindowBackgroundPixmap(app->display, win->id,
                               buttonpmaps[BUTTONCLICKED]);
    XClearWindow(app->display, win->id);
    XSync(app->display, False);
#endif
    if (evbut < NBUTTON) {
        token = calloc(1, sizeof(struct zpctoken));
        token->str = calloc(1, TOKENSTRLEN);
        if (type == ZPCINT64 || type == ZPCUINT64) {
            func = win->clickfunc[evbut];
            if (func) {
                if (type == ZPCINT64) {
                    func(src, dest, &res64);
                    token->type = ZPCINT64;
                    token->data.i64 = res64;
                    sprintf(token->str, "%lld", token->data.i64);
                } else {
                    func(usrc, udest, &ures64);
                    token->type = ZPCUINT64;
                    token->data.u64 = ures64;
                    sprintf(token->str, "%llu", token->data.u64);
                }
            }
        } else if (type == ZPCFLOAT) {
            func = win->clickfuncflt[evbut];
            if (func) {
                func(fsrc, fdest, &fres);
                token->type = ZPCFLOAT;
                token->data.f32 = fres;
                sprintf(token->str, "%f", token->data.f32);
            }
        } else {
            func = win->clickfuncdbl[evbut];
            if (func) {
                func(dsrc, ddest, &dres);
                token->type = ZPCDOUBLE;
                token->data.f64 = dres;
                sprintf(token->str, "%e", token->data.f64);
            }
        }
        if (func) {
            if (win->parm >= 1) {
                struct zpctoken *token1 = zpcregstk[0];
                struct zpctoken *token2;
                
                while (token1) {
                    token2 = token1->next;
                    free(token1);
                    token1 = token2;
                }
                zpcregstk[0] = NULL;
                if (win->parm == 2) {
                    token1 = zpcregstk[1];
                    while (token1) {
                        token2 = token1->next;
                        free(token1);
                        token1 = token2;
                    }
                    zpcregstk[1] = NULL;
                }
            }
            if (win->parm == 2) {
                zpcregstk[1] = token;
                memmove(&zpcregstk[0], &zpcregstk[1], (NREGSTK - 1) * sizeof(struct zpctoken *));
                zpcregstk[NREGSTK - 1] = NULL;
            } else {
                zpcregstk[0] = token;
            }
        }
    }
    x11drawdisp();

    return;
}

#if 0
void
buttonrelease(void *arg, XEvent *event)
{
    struct x11win *win = arg;
    int            evbut = toevbutton(event->xbutton.button);
    copfunc_t     *func;
    int64_t       *src = &zpcregstktab[1]->data.i64;
    int64_t       *dest = &zpcregstktab[0]->data.i64;

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
    Window         dispwin;
    int            row;
    int            col;
    struct x11win *wininfo;

    app = x11initapp(NULL);
    font = x11loadfont("fixed");
    if (!font) {
        fprintf(stderr, "failed to load font fixed\n");

        exit(1);
    }
#if (ZPCIMLIB2)
    mainwin = x11initwin(app, 0,
                         0, 0,
                         ZPC_WINDOW_WIDTH,
                         ZPC_WINDOW_HEIGHT + NSTKREG * (fonth + 8) + NSTKREG + ((fonth + 8) << 1) + 1,
                         ZPCREVERSE);
#else
    mainwin = x11initwin(app, 0,
                         0, 0,
                         ZPC_WINDOW_WIDTH + ZPC_NCOLUMN - 1,
                         ZPC_WINDOW_HEIGHT + NSTKREG * (fonth + 8) + NSTKREG + ((fonth + 8) << 1) + 1,
                         ZPCREVERSE);
#endif
    app->win = mainwin;
    x11inittitle();
    x11initgcs();
#if (ZPCIMLIB2)
    imlib2init(app);
    x11initpmaps();
#endif
    for (row = 0 ; row < ZPC_NROW ; row++) {
        for (col = 0 ; col < ZPC_NCOLUMN ; col++) {
#if (ZPCIMLIB2)
            win = x11initwin(app,
                             mainwin,
                             col * ZPC_BUTTON_WIDTH,
                             row * ZPC_BUTTON_HEIGHT,
                             ZPC_BUTTON_WIDTH,
                             ZPC_BUTTON_HEIGHT,
                             ZPCREVERSE);
#else
            win = x11initwin(app,
                             mainwin,
                             col * ZPC_BUTTON_WIDTH + col,
                             row * ZPC_BUTTON_HEIGHT + row,
                             ZPC_BUTTON_WIDTH,
                             ZPC_BUTTON_HEIGHT,
                             ZPCREVERSE);
#endif
            if (win) {
                wininfo = calloc(1, sizeof(struct x11win));
                wininfo->id = win;
                wininfo->str = buttonlabeltab[row][col];
                wininfo->asmstr = buttonasmtab[row][col];
                wininfo->fasmstr = buttonfltasmtab[row][col];
                wininfo->type = buttontypetab[row][col];
                if (wininfo->str) {
                    if (wininfo->type == ZPCBUTTONREG) {
                        wininfo->textgc = asmtextgc;
                    } else if (wininfo->type == ZPCBUTTONOPER) {
                        wininfo->textgc = optextgc;
                    } else if (wininfo->type == ZPCBUTTONDIGIT) {
                        wininfo->textgc = numtextgc;
                    } else if (wininfo->type == ZPCBUTTONUTIL) {
                        wininfo->textgc = utiltextgc;
                    } else {
                        wininfo->textgc = textgc;
                    }
                }
                wininfo->parm = parmtab[row][col];
                wininfo->row = row;
                wininfo->col = col;
#if 0
                wininfo->evfunc[EnterNotify] = buttonenter;
                wininfo->evfunc[LeaveNotify] = buttonleave;
#endif
                wininfo->evfunc[ButtonPress] = buttonpress;
                wininfo->evfunc[Expose] = buttonexpose;
                buttonwintab[row][col] = win;
                wininfo->clickfunc[0] = buttonopertab[row][col];
                wininfo->clickfuncflt[0] = buttonopertabflt[row][col];
                wininfo->clickfuncdbl[0] = buttonopertabdbl[row][col];
                x11addwin(wininfo);
                XSelectInput(app->display, win,
#if 0
                             EnterWindowMask | LeaveWindowMask
#endif
                             ExposureMask | ButtonPressMask);
                XMapRaised(app->display, win);
            } else {
                fprintf(stderr, "failed to create window\n");

                exit(1);
            }
        }
    }
    dispwin = x11initwin(app,
                        mainwin,
                        0,
                        ZPC_WINDOW_HEIGHT + ZPC_NROW,
                        ZPC_WINDOW_WIDTH,
                        NSTKREG * (fonth + 8) + NSTKREG + ((fonth + 8) << 2) + 1,
                        !ZPCREVERSE);
    XMapRaised(app->display, dispwin);
    for (row = 0 ; row < NSTKREG ; row++) {
        win = x11initwin(app,
                         dispwin,
                         0,
                         row * (fonth + 8) + row + 1,
                         ZPC_WINDOW_WIDTH,
                         fonth + 8,
                         ZPCREVERSE);
        if (win) {
            wininfo = calloc(1, sizeof(struct x11win));
            wininfo->id = win;
            wininfo->num = NSTKREG - row - 1;
            wininfo->evfunc[Expose] = displayexpose;
            stkwintab[NSTKREG - row - 1] = win;
            x11addwin(wininfo);
            XSelectInput(app->display, win,
                         ExposureMask);
            XMapRaised(app->display, win);
        }
    }
    inputwin = x11initwin(app,
                          dispwin,
                          0,
                          NSTKREG * (fonth + 8) + NSTKREG + 1,
                          ZPC_WINDOW_WIDTH,
                          (fonth + 8) << 2,
                          ZPCREVERSE);
    if (inputwin) {
        wininfo = calloc(1, sizeof(struct x11win));
        wininfo->id = win;
        wininfo->evfunc[Expose] = displayexpose;
        x11addwin(wininfo);
        XSelectInput(app->display, inputwin,
                     ExposureMask);
        XMapRaised(app->display, inputwin);
    }
    XMapSubwindows(app->display, mainwin);
    XMapSubwindows(app->display, dispwin);
    XMapRaised(app->display, mainwin);
}

void
x11nextevent(void)
{
    XEvent         event;
    struct x11win *win;
    x11evfunc_t   *func;

    XNextEvent(app->display, &event);
    win = x11findwin(event.xany.window);
    if (win) {
        func = win->evfunc[event.type];
        if (func) {
            func(win, &event);
        }
    }
}

#endif /* ZPCX11 */

