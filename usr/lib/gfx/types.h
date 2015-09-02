#ifndef __GFX_TYPES_H__
#define __GFX_TYPES_H__

/* FIXME: make X11 optional */

#include <zero/param.h>
#include <zero/cdecl.h>
#include <gfx/x11.h>
#include <gfx/rgb.h>

#if 0
struct gfximg {
    long  flg;
    long  w;
    long  h;
    void *data;
};
#endif

struct gfximg_x11 {
    char            *fname;
    gfxargb32_t     *data;
    gfxargb32_t     *maskdata;
    unsigned long    w;
    unsigned long    h;
    Imlib_Image     *imlib2;
    XImage          *xim;
#if (USE_SHM)
    int              shmid;
    int              pmapshmid;
    XShmSegmentInfo  ximseg;
    XShmSegmentInfo  pmapseg;
#endif
    Pixmap           pmap;
    Pixmap           mask;
};

#endif /* __GFX_TYPES_H__ */

