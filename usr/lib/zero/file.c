#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

/* TODO: zmapfile() */

void *
zreadfile(char *filename, size_t *sizeret)
{
    void        *buf = NULL;
    struct stat  statbuf;
    size_t       nread;
    size_t       nleft;
    int          fd;

    if (sizeret) {
        *sizeret = 0;
    }
    if (stat(filename, &statbuf) < 0) {

        return NULL;
    }
    if (!S_ISREG(statbuf.st_mode)) {

        return NULL;
    }
    fd = open(filename, O_RDONLY);
    if (fd < 0) {

        return NULL;
    }
    nread = 0;
    nleft = statbuf.st_size;
    if (nleft) {
        buf = malloc(nleft);
        if (buf) {
            while (nleft) {
                nread = read(fd, buf, nleft);
                if (nread < 0) {
                    if (errno == EINTR) {
                        
                        continue;
                    } else {
                        free(buf);
                        
                        return NULL;
                    }
                } else {
                    nleft -= nread;
                }
            }
            if (sizeret) {
                *sizeret = statbuf.st_size;
            }
        }
    }
    close(fd);

    return buf;
}

size_t
zwritefile(char *filename, void *buf, size_t nb)
{
    size_t       nwritten;
    size_t       nleft;
    int          fd;

    fd = open(filename, O_WRONLY | O_TRUNC);
    if (fd < 0) {
        fprintf(stderr, "failed to create %s\n", filename);

        return -1;
    }
    nwritten = 0;
    nleft = nb;
    if (nleft) {
        buf = malloc(nleft);
        if (buf) {
            while (nleft) {
                nwritten = write(fd, buf, nleft);
                if (nwritten < 0) {
                    if (errno == EINTR) {
                        
                        continue;
                    } else {
                        free(buf);
                        
                        return -1;
                    }
                } else {
                    nleft -= nwritten;
                }
            }
        }
    }
    close(fd);
    if (nwritten != nb) {
        fprintf(stderr, "short write of %s\n", filename);

        return -1;
    }

    return nb;
}

