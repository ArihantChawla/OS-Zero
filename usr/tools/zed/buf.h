#ifndef __ZED_BUF_H__
#define __ZED_BUF_H__

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
struct zedbuf {
#if (_REENTRANT)
    volatile long   lk;
#endif
    long            mode;       // per-buffer editor mode
    struct zedfile *file;       // buffer file
    size_t          nb;         // size of data-buffer
    void           *data;       // buffer
    long            charset;    // character set ID such as ISO 8859-1
    long            chartype;   // ASCII, 8BIT, UTF-8, UCS-2, UTF-16, UCS-4
    size_t          nrow;       // number of buffer rows
    size_t          ncol;       // number of buffer columns
    void           *ctab;       // character data; e.g., char **
    void           *rend;       // character rendition data, e.g. zedrend **
#if (ZEDZCPP)
    void           *cppq;
#endif
};

struct zedrow {
    size_t  size;
    void   *data;
};

struct zedrowbuf {
    size_t          ncol;
    size_t          nrow;
    size_t          nrowmax;
    struct zedline *rows;
};

#endif /* __ZED_BUF_H__ */

