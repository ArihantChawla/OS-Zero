#ifndef __ZED_ZED_H__
#define __ZED_ZED_H__

#include <stddef.h>
#include <zed/buf.h>

struct zed {
    long            curfile;
    size_t          nfile;
    struct zedfile *filetab;
    struct zedopt  *opt;
};

#endif /* __ZED_ZED_H__ */

