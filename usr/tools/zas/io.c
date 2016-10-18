/* zero assembler I/O interface */

#include <zas/conf.h>

#include <stdlib.h>
#include <zas/zas.h>
#include <zas/io.h>

extern uint8_t        *zaslinebuf;
extern uint8_t        *zasstrbuf;
extern struct readbuf *zasreadbuftab;
extern long            zasnreadbuf;

void
zasinitbuf(void)
{
#if (ZASBUF)
    long l;
#endif

    zaslinebuf = malloc(ZASLINELEN);
    zasstrbuf = malloc(ZASLINELEN);
#if (ZASBUF)
    zasreadbuftab = malloc(zasnreadbuf * sizeof(struct readbuf));
    for (l = 0 ; l < zasnreadbuf ; l++) {
        zasreadbuftab[l].data = malloc(ZASBUFSIZE);
    }
#endif

    return;
}

#if !(ZASMMAP) && (ZASBUF)
static int
zasgetc(int fd, int bufid)
{
    struct readbuf *buf = &zasreadbuftab[bufid];
    void           *ptr;
    ssize_t         nleft = ZASBUFSIZE;
    ssize_t         n;
    int             ch = EOF;
    long            l = zasnreadbuf;

    if (bufid >= zasnreadbuf) {
        zasnreadbuf <<= 1;
        ptr = realloc(zasreadbuftab, zasnreadbuf * sizeof(struct readbuf));
        if (!ptr) {

            exit(1);
        }
        zasreadbuftab = ptr;
        for ( ; l < zasnreadbuf ; l++) {
            ptr = malloc(ZASBUFSIZE);
            if (!ptr) {

                exit(1);
            }
            zasreadbuftab[l].data = ptr;
        }
    }
    if (buf->cur < buf->lim) {
        ch = *buf->cur++;
    } else if (buf->cur == buf->lim) {
        n = 0;
        while (nleft) {
            n = read(fd, buf->data, ZASBUFSIZE);
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
        if (nleft == ZASBUFSIZE) {

            return EOF;
        }
        buf->cur = buf->data;
        buf->lim = (uint8_t *)buf->data + ZASBUFSIZE - nleft;
        ch = *buf->cur++;
    }
    
    return ch;
}
#endif

