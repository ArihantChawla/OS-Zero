/* valhalla assembler I/O interface */

#include <vas/conf.h>

#include <stdlib.h>
#include <vas/vas.h>
#include <vas/io.h>

extern uint8_t        *vaslinebuf;
extern uint8_t        *vasstrbuf;
extern struct readbuf *vasreadbuftab;
extern long            vasnreadbuf;

void
vasinitbuf(void)
{
#if (VASBUF)
    long l;
#endif

    vaslinebuf = malloc(VAS_LINE_BUFSIZE);
    vasstrbuf = malloc(VAS_LINE_BUFSIZE);
#if (VASBUF)
    vasreadbuftab = malloc(vasnreadbuf * sizeof(struct readbuf));
    for (l = 0 ; l < vasnreadbuf ; l++) {
        vasreadbuftab[l].data = malloc(VAS_LINE_BUFSIZE);
    }
#endif

    return;
}

#if !(VASMMAP) && (VASBUF)
static int
vasgetc(int fd, int bufid)
{
    struct readbuf *buf = &vasreadbuftab[bufid];
    void           *ptr;
    ssize_t         nleft = VASBUFSIZE;
    ssize_t         n;
    int             ch = EOF;
    long            l = vasnreadbuf;

    if (bufid >= vasnreadbuf) {
        vasnreadbuf <<= 1;
        ptr = realloc(vasreadbuftab, vasnreadbuf * sizeof(struct readbuf));
        if (!ptr) {

            exit(1);
        }
        vasreadbuftab = ptr;
        for ( ; l < vasnreadbuf ; l++) {
            ptr = malloc(VASBUFSIZE);
            if (!ptr) {

                exit(1);
            }
            vasreadbuftab[l].data = ptr;
        }
    }
    if (buf->cur < buf->lim) {
        ch = *buf->cur++;
    } else if (buf->cur == buf->lim) {
        n = 0;
        while (nleft) {
            n = read(fd, buf->data, VASBUFSIZE);
            if (n < 0) {
                if (errno == EINTR) {

                    continue;
                } else {

                    return EOF;
                }
            } else if (n == 0) {

                break;
            } else {
                nleft -= n;
            }
        }
        if (nleft == VASBUFSIZE) {

            return EOF;
        }
        buf->cur = buf->data;
        buf->lim = (uint8_t *)buf->data + VASBUFSIZE - nleft;
        ch = *buf->cur++;
    }

    return ch;
}
#endif

