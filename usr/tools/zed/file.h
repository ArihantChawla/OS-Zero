#ifndef __ZED_FILE_H__
#define __ZED_FILE_H__

#include <zed/conf.h>

struct zedfile {
#if (_REENTRANT)
    volatile long  lk;          // mutex
#endif
    size_t         nb;          // file size in bytes
    char          *name;        // file name
    char          *tmpname;     // temporary name for file
    struct zedbuf *head;        // the first buffer of file
    struct zedbuf *tail;        // the last buffer of file
#if (ZEDZCC)
    void          *ccq;         // file tokenised by zcc
#endif
};

#endif /* __ZED_FILE_H__ */

