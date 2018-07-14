/*
 * shamelessly ripped from http://locklessinc.com/articles/mandelbrot/
 *
 * THANKS A MILLION to the original authors. ;)
 */

#define OPTIMIZED 1
#define FLOAT     1
#define DOUBLE    0
#define NEWSSE2   0
#define SSE2      0

#if (SSE2)
#include <xmmintrin.h>
//typedef float v4sf __attribute__((vector_size(16)));
typedef __v4sf v4sf;
//void mandel_sse(v4sf cr, v4sf ci, unsigned *counts);
void mandel_sse(__m128 cr, __m128 ci, unsigned *counts);
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <err.h>
#include <sys/sysinfo.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <zero/cdefs.h>
#include <mach/param.h>
#include <mach/asm.h>

struct mandelthr {
    int yofs;
    int ylim;
};

struct mandel {
    volatile int nthr;
};

struct mandel_x11 {
    Display *dpy;
    Window   win;
    XImage  *bitmap;
    Atom     wmdelmsg;
    GC       gc;
};

static struct mandel     g_mandel;
static struct mandel_x11 g_x11;

/* X11 data */
#if 0
static Display *dpy;
static Window win;
static XImage *bitmap;
static Atom wmDeleteMessage;
static GC gc;
static volatile int g_nthr;
#endif

static void exit_x11(void)
{
    XDestroyImage(g_x11.bitmap);
    XDestroyWindow(g_x11.dpy, g_x11.win);
    XCloseDisplay(g_x11.dpy);
}

static void init_x11(int size)
{
    char           name[128] = "Mandelbrot";
    int            bytes_per_pixel;
    unsigned long  white;
    unsigned long  black;
    XTextProperty  tp;
    char          *n = name;
    Status         st;
    unsigned int   ii, jj;
    unsigned int   depth;
    Visual        *visual;
    int            total;

    XInitThreads();
    /* Attempt to open the display */
    g_x11.dpy = XOpenDisplay(NULL);
    white = WhitePixel(g_x11.dpy,DefaultScreen(g_x11.dpy));
    black = BlackPixel(g_x11.dpy,DefaultScreen(g_x11.dpy));
    depth = DefaultDepth(g_x11.dpy, DefaultScreen(g_x11.dpy));
    visual = DefaultVisual(g_x11.dpy, DefaultScreen(g_x11.dpy));

    /* Failure */
    if (!g_x11.dpy) exit(0);

    g_x11.win = XCreateSimpleWindow(g_x11.dpy,
                                    DefaultRootWindow(g_x11.dpy),
                                    0, 0,
                                    size, size,
                                    0, black,
                                    white);

    /* We want to be notified when the window appears */
    XSelectInput(g_x11.dpy, g_x11.win, ExposureMask);

    /* Make it appear */
    XMapWindow(g_x11.dpy, g_x11.win);

#if 0
    while (1)
	{
            XEvent e;

            XNextEvent(g_x11.dpy, &e);
            if (e.type == MapNotify) break;
	}
#endif
    /* we need to wait for the Expose event instead of MapNotify */
    while (1)
	{
            XEvent e;

            XNextEvent(g_x11.dpy, &e);
            if (e.type == Expose) break;
	}

    st = XStringListToTextProperty(&n, 1, &tp);
    if (st) XSetWMName(g_x11.dpy, g_x11.win, &tp);

    /* Wait for the MapNotify event */
    XFlush(g_x11.dpy);

    /* Determine total bytes needed for image */
    ii = 1;
    jj = (depth - 1) >> 2;
    while (jj >>= 1) ii <<= 1;

    /* Pad the scanline to a multiple of 4 bytes */
    total = size * ii;
    total = (total + 3) & ~3;
    total *= size;
    bytes_per_pixel = ii;

    if (bytes_per_pixel != 4)
	{
            printf("Need 32bit colour screen!");

	}

    /* Make bitmap */
    g_x11.bitmap = XCreateImage(g_x11.dpy, visual, depth,
                                ZPixmap, 0, malloc(total),
                                size, size, 32, 0);

    /* Init GC */
    g_x11.gc = XCreateGC(g_x11.dpy, g_x11.win, 0, NULL);
    XSetForeground(g_x11.dpy, g_x11.gc, black);

    if (bytes_per_pixel != 4)
	{
            printf("Need 32bit colour screen!\n");
            exit_x11();
            exit(0);
	}

    XSelectInput(g_x11.dpy, g_x11.win, ExposureMask | KeyPressMask | StructureNotifyMask);

    g_x11.wmdelmsg = XInternAtom(g_x11.dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(g_x11.dpy, g_x11.win, &g_x11.wmdelmsg, 1);
}

//#define MAX_ITER	(1 << 14)
#define MAX_ITER	(1 << 16)
static unsigned cols[MAX_ITER + 1];

static void init_colours(void)
{
    int i;

    for (i = 0; i < MAX_ITER; i++)
	{
            char r = (rand() & 0xff) * MAX_ITER / (i + MAX_ITER + 1);
            char g = (rand() & 0xff) * MAX_ITER / (i + MAX_ITER + 1);
            char b = (rand() & 0xff) * MAX_ITER / (i + MAX_ITER + 1);

            cols[i] = b + 256 * g + 256 * 256 * r;
	}

    cols[MAX_ITER] = 0;
}

#if (NEWSSE2)

/* For each point, evaluate its colour */
#if 0
static void display_sse2(int size, float xmin, float xmax, float ymin, float ymax, int yofs, int ylim)
#endif
static void display_sse2(int width, int height, float xmin, float xmax, float ymin, float ymax, int yofs, int ylim)
{
    int x, y;
    int xpos, ypos;
    float xscal = (xmax - xmin) / width;
    float yscal = (ymax - ymin) / height;

    unsigned counts[4];

#if (OPTIMIZED) && 0
    __m128 ci = (__m128){ ymin, ymin, ymin, ymin };
    __m128 di = (__m128){ yscal, yscal, yscal, yscal };
#if 0
    v4sf ci = { ymin,
                ymin,
                ymin,
                ymin };
    v4sf di = { yscal, yscal, yscal, yscal };
#endif
#endif

    for (y = yofs; y < ylim; y++) {
	{
            for (ypos = 0 ; ypos < height ; ypos++) {
                for (xpos = 0; xpos < width; xpos += 4) {
                    {
                        v4sf cr = { xmin + xpos * xscal,
                                    xmin + (xpos + 1) * xscal,
                                    xmin + (xpos + 2) * xscal,
                                    xmin + (xpos + 3) * xscal };
#if (!OPTIMIZED) || 1
                        v4sf ci = { ymin + y * yscal,
                                    ymin + y * yscal,
                                    ymin + y * yscal,
                                    ymin + y * yscal };
#endif

                        mandel_sse(cr, (v4sf)ci, counts);

                        ((unsigned *) g_x11.bitmap->data)[xpos + y * width] = cols[counts[0]];
                        ((unsigned *) g_x11.bitmap->data)[xpos + 1 + y * width] = cols[counts[1]];
                        ((unsigned *) g_x11.bitmap->data)[xpos + 2 + y * width] = cols[counts[2]];
                        ((unsigned *) g_x11.bitmap->data)[xpos + 3 + y * width] = cols[counts[3]];
                    }

                    /* Display it line-by-line for speed */
                    XPutImage(g_x11.dpy, g_x11.win, g_x11.gc, g_x11.bitmap,
                              0, y, 0, y,
                              width, 1);
                }
#if (OPTIMIZED) && 0
                ci = _mm_add_ps(ci, di);
#endif
            }
        }

        XFlush(g_x11.dpy);
    }

    return;
}

#elif (SSE2)

/* For each point, evaluate its colour */
static void display_sse2(int width, int height, float xmin, float xmax, float ymin, float ymax, int yofs, int ylim)
{
    int x, y;

    float xscal = (xmax - xmin) / size;
    float yscal = (ymax - ymin) / size;

    unsigned counts[4];

    for (y = yofs; y < ylim; y++) {
        {
            for (x = 0; x < size; x += 4) {
                {
                    v4sf cr = { xmin + x * xscal,
                                xmin + (x + 1) * xscal,
                                xmin + (x + 2) * xscal,
                                xmin + (x + 3) * xscal };
                    v4sf ci = { ymin + y * yscal,
                                ymin + y * yscal,
                                ymin + y * yscal,
                                ymin + y * yscal };

                    mandel_sse(cr, ci, counts);

                    ((unsigned *) g_x11.bitmap->data)[x + y * size] = cols[counts[0]];
                    ((unsigned *) g_x11.bitmap->data)[x + 1 + y * size] = cols[counts[1]];
                    ((unsigned *) g_x11.bitmap->data)[x + 2 + y * size] = cols[counts[2]];
                    ((unsigned *) g_x11.bitmap->data)[x + 3 + y * size] = cols[counts[3]];
                }

                /* Display it line-by-line for speed */
                XPutImage(g_x11.dpy, g_x11.win, g_x11.gc, g_x11.bitmap,
                          0, y, 0, y,
                          size, 1);
            }
        }
    }

    XFlush(g_x11.dpy);
}

#elif (DOUBLE)

static unsigned mandel_double(double cr, double ci)
{
    double zr = cr, zi = ci;
    double tmp;

    unsigned i;

    for (i = 0; i < MAX_ITER; i++)
        {
            tmp = zr * zr - zi * zi + cr;
            zi *= 2 * zr;
            zi += ci;
            zr = tmp;

            if (zr * zr + zi * zi > 4.0) break;
        }

    return i;
}

/* For each point, evaluate its colour */
static void display_double(int size, double xmin, double xmax, double ymin, double ymax, int yofs, int ylim)
{
    int x, y;

    double cr, ci;

    double xscal = (xmax - xmin) / size;
    double yscal = (ymax - ymin) / size;

    unsigned counts;

#if (OPTIMIZED)
    cr = xmin;
    ci = ymin;
#endif

    for (y = yofs ; y < ylim; y++)
        {
            for (x = 0; x < size; x++)
                {
#if (!OPTIMIZED)
                    cr = xmin + x * xscal;
                    ci = ymin + y * yscal;
#endif

                    counts = mandel_double(cr, ci);

                    ((unsigned *) g_x11.bitmap->data)[x + y*size] = cols[counts];

#if (OPTIMIZED)
                    cr += xscal;
#endif
                }

            /* Display it line-by-line for speed */
            XPutImage(g_x11.dpy, g_x11.win, g_x11.gc, g_x11.bitmap,
                      0, y, 0, y,
                      size, 1);
#if (OPTIMIZED)
            ci += yscal;
#endif
        }

    XFlush(g_x11.dpy);
}

#elif FLOAT

static unsigned mandel_float(float cr, float ci)
{
    float zr = cr, zi = ci;
    float tmp;

    unsigned i;

    for (i = 0; i < MAX_ITER; i++)
        {
            tmp = zr * zr - zi * zi + cr;
            zi *= 2 * zr;
            zi += ci;
            zr = tmp;

            if (zr * zr + zi * zi > 4.0) break;
        }

    return i;
}

/* For each point, evaluate its colour */
static void display_float(int width, int height,
                          float xmin, float xmax,
                          float ymin, float ymax, int yofs, int ylim)
{
    int size = width * height;
    int x;
    int y;

    float cr, ci;

    float xscal = (xmax - xmin) / size;
    float yscal = (ymax - ymin) / size;

    unsigned counts;

#if (OPTIMIZED)
    cr = xmin;
    ci = ymin;
#endif
    for (y = yofs; y < ylim; y++)
        {
            for (x = 0; x < width; x++)
                {
#if (!OPTIMIZED)
                    cr = xmin + x * xscal;
                    ci = ymin + y * yscal;
#endif
                    counts = mandel_float(cr, ci);
                    ((unsigned *) g_x11.bitmap->data)[x + y*width] = cols[counts];
#if (OPTIMIZED)
                    cr += xscal;
#endif
                }

            /* Display it line-by-line for speed */
            XPutImage(g_x11.dpy, g_x11.win, g_x11.gc, g_x11.bitmap,
                      0, y, 0, y,
                      width, 1);
#if (OPTIMIZED)
            ci += yscal;
#endif
        }

    XFlush(g_x11.dpy);
}

#endif /* SSE2 */

/* Image size */
//#define ASIZE 1000
//#define ASIZE 1280
#define WIDTH  1280
#define HEIGHT 720

/* Comment out this for benchmarking */
//#define WAIT_EXIT

void *
mandel_start(void *arg)
{
    float xmin = -2;
    float xmax = 1.0;
    float ymin = -1.5;
    float ymax = 1.5;
    struct mandelthr *args = arg;

#if (NEWSSE2)
    display_sse2(WIDTH, HEIGHT, xmin, xmax, ymin, ymax, args->yofs, args->ylim);
#elif (SSE2)
    display_sse2(WIDTH, HEIGHT, xmin, xmax, ymin, ymax, args->yofs, args->ylim);
#elif (DOUBLE)
    display_double(WIDTH, HEIGHT, xmin, xmax, ymin, ymax, args->yofs, args->ylim);
#elif (FLOAT)
    display_float(WIDTH, HEIGHT, xmin, xmax, ymin, ymax, args->yofs, args->ylim);
#endif
    g_mandel.nthr--;

    pthread_exit(NULL);
}

#define THRNUMUNITS CLSIZE

int main(void)
{
    int                nthr = 4; //get_nprocs_conf();
    int                ndx;
    int                delta = HEIGHT / nthr;
    int                ofs = 0;
    struct mandelthr  *args;
    pthread_t         *thrtab = calloc(nthr, sizeof(pthread_t));
    pthread_t        **thrptrtab = calloc(nthr, sizeof(pthread_t *));

    fprintf(stderr, "mandel launching %d threads\n", nthr);
    /* Make a window! */
    init_x11(WIDTH);

    init_colours();

    g_mandel.nthr = nthr;
    for (ndx = 0 ; ndx < nthr ; ndx++) {
        args = calloc(1, sizeof(struct mandelthr));
        args->yofs = ofs;
        args->ylim = ofs + delta;
        pthread_create(&thrtab[ndx], NULL, mandel_start, args);
        ofs += delta;
    }
    for (ndx = 0 ; ndx < nthr ; ndx++) {
        pthread_join(thrtab[ndx], (void **)&thrptrtab[ndx]);
    }
    do {
        //        pthread_yield();
        //        m_waitspin();
        usleep(50000);
        pthread_yield();
    } while (g_mandel.nthr);

    XPutImage(g_x11.dpy, g_x11.win, g_x11.gc, g_x11.bitmap,
              0, 0, 0, 0,
              WIDTH, HEIGHT);
    XFlush(g_x11.dpy);

//    display_double(ASIZE, xmin, xmax, ymin, ymax);

#ifdef WAIT_EXIT
    while(1)
        {
            XEvent event;
            KeySym key;
            char text[255];

            XNextEvent(g_x11.dpy, &event);

            /* Just redraw everything on expose */
            if ((event.type == Expose) && !event.xexpose.count)
                {
                    XPutImage(g_x11.dpy, g_x11.win, g_x11.gc, g_x11.bitmap,
                              0, 0, 0, 0,
                              WIDTH, HEIGHT);
                }

            /* Press 'q' to quit */
            if ((event.type == KeyPress) &&
                XLookupString(&event.xkey, text, 255, &key, 0) == 1)
                {
                    if (text[0] == 'q') break;
                }

            /* Or simply close the window */
            if ((event.type == ClientMessage) &&
                ((Atom) event.xclient.data.l[0] == g_x11.wmdelmsg))
                {
                    break;
                }
        }
#endif

    sleep(5);

    /* Done! */
    exit_x11();

    return 0;
}

