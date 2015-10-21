#ifndef __ZED_BUF_H__
#define __ZED_BUF_H__

#include <stddef.h>
#include <stdint.h>
#include <vt/color.h>
#include <vt/vt.h>

/* values for the flg-member */
#define BUFSYNCIO  (1 << 0)
#define BUFDUALMAP (1 << 1)
#define BUFSHM     (1 << 2)
struct zediobuf {
    volatile long  lk;          // mutex
    int            fd;          // file descriptor
    off_t          ofs;         // file offset
    size_t         len;         // buffer length in bytes
    long           flg;         // buffer flags
    long           nref;        // object reference count
    void          *data;        // [character] data
    uintptr_t      link;        // prev ^ next
};

struct zedrow {
    volatile long  lk;
    size_t         size;
    void          *data;
    struct zedrow *prev;
    struct zedrow *next;
    uint8_t        pad[CLSIZE
                       - sizeof(long) - sizeof(size_t) - 3 * sizeof(void *)];
};

/* values for charset- and chartype-fields */
#define ZED_ASCII 0x00
/* values for charset-field */
#define ZED_ISO8859_1 0x01
/* values for chartype-field */
#define ZED_8BIT  0x01
#define ZED_UTF8  0x02
#define ZED_UCS2  0x03
#define ZED_UTF16 0x04
#define ZED_UCS4  0x05
struct zedrowbuf {
#if (_REENTRANT)
    volatile long   lk;
#endif
    long              mode;     // per-buffer editor mode
    struct zedfile   *file;     // buffer file
    long              charset;  // character set ID such as ISO 8859-1
    long              chartype; // ASCII, 8BIT, UTF-8, UCS-2, UTF-16, UCS-4
    size_t            nrow;     // # of rows
    size_t            nrowmax;  // # of allocated rows
    struct zedrowbuf *rowtab;   // row structures
    void             *text;     // text data such as 8-bit ISO-8859
    void             *rend;     // rendition attributes
#if (ZEDZCPP)
    void             *cppq;
#endif
};

struct zedbuf {
    char             *path;     // file path
    struct zediobuf  *rdqueue;  // read request queue
    struct zediobuf  *wrqueue;  // write request queue
    volatile long     rowlk;    // lock to initialise nrow and rowbuf
    struct zedrowbuf *rowbuf;
};

#endif /* __ZED_BUF_H__ */

