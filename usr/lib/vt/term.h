#ifndef __VT_TERM_H__
#define __VT_TERM_H__

#define TERMXORG    1
#define TERMNSCREEN 2

#include <vt/vt.h>

struct term {
    struct vt *vt;
    char      *path;
#if (TERMXORG)
    int        connfd;
#endif
};

#endif /* __VT_TERM_H__ */

