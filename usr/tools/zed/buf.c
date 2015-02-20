#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <endian.h>
#include <zero/param.h>
#include <zero/cdecl.h>
#include <zero/file.h>
#include <zero/unix.h>
#include <gfx/rgb.h>

#define ZEDZCC  1       // tokenise files with zcc
#define ZEDZCPP 1       // preprocess files into temporary buffers with zcc

/* 32-bit encoding for RGB + character value */

#if defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(__amd64__)
#define zedgetblue(rp)  ((rp)->blue)
#define zedgetgreen(rp) ((rp)->green)
#define zedgetred(rp)   ((rp)->blue)
#define zedgetcval(rp)  ((rp)->blue)
#if (_BYTE_ORDER == _LITTLE_ENDIAN)
struct rend {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t data;
} PACK();
#else
struct rend {
    uint8_t data;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} PACK();
#endif
typedef struct rend zedrend32;
#else /* !X86 */
#define zedgetblue(rp)  ((rp) & 0xff)
#define zedgetgreen(rp) (((rp) >> 8) & 0xff)
#define zedgetred(rp)   (((rp) >> 16) & 0xff)
#define zedgetcval(rp)  (((rp) >> 24) & 0xff)
typedef uint32_t zedrend32;
#endif

/* 64-bit encoding for ARGB + Unicode */
struct rendpix {
    argb32_t pix;       // 32-bit ARGB-pixel
    int32_t  data;      // 32-bit Unicode character
} PACK();
typedef struct rendpix zedrendpix;

struct zed {
    size_t          nfile;
    size_t          curfile;
    size_t          ntab;
    struct zedfile *ftab;
};

struct zedfile {
#if (_REENTRANT)
    volatile long  lk;
#endif
    size_t         nb;
    char          *name;
    char          *tmpname;
    struct zedbuf *head;
    struct zedbuf *tail;
#if (ZEDZCC)
    void          *ccq;        // file tokenised by zcc
#endif
};

#define ZED_ASCII 0x00
#define ZED_8BIT  0x01
#define ZED_UTF8  0x02
#define ZED_UCS2  0x03
#define ZED_UTF16 0x04
#define ZED_UCS4  0x05

struct zedbuf {
#if (_REENTRANT)
    volatile long  lk;
#endif
    size_t         nb;          // size of data-buffer
    void          *data;        // buffer
    struct zedbuf *prev;        // previous on list
    struct zedbuf *next;        // next on list
    long           ctype;       // ASCII, 8BIT, UTF-8, UCS-2, UTF-16, UCS-4
    size_t         nrow;        // number of window rows
    size_t         ncol;        // number of window columns
    void          *ctab;        // character data; e.g., char **
    void          *rend;        // character rendition data, e.g. zedrend **
#if (ZEDZCPP)
    void          *cppq;
#endif
};

static struct zed zed;

/*
 * read file into a buffer; return pointer to buffer;
 * *nameret = tmpname; *sizeret = nb;
 */
void *
zedclonefile(char *srcname, char **nameret, size_t *sizeret)
{
    void   *buf;
    size_t  nb;
    char   *src = srcname;
    char   *dest;
    char   *tmpname;
    int     bufsz = PATH_MAX;

    buf = zreadfile(srcname, &nb);
    if ((buf) && (nb)) {
        tmpname = malloc(bufsz);
        if (tmpname) {
            tmpname[0] = '.';
            tmpname[1] = '#';
            dest = &tmpname[2];
            bufsz -= 3;
            if (src) {
                while ((*src) && (bufsz)) {
                    *dest++ = *src++;
                    bufsz--;
                }
                *dest = '\0';
            }
        }
        if (zwritefile(tmpname, buf, nb) < 0) {
            free(buf);
            free(tmpname);

            return NULL;
        }
        *nameret = tmpname;
        *sizeret = nb;
    }

    return buf;
}

struct zedfile *
zedinitfile(char *filename)
{
    int             fd1;
    int             fd2;
    struct zedfile *file = calloc(1, sizeof(struct zedbuf));
    void           *buf;
    void           *mptr;
    char           *tmpname;
    size_t          nb;
    size_t          nfile;

    if (buf) {
        nfile = zed.nfile;
        if (zed.curfile == nfile - 1) {
            nfile <<= 1;
            mptr = realloc(zed.ftab, nfile * sizeof(struct zedfile));
            if (!mptr) {
                free(zed.ftab);
                fprintf(stderr, "out of memory\n");

                exit(1);
            }
            zed.ftab = mptr;
            memset(zed.ftab + zed.nfile,
                   0,
                   zed.nfile * sizeof(struct zedfile *));
            zed.nfile = nfile;
        }
        zed.curfile++;
        buf = zedclonefile(filename, &tmpname, &nb);
        if (buf) {
            file->name = strdup(filename);
            file->tmpname = tmpname;
            file->nb = nb;
        }
    }

    return file;
}

long
zedinit(int argc, char *argv[])
{
    struct zedfile *ftab = calloc(8, sizeof(struct zedfile));

    zed.nfile = 8;
    zed.curfile = 0;
    zed.ftab = ftab;
}

