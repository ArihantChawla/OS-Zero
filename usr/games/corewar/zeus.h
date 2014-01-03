#ifndef __COREWAR_ZEUS_H__
#define __COREWAR_ZEUS_H__

#if (ZEUSWINX11)
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <X11/Xutil.h>

#include <corewar/x11.h>
#endif

struct zeussel {
    uint8_t *bmap;
    int      last;
};

char * zeusdisasm(long pc, int *lenret);

#endif /* __COREWAR_ZEUS_H__ */

