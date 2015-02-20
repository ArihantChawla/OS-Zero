#ifndef __ZED_BUF_H__
#define __ZED_BUF_H__

/* values for ctype-field */
#define ZED_ASCII 0x00
#define ZED_8BIT  0x01
#define ZED_UTF8  0x02
#define ZED_UCS2  0x03
#define ZED_UTF16 0x04
#define ZED_UCS4  0x05
struct zedbuf {
#if (_REENTRANT)
    volatile long   lk;
#endif
    struct zedfile *file;
    size_t          nb;         // size of data-buffer
    void           *data;       // buffer
    struct zedbuf  *prev;       // previous on list
    struct zedbuf  *next;       // next on list
    long            ctype;      // ASCII, 8BIT, UTF-8, UCS-2, UTF-16, UCS-4
    size_t          nrow;       // number of buffer rows
    size_t          ncol;       // number of buffer columns
    void           *ctab;       // character data; e.g., char **
    void           *rend;       // character rendition data, e.g. zedrend **
#if (ZEDZCPP)
    void           *cppq;
#endif
};

#endif /* __ZED_BUF_H__ */

