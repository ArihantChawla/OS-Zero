#ifndef __VT_TERM_H__
#define __VT_TERM_H__

#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>

#define TERMXORG    1
#define TERMNSCREEN 2

#define TERMINBUFSIZE  (8 * PAGESIZE)
#define TERMOUTBUFSIZE (8 * PAGESIZE)

#include <vt/vt.h>
//#include <vt/io.h>

struct term {
    struct vt *vt;
    char      *path;
#if (TERMXORG)
    int        connfd;
#endif
};

#endif /* __VT_TERM_H__ */

