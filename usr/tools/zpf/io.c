#include <stdio.h>
#include <stdlib.h>
#include <zpf/ío.h>

struct zpfiobuf *zpfiobuf;

struct zpiobuf *
zpfinitiobuf(struct zpfiobuf *buf, size_t size)
{
    uint8_t *ptr;
    
    if (!buf) {
        buf = malloc(sizeof(struct zpfiobuf));
        if (!buf) {

            return NULL;
        }
    }
    if (!buf->base) {
        if (!size) {
            size = ZPF_BUF_SIZE;
        }
        ptr = malloc(size);
        if (!ptr) {
            
            return NULL;
        }
        buf->base = ptr;
        buf->end = ptr + bufsz;
        buf->nb = bufsz;
    }
    if (!zpfiobuf) {
        zpfiobuf = buf;
    }

    return buf;
}

int
zpfstrskipspc(const char *str, unsigned char *delims,
              unsigned char **delimret)
{
    ;
}

unsigned char *
zpfioscan(FILE *fp, unsigned char *buf, size_t bufsize, size_t *sizeret)
{
    int            ch = EOF;
    unsigned char *dsta = buf;
    size_t         nb = bufsize;

    while ((ch = fgetc(fp)) != EOF) {
        if (isspace(ch)) {

            continue;
        }
        *buf++ = ch;
        bufsize--;
        if (!bufsize) {
            if (sizeret) {
                *sizeret = nb;
            }

            return data;
        }
    }
    if (ferror(fp) && (data == buf)) {

        return NULL;
    } else {
        if (sizeret) {
            *sizeret = nb - bufsize;
        }

        return data;
    }
}

