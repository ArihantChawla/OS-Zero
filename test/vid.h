#ifndef __ZERO_VID_H__
#define __ZERO_VID_H__

/* fmt (format) values */
#define VIDBGRA32 0
#define VIDBGR24  1
#define VIDBGR555 2
#define VIDBGR565 3
#define VIDVGA8   4
struct vidbuf {
    long  desc;         // object descriptor
    long  fps;          // refresh rate in frames per second
    long  fmt;          // video data format
    long  width;        // image width
    long  height;       // image height
    void *adr;          // buffer address
    long  size;         // buffer size in bytes
};

#endif /* __ZERO_VID_H__ */

