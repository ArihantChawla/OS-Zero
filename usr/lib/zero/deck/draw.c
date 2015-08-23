#include <zero/gfx/rgb.h>

/*
 * TODO
 * ----
 * deckdrawline();
 * deckdrawrect();
 * deckdrawellipse();
 * deckdrawarc();
 */

__inline__ void
deckdrawpoint(struct deckimg *img,
              long x, long y,
              gfxargb32_t pixel)
{
    gfxargb32_t   *dest;
    gfxargb32_t   *destptr;
    unsigned long  destw;

    dest = img->data;
    destw = img->w;
    destptr = dest + y * destw + x;
    *destptr = pixel;

    return;
}

/*
 * (x1, y1)
 *        \
 *         \
 *          \
 *           \
 *           (x2, y2)
 */
void
deckdrawline(struct deckimg *img,
             long x1, long y1,
             long x2, long y2,
             gfxargb32_t pixel)
{
    gfxargb32_t   *dest;
    gfxargb32_t   *destptr;
    float          slope;
    unsigned long  destw;
    long           x;
    long           y;
    long           xpos;
    long           xmax;
    long           ymax;
    long           xdelta;
    long           ydelta;

    dest = img->data;
    destw = img->w;
    xdelta = x2 - x1;
    ydelta = y2 - y1;
    y = y1;
    xpos = 0;
    if (xdelta && ydelta) {
        slope = (float)ydelta / xdelta;
        if (x1 < x2) {
            xmax = x2;
            if (y1 < y2) {
                for (x = x1 ; x <= xmax ; x++) {
                    destptr = dest + y * destw + x;
                    ymax = y1 + (long)(xpos * slope);
                    while (y <= ymax) {
                        *destptr = pixel;
                        destptr += destw;
                        y++;
                    }
                    xpos++;
                }
            } else {
                for (x = x1 ; x <= xmax ; x++) {
                    ymax = y1 + (long)(xpos * slope);
                    destptr = dest + y * destw + x;
                    while (y >= ymax) {
                        *destptr = pixel;
                        destptr += destw;
                        y--;
                    }
                    xpos++;
                }
            }
        } else {
            xmax = x2;
            if (y1 < y2) {
                for (x = x1 ; x >= xmax ; x--) {
                    ymax = y1 + (long)(xpos * slope);
                    destptr = dest + y * destw + x;
                    while (y <= ymax) {
                        *destptr = pixel;
                        destptr += destw;
                        y++;
                    }
                    xpos--;
                }
            } else {
                for (x = x1 ; x >= xmax ; x--) {
                    ymax = y1 + (long)(xpos * slope);
                    destptr = dest + y * destw + x;
                    while (y >= ymax) {
                        *destptr = pixel;
                        destptr += destw;
                        y--;
                    }
                    xpos--;
                }
            }
        }
    } else if (xdelta) {
        if (x1 < x2) {
            xmax = x2;
            destptr = dest + y1 * destw + x1;
            for (x = x1 ; x <= xmax ; x++) {
                *destptr = pixel;
                destptr++;
            }
        } else {
            xmax = x1;
            destptr = dest + y1 * destw + x1;
            for (x = x2 ; x <= xmax ; x++) {
                *destptr = pixel;
                destptr++;
            }
        }
    } else if (ydelta) {
        if (y1 < y2) {
            ymax = y2;
            destptr = dest + y1 * destw + x1;
            for (y = y1 ; y <= ymax ; y++) {
                *destptr = pixel;
                destptr += destw;
            }
        } else {
            ymax = y1;
            destptr = dest + y2 * destw + x1;
            for (y = y2 ; y <= ymax ; y++) {
                *destptr = pixel;
                destptr += destw;
            }
        }
    }

    return;
}

/*
 *      (x1, y1)
 *            /\
 *           /  \
 *          /    \
 *         /      \
 *         --------
 * (x2, y2)        (x3, y3)
 */
void
deckdrawtriangle(struct deckimg *img,
                 long x1, long y1,
                 long x2, long y2,
                 long x3, long y3,
                 gfxargb32_t pixel)
{
    deckdrawline(img, x1, y1, x2, y2, pixel);
    deckdrawline(img, x1, y1, x3, y3, pixel);
    deckdrawline(img, x2, y2, x3, y3, pixel);
}

void
deckdrawrectangle(struct deckimg *img,
                  long x1, long y1,
                  long x2, long y2,
                  gfxargb32_t pixel)
{
    deckdrawline(img, x1, y1, x2, y1, pixel);
    deckdrawline(img, x1, y1, x1, y2, pixel);
    deckdrawline(img, x1, y2, x2, y2, pixel);
    deckdrawline(img, x2, y1, x2, y2, pixel);
    
    return;
}

void
deckfillrectangle(struct deckimg *img,
                  long x1, long y1,
                  long x2, long y2,
                  gfxargb32_t pixel)
{
    gfxargb32_t   *dest;
    gfxargb32_t   *destptr;
    unsigned long  destw;
    long           x;
    long           y;
    long           xmin;
    long           ymin;
    long           xmax;
    long           ymax;
    long           xpitch;
    
    dest = img->data;
    destw = img->w;
    xmin = MIN(x1, x2);
    ymin = MIN(y1, y2);
    xmax = MAX(x1, x2);
    ymax = MAX(y1, y2);
    xpitch = destw - xmax - xmin;
    for (y = ymin ; y <= ymax ; y++) {
        destptr = dest + y * destw + xmin;
        for (x = xmin ; x <= xmax ; x++) {
            *destptr = pixel;
            destptr++;
        }
    }
    
    return;
}

void
deckdrawcircle(struct deckimg *img,
               long x1, long y1,
               unsigned long r,
               gfxargb32_t pixel)
{
    gfxargb32_t   *dest;
    gfxargb32_t   *destptr;
    unsigned long  destw;
    unsigned long  r2;
    long           x;
    long           y;
    long           xpos;
    long           ylim;
    long           xval;
    long           yval;
    
    dest = img->data;
    destw = img->w;
    r2 = r * r;
    xpos = 0;
    ylim = r;
    for (yval = ylim ; yval > 0 ; yval--) {
        xval  = (long)(sqrt((double)(r2 - yval * yval)) + 0.5);
        fprintf(stderr, "XVAL: %d, XPOS: %d\n", xval, xpos);
        for (x = -xval ; x <= -xpos ; x++) {
            destptr = dest + (y1 - yval) * destw + x1 + x;
            *destptr = pixel;
        }
        for (x = xpos ; x <= xval ; x++) {
            fprintf(stderr, "X: %d\n", x);
            destptr = dest + (y1 - yval) * destw + x1 + x;
            *destptr = pixel;
        }
        xpos = xval;
    }
    xpos = r;
    for (yval = 0 ; yval >= -ylim ; yval--) {
        xval  = (long)(sqrt((double)(r2 - yval * yval)) + 0.5);
        fprintf(stderr, "XVAL: %d, XPOS: %d\n", xval, xpos);
        for (x = -xval ; x <= -xpos ; x++) {
            destptr = dest + (y1 - yval) * destw + x1 + x;
            *destptr = pixel;
        }
        for (x = xpos ; x <= xval ; x++) {
            fprintf(stderr, "X: %d\n", x);
            destptr = dest + (y1 - yval) * destw + x1 + x;
            *destptr = pixel;
        }
        xpos = xval;
    }

    return;
}

