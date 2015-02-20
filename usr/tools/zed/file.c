#include <limits.h>
#include <zed/conf.h>

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

