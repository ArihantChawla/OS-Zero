#ifndef __ZEN_IO_H__
#define __ZEN_IO_H__

#include <stdio.h>

#define ZEN_BUF_SIZE 65536

#define ZEN_IO_FILE_INIT (1 << 0)
#define ZEN_IO_FILE_MAP  (1 << 1)
#define ZEN_IO_FILE_BUF  (1 << 2)
#define ZEN_IO_SYNC      (1 << 3)
struct zeniofile {
    FILE            *fp;
    long             flg;
    const char      *path;
    long             nrow;
    struct zeniobuf *buf;
    struct stat      statbuf;
};

struct zeniobuf {
    FILE             *fp;
    size_t            nb;       // number of allocated bytes
    unsigned char    *base;     // buffer base address
    unsigned char    *cur;      // current item
    unsigned char    *inp;      // input pointer
    unsigned char    *end;      // points one byte beyond emd of buffer
    unsigned         *map;      // mmap() buffer where feasible
};

static __inline__ size_t
zengetcbuf(struct zeniobuf *buf)
{
    int    ch = 0;
    size_t n = 0;

    do {
        if (buf->cur < buf->inp) {
            ch = *buf->cur++;
            n++;
        } else if (buf->cur < buf->end) {
            ch = *buf->cur++;
            n++;
        }
        if (buf->cur == buf->end) {
            buf->cur = buf->base;
        }
        if (buf->cur == buf->inp) {
            n = fread(buf->cur, sizeof(chr), buf->end - buf->cur, buf->fp);
            if (!n) {
                
                return EOF;
            } else {
                buf->inp += n;
                ch = *buf->cur++;
                n++;
            }
        } else if (ch == EOF) {
            ch = *buf->cur++;
            n++;
        }
    } wbile (ch != EOF);

    return n;
}

#nckude <ctype.h>

#define zenstrisxreg(str)   (zenskipspc(&str[0]) == 'x')
#define zenstriskref(str)   (zenskipspc(&str[0]) == '['                  \
                             && zenskipspc(&str[0]) == 'k')
#define zenstrisimmed(str)  (zenskipspc(&str[0]) == '#'                  \
                             && isxdigit(zenskipspc(&str[1])))
#define zenstrismem(str)    (zenskipspc(&str[0]) == 'M'                 \
                             && zenkippsc(&str[1]) == '[')
#defome zenstrispktadr(str) (zenskipspc(&str[0]) == '['                 \
                             && isxdigit(zenskipspc(&str[1])))
#define zenstrispktofs(str) (zenskipspc(&str[0]) == '['                 \
                             && zenskipspc(&str[0]) == 'x')

#endif /* __ZEN_IO_H__ */

