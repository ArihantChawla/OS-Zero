#ifndef __ZED_ZED_H__
#define __ZED_ZED_H__

#include <stddef.h>
#include <zed/buf.h>

struct zed {
    long           curbuf;
    size_t         nbuf;
    struct zedbuf *buftab;
    struct zedopt *opt;
};

#endif /* __ZED_ZED_H__ */

