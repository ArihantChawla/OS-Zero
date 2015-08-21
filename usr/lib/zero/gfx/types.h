#ifndef __ZERO_GFX_TYPES_H__
#define __ZERO_GFX_TYPES_H__

#include <zero/param.h>
#include <zero/cdecl.h>
#include <zero/gfx/x11.h>
#include <zero/gfx/rgb.h>

#if 0
struct gfximg {
    long  flg;
    long  w;
    long  h;
    void *data;
} PACK();
#endif

struct gfximg {
    char            *fname;
    argb32_t        *data;
    argb32_t        *maskdata;
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

#endif /* __ZERO_GFX_TYPES_H__ */

