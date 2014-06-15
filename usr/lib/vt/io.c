#include <stddef.h>
#if !(__KERNEL__)
#include <stdlib.h>
#include <vt/term.h>
#endif
#include <vt/vt.h>
#if !defined(FREE)
#define FREE(x) FREE(x)
#endif
#if !defined(MALLOC)
#define MALLOC(x) malloc(x)
#endif

#if (__KERNEL__)

long
vtinitbuf(struct vt *vt)
{
    if (!ringinit(&vt->devbuf.in, NULL, VTBUFSIZE / sizeof(RING_ITEM))
        || !ringinit(&vt->devbuf.in, NULL, VTBUFSIZE / sizeof(RING_ITEM))) {
        
        return 0;
    }
    
    return 1;
}

void
vtfreebuf(struct vt *vt)
{
    void *ptr;

    ptr = vt->devbuf.in.base;
    if ((ptr) && ptr != vt->devbuf.in.data) {
        free(ptr);
        vt->devbuf.in.base = NULL;
    }
    ptr = vt->devbuf.out.base;
    if ((ptr) && ptr != vt->devbuf.out.data) {
        free(ptr);
        vt->devbuf.out.base = NULL;
    }
}

#else /* !__KERNEL__ */

long
vtinitbuf(struct vt *vt)
{
    void *ptr;

    ptr = MALLOC(TERMINBUFSIZE);
    if (!ptr) {

        return 0;
    }
    vt->iobuf.nin = TERMINBUFSIZE / sizeof(VTCHAR_T);
    vt->iobuf.inbuf = ptr;
    ptr = MALLOC(TERMOUTBUFSIZE);
    if (!ptr) {
        FREE(vt->iobuf.inbuf);

        return 0;
    }
    vt->iobuf.nout = TERMOUTBUFSIZE / sizeof(VTCHAR_T);
    vt->iobuf.outbuf = ptr;

    return 1;
}

void
vtfreebuf(struct vt *vt)
{
    void *ptr;

    ptr = vt->iobuf.inbuf;
    if (ptr) {
        FREE(ptr);
    }
    ptr = vt->iobuf.outbuf;
    if (ptr) {
        FREE(ptr);
    }

    return;
}

#endif

