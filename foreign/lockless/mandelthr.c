/*
 * shamelessly ripped from http://locklessinc.com/articles/mandelbrot/
 *
 * THANKS A MILLION to the original authors. ;)
 */

#define OPTIMIZED 1
#define FLOAT     0
#define DOUBLE    0
#define NEWSSE2   1
#define SSE2      1

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
#include <zero/param.h>

struct mandelthr {
    int yofs;
    int ylim;
};

/* X11 data */
static Display *dpy;
static Window win;
static XImage *bitmap;
static Atom wmDeleteMessage;
static GC gc;
static volatile int g_nthr;

static void exit_x11(void)
{
    XDestroyImage(bitmap);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

static void init_x11(int size)
{
    int bytes_per_pixel;

    XInitThreads();
    /* Attempt to open the display */
    dpy = XOpenDisplay(NULL);
	
    /* Failure */
    if (!dpy) exit(0);
	
    unsigned long white = WhitePixel(dpy,DefaultScreen(dpy));
    unsigned long black = BlackPixel(dpy,DefaultScreen(dpy));
	

    win = XCreateSimpleWindow(dpy,
                              DefaultRootWindow(dpy),
                              0, 0,
                              size, size,
                              0, black,
                              white);
	
    /* We want to be notified when the window appears */
//    XSelectInput(dpy, win, StructureNotifyMask);
    XSelectInput(dpy, win, ExposureMask);
	
    /* Make it appear */
    XMapWindow(dpy, win);

#if 0
    while (1)
	{
            XEvent e;
            XNextEvent(dpy, &e);
            if (e.type == MapNotify) break;
	}
#endif
    /* we need to wait for the Expose event instead of MapNotify */
    while (1)
	{
            XEvent e;
            XNextEvent(dpy, &e);
            if (e.type == Expose) break;
	}
	
    XTextProperty tp;
    char name[128] = "Mandelbrot";
    char *n = name;
    Status st = XStringListToTextProperty(&n, 1, &tp);
    if (st) XSetWMName(dpy, win, &tp);

    /* Wait for the MapNotify event */
    XFlush(dpy);
	
    int ii, jj;
    int depth = DefaultDepth(dpy, DefaultScreen(dpy));
    Visual *visual = DefaultVisual(dpy, DefaultScreen(dpy));
    int total;

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
    bitmap = XCreateImage(dpy, visual, depth,
                          ZPixmap, 0, malloc(total),
                          size, size, 32, 0);
	
    /* Init GC */
    gc = XCreateGC(dpy, win, 0, NULL);
    XSetForeground(dpy, gc, black);
	
    if (bytes_per_pixel != 4)
	{
            printf("Need 32bit colour screen!\n");
            exit_x11();
            exit(0);
	}
	
    XSelectInput(dpy, win, ExposureMask | KeyPressMask | StructureNotifyMask);
	
    wmDeleteMessage = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, win, &wmDeleteMessage, 1);
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
static void display_sse2(int size, float xmin, float xmax, float ymin, float ymax, int yofs, int ylim)
{
    int x, y;
    
    float xscal = (xmax - xmin) / size;
    float yscal = (ymax - ymin) / size;
    
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
            for (x = 0; x < size; x += 4) {
		{
                    v4sf cr = { xmin + x * xscal,
                                xmin + (x + 1) * xscal,
                                xmin + (x + 2) * xscal,
                                xmin + (x + 3) * xscal };
#if (!OPTIMIZED) || 1
                    v4sf ci = { ymin + y * yscal,
                                ymin + y * yscal,
                                ymin + y * yscal,
                                ymin + y * yscal };
#endif
                    
                    mandel_sse(cr, (v4sf)ci, counts);
                    
                    ((unsigned *) bitmap->data)[x + y * size] = cols[counts[0]];
                    ((unsigned *) bitmap->data)[x + 1 + y * size] = cols[counts[1]];
                    ((unsigned *) bitmap->data)[x + 2 + y * size] = cols[counts[2]];
                    ((unsigned *) bitmap->data)[x + 3 + y * size] = cols[counts[3]];
		}
                
                /* Display it line-by-line for speed */
                XPutImage(dpy, win, gc, bitmap,
                          0, y, 0, y,
                          size, 1);
            }
#if (OPTIMIZED) && 0
            ci = _mm_add_ps(ci, di);
#endif
        }
    }
    
    XFlush(dpy);
}

#elif (SSE2)

/* For each point, evaluate its colour */
static void display_sse2(int size, float xmin, float xmax, float ymin, float ymax, int yofs, int ylim)
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
                    
                    ((unsigned *) bitmap->data)[x + y * size] = cols[counts[0]];
                    ((unsigned *) bitmap->data)[x + 1 + y * size] = cols[counts[1]];
                    ((unsigned *) bitmap->data)[x + 2 + y * size] = cols[counts[2]];
                    ((unsigned *) bitmap->data)[x + 3 + y * size] = cols[counts[3]];
		}
            
            /* Display it line-by-line for speed */
                XPutImage(dpy, win, gc, bitmap,
                          0, y, 0, y,
                          size, 1);
            }
        }
    }
    
    XFlush(dpy);
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
			
                    ((unsigned *) bitmap->data)[x + y*size] = cols[counts];

#if (OPTIMIZED)
                    cr += xscal;
#endif
		}
		
            /* Display it line-by-line for speed */
            XPutImage(dpy, win, gc, bitmap,
                      0, y, 0, y,
                      size, 1);
#if (OPTIMIZED)
            ci += yscal;
#endif
	}
	
    XFlush(dpy);
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
static void display_float(int size, float xmin, float xmax, float ymin, float ymax, int yofs, float ylim)
{
    int x, y;
	
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
            for (x = 0; x < size; x++)
		{
#if (!OPTIMIZED)
                    cr = xmin + x * xscal;
                    ci = ymin + y * yscal;
#endif
                    counts = mandel_float(cr, ci);
                    ((unsigned *) bitmap->data)[x + y*size] = cols[counts];
#if (OPTIMIZED)
                    cr += xscal;
#endif
		}

            /* Display it line-by-line for speed */
            XPutImage(dpy, win, gc, bitmap,
                      0, y, 0, y,
                      size, 1);
#if (OPTIMIZED)
            ci += yscal;
#endif
	}
	
    XFlush(dpy);
}

#endif /* SSE2 */

/* Image size */
#define ASIZE 1000

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
    display_sse2(ASIZE, xmin, xmax, ymin, ymax, args->yofs, args->ylim);
#elif (SSE2)
    display_sse2(ASIZE, xmin, xmax, ymin, ymax, args->yofs, args->ylim);
#elif (DOUBLE)
    display_double(ASIZE, xmin, xmax, ymin, ymax, args->yofs, args->ylim);
#elif (FLOAT)
    display_float(ASIZE, xmin, xmax, ymin, ymax, args->yofs, args->ylim);
#endif
    g_nthr--;

    pthread_exit(NULL);
}

#define THRNUMUNITS CLSIZE

int main(void)
{
    int   nthr = get_nprocs_conf();
    int   ndx;
    int   delta = ASIZE / nthr;
    int   ofs = 0;
    struct mandelthr *args;
    pthread_t *thrtab = calloc(nthr, sizeof(pthread_t));
    pthread_t **thrptrtab = calloc(nthr, sizeof(pthread_t *));

    /* Make a window! */
    init_x11(ASIZE);
	
    init_colours();

    g_nthr = nthr;
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
        pthread_yield();
    } while (g_nthr);

    XPutImage(dpy, win, gc, bitmap,
              0, 0, 0, 0,
              ASIZE, ASIZE);
    XFlush(dpy);

//    display_double(ASIZE, xmin, xmax, ymin, ymax);

#ifdef WAIT_EXIT
    while(1)
	{
            XEvent event;
            KeySym key;
            char text[255];
		
            XNextEvent(dpy, &event);
	
            /* Just redraw everything on expose */
            if ((event.type == Expose) && !event.xexpose.count)
		{
                    XPutImage(dpy, win, gc, bitmap,
                              0, 0, 0, 0,
                              ASIZE, ASIZE);
		}
		
            /* Press 'q' to quit */
            if ((event.type == KeyPress) &&
                XLookupString(&event.xkey, text, 255, &key, 0) == 1)
		{
                    if (text[0] == 'q') break;
		}
		
            /* Or simply close the window */
            if ((event.type == ClientMessage) &&
                ((Atom) event.xclient.data.l[0] == wmDeleteMessage))
		{
                    break;
		}
	}
#endif

    /* Done! */
    exit_x11();
	
    return 0;
}

